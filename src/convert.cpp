#include <algorithm>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>

#include "yaml-cpp/exceptions.h"
#include "yaml-cpp/node/convert.h"

namespace {
// we're not gonna mess with the mess that is all the isupper/etc. functions
bool IsLower(char ch) { return 'a' <= ch && ch <= 'z'; }
bool IsUpper(char ch) { return 'A' <= ch && ch <= 'Z'; }
char ToLower(char ch) { return IsUpper(ch) ? ch + 'a' - 'A' : ch; }

std::string tolower(const std::string& str) {
  std::string s(str);
  std::transform(s.begin(), s.end(), s.begin(), ToLower);
  return s;
}

template <typename T>
bool IsEntirely(const std::string& str, T func) {
  for (std::size_t i = 0; i < str.size(); i++)
    if (!func(str[i]))
      return false;

  return true;
}

// IsFlexibleCase
// . Returns true if 'str' is:
//   . UPPERCASE
//   . lowercase
//   . Capitalized
bool IsFlexibleCase(const std::string& str) {
  if (str.empty())
    return true;

  if (IsEntirely(str, IsLower))
    return true;

  bool firstcaps = IsUpper(str[0]);
  std::string rest = str.substr(1);
  return firstcaps && (IsEntirely(rest, IsLower) || IsEntirely(rest, IsUpper));
}
}

namespace YAML {
bool convert<bool>::decode(const Node& node, bool& rhs) {
  if (!node.IsScalar())
    return false;

  // we can't use iostream bool extraction operators as they don't
  // recognize all possible values in the table below (taken from
  // http://yaml.org/type/bool.html)
  static const struct {
    std::string truename, falsename;
  } names[] = {
      {"y", "n"}, {"yes", "no"}, {"true", "false"}, {"on", "off"},
  };

  if (!IsFlexibleCase(node.Scalar()))
    return false;

  for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
    if (names[i].truename == tolower(node.Scalar())) {
      rhs = true;
      return true;
    }

    if (names[i].falsename == tolower(node.Scalar())) {
      rhs = false;
      return true;
    }
  }

  return false;
}
}

