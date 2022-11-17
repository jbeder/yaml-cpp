#include "stream.h"

#ifndef YAML_PREFETCH_SIZE
#define YAML_PREFETCH_SIZE 2048
#endif

#define CP_REPLACEMENT_CHARACTER (0xFFFD)

namespace YAML {

inline char Utf8Adjust(unsigned long ch, unsigned char lead_bits,
                       unsigned char rshift) {
  const unsigned char header =
      static_cast<unsigned char>(((1 << lead_bits) - 1) << (8 - lead_bits));
  const unsigned char mask = (0xFF >> (lead_bits + 1));
  return static_cast<char>(
      static_cast<unsigned char>(header | ((ch >> rshift) & mask)));
}

inline void QueueUnicodeCodepoint(std::deque<char>& q, unsigned long ch) {
  // We are not allowed to queue the Stream::eof() codepoint, so
  // replace it with CP_REPLACEMENT_CHARACTER
  if (static_cast<unsigned long>(Stream::eof()) == ch) {
    ch = CP_REPLACEMENT_CHARACTER;
  }

  if (ch < 0x80) {
    q.push_back(Utf8Adjust(ch, 0, 0));
  } else if (ch < 0x800) {
    q.push_back(Utf8Adjust(ch, 2, 6));
    q.push_back(Utf8Adjust(ch, 1, 0));
  } else if (ch < 0x10000) {
    q.push_back(Utf8Adjust(ch, 3, 12));
    q.push_back(Utf8Adjust(ch, 1, 6));
    q.push_back(Utf8Adjust(ch, 1, 0));
  } else {
    q.push_back(Utf8Adjust(ch, 4, 18));
    q.push_back(Utf8Adjust(ch, 1, 12));
    q.push_back(Utf8Adjust(ch, 1, 6));
    q.push_back(Utf8Adjust(ch, 1, 0));
  }
}

// Determine (or guess) the character-set by reading the BOM, if any.
// See the YAML specification for the determination algorithm.
// Returns the size of detected BOM
uint_fast8_t Stream::CheckBOM(const uint8_t* buffer, std::size_t size) {
  if (size >= 4) {
    if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0xFE &&
        buffer[3] == 0xFF) {
      m_charSet = utf32be;
      return 4;
    }
    if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0x00) {
      m_charSet = utf32be;
      return 0;
    }

    if (buffer[0] == 0xFF && buffer[1] == 0xFE && buffer[2] == 0x00 &&
        buffer[3] == 0x00) {
      m_charSet = utf32le;
      return 4;
    }
    if (buffer[1] == 0x00 && buffer[2] == 0x00 && buffer[3] == 0x00) {
      m_charSet = utf32le;
      return 0;
    }
  }

  if (size >= 2) {
    if (buffer[0] == 0xFE && buffer[1] == 0xFF) {
      m_charSet = utf16be;
      return 2;
    }
    if (buffer[0] == 0x00) {
      m_charSet = utf16be;
      return 0;
    }

    if (buffer[0] == 0xFF && buffer[1] == 0xFE) {
      m_charSet = utf16le;
      return 2;
    }
    if (buffer[1] == 0x00) {
      m_charSet = utf16le;
      return 0;
    }
  }

  if (size >= 3) {
    if (buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) {
      m_charSet = utf8;
      return 3;
    }
  }

  m_charSet = utf8;
  return 0;
}

Stream::Stream(std::istream& input)
    : m_input(input),
      m_pPrefetched(new uint8_t[YAML_PREFETCH_SIZE]),
      m_nPrefetchedAvailable(0),
      m_nPrefetchedUsed(0),
      m_readahead{},
      m_mark{},
      m_charSet(utf8) {

  if (!input.good())
    return;

  input.read(reinterpret_cast<char*>(m_pPrefetched), YAML_PREFETCH_SIZE);
  m_nPrefetchedAvailable = input.gcount();
  m_nPrefetchedUsed = CheckBOM(m_pPrefetched, m_nPrefetchedAvailable);
}

Stream::~Stream() { delete[] m_pPrefetched; }

bool Stream::isEmpty() const {
  return m_readahead.empty() && m_nPrefetchedUsed >= m_nPrefetchedAvailable &&
         !m_input.good();
}

Stream::operator bool() const { return !isEmpty(); }

char Stream::peek(std::size_t i) const {
  if (prepare(i)) {
    return m_readahead[i];
  } else {
    return Stream::eof();
  }
}

// Extracts a character from the stream and updates our position
char Stream::get() {
  if (prepare(0)) {
    char c = m_readahead.front();
    m_readahead.pop_front();
    m_mark.pos++;
    if (c == '\n') {
      m_mark.column = 0;
      m_mark.line++;
    } else {
      m_mark.column++;
    }
    return c;
  } else {
    return Stream::eof();
  }
}