namespace YAML {
Node convert<std::chrono::time_point<std::chrono::system_clock>>::encode(
  const std::chrono::time_point<std::chrono::system_clock>& rhs
) {
  // Constants
  constexpr uint16_t MS_PER_S = 1000;
  constexpr uint16_t TM_BASE_YEAR = 1900;
  
  std::time_t tt = std::chrono::system_clock::to_time_t(rhs);
  std::tm utc_tm = *std::gmtime(&tt);
  
  using namespace std::chrono;
  uint16_t ms = duration_cast<milliseconds>(rhs.time_since_epoch())
    .count() % MS_PER_S;

  std::stringstream canonical;
  {
  using namespace std;
  canonical << setw(4) << setfill('0') << TM_BASE_YEAR + utc_tm.tm_year << "-"
            << setw(2) << setfill('0') << utc_tm.tm_mon + 1 << "-"
            << setw(2) << setfill('0') << utc_tm.tm_mday << "T"
            << setw(2) << setfill('0') << utc_tm.tm_hour << ":"
            << setw(2) << setfill('0') << utc_tm.tm_min << ":"
            << std::setw(2) << std::setfill('0') << utc_tm.tm_sec << "."
            << ms / 100 << "Z";
  }
  return Node(canonical.str());
}
    
bool convert<std::chrono::time_point<std::chrono::system_clock>>::decode(
  const Node& node, 
  std::chrono::time_point<std::chrono::system_clock>& rhs
) {
  
  if (!node.IsScalar()) { return false; }
  
  // Constants
  constexpr uint16_t TM_BASE_YEAR = 1900;
  constexpr uint16_t MIN_PER_HR = 60;
  constexpr uint16_t MS_PER_S = 1000;
  constexpr uint16_t S_PER_MIN = 60;
  constexpr uint16_t S_PER_HR = 3600;
  constexpr uint32_t S_PER_DAY = 86400;
  
  const std::string s = node.Scalar();
    
  // Working variables.
  std::tm l_tm;
  std::time_t l_time_t;
  uint16_t year, month, day, hour = 0, minute = 0;
  double second = 0.0f, zone = 0.0f;

  // Regex patterns for date, time, and timezone.
  std::regex r_yr("[0-9][0-9][0-9][0-9]-[0-9][0-9]?-[0-9][0-9]?");
  std::regex r_tm("[0-9][0-9]?:[0-9][0-9]:[0-9][0-9].[0-9][0-9]?");
  std::regex r_zn("[Z]|[+|-][0-9][0-9]:[0-9][0-9]| [+|-][0-9]");
  std::smatch m;

  // Parse the date string.
  if(std::regex_search(s.begin(), s.end(), m, r_yr)) {
    std::stringstream ss(m[0]);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, '-')) {
       elems.push_back(item);
    }
    year =  std::stoi(elems[0]);  // Year
    month = std::stoi(elems[1]);  // Month
    day =   std::stoi(elems[2]);  // Day
  }
  else {
    // No date found in the string.
    throw YAML::TypedBadConversion<
      std::chrono::time_point<std::chrono::system_clock>>(node.Mark());
  }

  // Parse the time string.
  if(std::regex_search(s.begin(), s.end(), m, r_tm)) {
    std::stringstream ss(m[0]);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, ':')) {
      elems.push_back(item);
    }
    hour =   std::stoi(elems[0]); // Hours
    minute = std::stoi(elems[1]); // Minutes
    second = std::stod(elems[2]); // Seconds
  }

  // Parse the timezone string.
  if(std::regex_search(s.begin(), s.end(), m, r_zn)) {
    std::string z = m[0];
      // Declared Zulu.
    if(z.find('Z') != std::string::npos) {
      zone = 0.0f;
    } // Potentially non-whole hour.
    else if (z.find(':') != std::string::npos) {
      std::stringstream ss(m[0]);
      std::string item;
      std::vector<std::string> elems;
      while (std::getline(ss, item, ':')) {
        elems.push_back(item);
      }
      zone = stod(elems[0]);  // Apply whole hours.
      zone > 0                // Apply partial hours.
        ? zone += stod(elems[1]) / MIN_PER_HR
        : zone -= stod(elems[1]) / MIN_PER_HR;
    } // Assumed Zulu.
    else {
      zone = stod(z);
    }
  }

  // Get the date stored avoiding mktime's problematic local-time return.
  l_tm.tm_mday = day;
  l_tm.tm_mon = month - 1;
  l_tm.tm_year = year - TM_BASE_YEAR;
  l_tm.tm_hour = 12;  // Noon
  l_tm.tm_min = 0;
  l_tm.tm_sec = 0;
  l_time_t = std::mktime(&l_tm);
  if(l_time_t == -1) {
    // Invalid std::time_t value parsed from the std::tm;
    throw YAML::TypedBadConversion<
      std::chrono::time_point<std::chrono::system_clock>>(node.Mark());
  }
  l_time_t -= static_cast<time_t>(l_time_t % S_PER_DAY);

  // Add time of day in seconds having dodged mktime's annoyances.
  l_time_t += static_cast<time_t>(
    // Hours             Minutes               Seconds
    (hour * S_PER_HR) + (minute * S_PER_MIN) + second
  );
  
  // Adjust for the time zone;
  l_time_t -= static_cast<time_t>(zone * S_PER_HR);

  // Create time_point.
  using namespace std::chrono;
  time_point<system_clock> l_time_point = system_clock::from_time_t(l_time_t);

  // Add milliseconds.
  uint32_t ms = static_cast<uint32_t>(second * MS_PER_S) % MS_PER_S;
  std::chrono::milliseconds dur_ms{ms};
  l_time_point += duration_cast<system_clock::duration>(dur_ms);
  
  // Set rhs.
  rhs = l_time_point;

  return true;
}

}