// Extracts 'n' characters from the stream and updates our position
std::string Stream::get(int n) {
  std::string ret;
  if (n > 0) {
    ret.reserve(static_cast<std::string::size_type>(n));
    for (int i = 0; i < n; i++)
      ret += get();
  }
  return ret;
}

// Eats 'n' characters and updates our position.
void Stream::eat(int n) {
  for (int i = 0; i < n; i++)
    get();
}

bool Stream::prepare(std::size_t i) const {
  while (m_readahead.size() <= i) {
    bool resume;
    switch (m_charSet) {
      case utf8:
        resume = StreamInUtf8();
        break;
      case utf16le:
        resume = StreamInUtf16();
        break;
      case utf16be:
        resume = StreamInUtf16();
        break;
      case utf32le:
        resume = StreamInUtf32();
        break;
      case utf32be:
        resume = StreamInUtf32();
        break;
    }
    if (!resume) {
      break;
    }
  }

  return m_readahead.size() > i;
}

bool Stream::GetNextByte(uint8_t& byte) const {
  if (m_nPrefetchedUsed >= m_nPrefetchedAvailable) {
    if (m_input.good()) {
      m_input.read(reinterpret_cast<char*>(m_pPrefetched), YAML_PREFETCH_SIZE);
      m_nPrefetchedAvailable = m_input.gcount();
      m_nPrefetchedUsed = 0;
    } else {
      m_nPrefetchedAvailable = 0;
    }
  }

  if (m_nPrefetchedAvailable != 0) {
    byte = m_pPrefetched[m_nPrefetchedUsed++];
    return true;
  } else {
    return false;
  }
}

bool Stream::StreamInUtf8() const {
  uint8_t byte;
  if (GetNextByte(byte)) {
    m_readahead.push_back(static_cast<char>(byte));
    return true;
  } else {
    return false;
  }
}

bool Stream::StreamInUtf16() const {
  unsigned long ch = 0;
  uint8_t bytes[2];
  int nBigEnd = (m_charSet == utf16be) ? 0 : 1;

  if (!GetNextByte(bytes[0]) || !GetNextByte(bytes[1])) {
    return false;
  }

  ch = (static_cast<unsigned long>(bytes[nBigEnd]) << 8) |
       static_cast<unsigned long>(bytes[1 ^ nBigEnd]);

  if (ch >= 0xDC00 && ch < 0xE000) {
    // Trailing (low) surrogate...ugh, wrong order
    QueueUnicodeCodepoint(m_readahead, CP_REPLACEMENT_CHARACTER);
    return true;
  }

  if (ch >= 0xD800 && ch < 0xDC00) {
    // ch is a leading (high) surrogate

    // Four byte UTF-8 code point

    // Read the trailing (low) surrogate
    for (;;) {
      if (!GetNextByte(bytes[0]) || !GetNextByte(bytes[1])) {
        return false;
      }
      unsigned long chLow = (static_cast<unsigned long>(bytes[nBigEnd]) << 8) |
                            static_cast<unsigned long>(bytes[1 ^ nBigEnd]);
      if (chLow < 0xDC00 || chLow >= 0xE000) {
        // Trouble...not a low surrogate.  Dump a REPLACEMENT CHARACTER into the
        // stream.
        QueueUnicodeCodepoint(m_readahead, CP_REPLACEMENT_CHARACTER);

        // Deal with the next UTF-16 unit
        if (chLow < 0xD800 || chLow >= 0xE000) {
          // Easiest case: queue the codepoint and return
          QueueUnicodeCodepoint(m_readahead, ch);
          return true;
        }
        // Start the loop over with the new high surrogate
        ch = chLow;
        continue;
      }

      // Select the payload bits from the high surrogate
      ch &= 0x3FF;
      ch <<= 10;

      // Include bits from low surrogate
      ch |= (chLow & 0x3FF);

      // Add the surrogacy offset
      ch += 0x10000;
      break;
    }
  }

  QueueUnicodeCodepoint(m_readahead, ch);
  return true;
}

bool Stream::StreamInUtf32() const {
  static int indexes[2][4] = {{3, 2, 1, 0}, {0, 1, 2, 3}};

  unsigned long ch = 0;
  uint8_t bytes[4];
  int* pIndexes = (m_charSet == utf32be) ? indexes[1] : indexes[0];

  if (!GetNextByte(bytes[0]) || !GetNextByte(bytes[1]) ||
      !GetNextByte(bytes[2]) || !GetNextByte(bytes[3])) {
    return false;
  }

  for (int i = 0; i < 4; ++i) {
    ch <<= 8;
    ch |= bytes[pIndexes[i]];
  }

  QueueUnicodeCodepoint(m_readahead, ch);
  return true;
}

}  // namespace YAML
