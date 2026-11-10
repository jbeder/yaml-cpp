#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>

#include "exp.h"
#include "regex_yaml.h"
#include "yaml-cpp/emitter.h"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/parser.h"
#include "yaml-cpp/yaml.h"

namespace {
std::size_t allocation_count = 0;
std::size_t allocated_bytes = 0;
volatile std::size_t benchmark_sink = 0;

class NullEventHandler : public YAML::EventHandler {
 public:
  void OnDocumentStart(const YAML::Mark&) override {}
  void OnDocumentEnd() override {}

  void OnNull(const YAML::Mark&, YAML::anchor_t) override {}
  void OnAlias(const YAML::Mark&, YAML::anchor_t) override {}
  void OnScalar(const YAML::Mark&, const std::string&, YAML::anchor_t,
                const std::string&) override {}

  void OnSequenceStart(const YAML::Mark&, const std::string&, YAML::anchor_t,
                       YAML::EmitterStyle::value) override {}
  void OnSequenceEnd() override {}

  void OnMapStart(const YAML::Mark&, const std::string&, YAML::anchor_t,
                  YAML::EmitterStyle::value) override {}
  void OnMapEnd() override {}
};

void ResetAllocationCounters() {
  allocation_count = 0;
  allocated_bytes = 0;
}

void* Allocate(std::size_t size) {
  void* result = std::malloc(size);
  if (!result)
    throw std::bad_alloc();
  ++allocation_count;
  allocated_bytes += size;
  return result;
}

template <typename Function>
void RunBenchmark(const char* name, std::size_t iterations,
                  std::size_t bytes_per_iteration, Function function) {
  for (int i = 0; i < 2; ++i)
    benchmark_sink += function();

  ResetAllocationCounters();
  const std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  for (std::size_t i = 0; i < iterations; ++i)
    benchmark_sink += function();
  const std::chrono::steady_clock::time_point end =
      std::chrono::steady_clock::now();

  const std::chrono::nanoseconds elapsed =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  const double elapsed_seconds =
      static_cast<double>(elapsed.count()) / 1000000000.0;
  const double ns_per_operation =
      static_cast<double>(elapsed.count()) / iterations;
  const double megabytes_per_second =
      elapsed_seconds == 0.0 ? 0.0
                             : static_cast<double>(bytes_per_iteration) *
                                   iterations / elapsed_seconds / 1000000.0;

  std::cout << std::left << std::setw(28) << name << std::right
            << " iterations=" << std::setw(8) << iterations
            << " ns/op=" << std::setw(12) << std::fixed << std::setprecision(2)
            << ns_per_operation << " MB/s=" << std::setw(10)
            << megabytes_per_second << " allocations=" << std::setw(8)
            << allocation_count << " bytes=" << std::setw(12) << allocated_bytes
            << "\n";
}

std::string MakePlainPayload(std::size_t record_count) {
  std::string payload("---\nitems:\n");
  for (std::size_t i = 0; i < record_count; ++i) {
    payload += "  item";
    payload += std::to_string(i);
    payload += ": plain scalar value ";
    payload += std::to_string(i % 17);
    payload += "\n";
  }
  return payload;
}

std::string MakeQuotedPayload(std::size_t record_count) {
  std::string payload("---\nitems:\n");
  for (std::size_t i = 0; i < record_count; ++i) {
    payload += "  - \"quoted scalar value ";
    payload += std::to_string(i % 17);
    payload += "\"\n";
  }
  return payload;
}

std::string MakeNestedPayload(std::size_t record_count) {
  std::string payload("---\nroot:\n");
  for (std::size_t i = 0; i < record_count; ++i) {
    payload += "  group";
    payload += std::to_string(i);
    payload += ":\n    name: nested value\n    count: ";
    payload += std::to_string(i);
    payload += "\n";
  }
  return payload;
}

std::size_t ParseWithEvents(const std::string& payload) {
  std::istringstream input(payload);
  YAML::Parser parser(input);
  NullEventHandler handler;
  parser.HandleNextDocument(handler);
  return payload.size();
}

std::size_t EmitNode(const YAML::Node& node) {
  YAML::Emitter emitter;
  emitter << node;
  if (!emitter.good())
    throw std::runtime_error(emitter.GetLastError());
  return emitter.size();
}

std::size_t ParseIterations(const char* value) {
  char* end = nullptr;
  const unsigned long parsed = std::strtoul(value, &end, 10);
  if (*value == '\0' || end == value || *end != '\0' || parsed == 0)
    throw std::invalid_argument("iterations must be a positive integer");
  return static_cast<std::size_t>(parsed);
}
}  // namespace

void* operator new(std::size_t size) { return Allocate(size); }

void* operator new[](std::size_t size) { return Allocate(size); }

void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
  try {
    return Allocate(size);
  } catch (const std::bad_alloc&) {
    return nullptr;
  }
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
  try {
    return Allocate(size);
  } catch (const std::bad_alloc&) {
    return nullptr;
  }
}

void operator delete(void* pointer) noexcept { std::free(pointer); }

void operator delete[](void* pointer) noexcept { std::free(pointer); }

int main(int argc, char** argv) {
  try {
    std::size_t iterations = 100;
    if (argc == 3 && std::string(argv[1]) == "--iterations") {
      iterations = ParseIterations(argv[2]);
    } else if (argc != 1) {
      std::cerr << "Usage: regex_benchmark [--iterations N]\n";
      return 2;
    }

    const std::string plain_payload = MakePlainPayload(512);
    const std::string quoted_payload = MakeQuotedPayload(512);
    const std::string nested_payload = MakeNestedPayload(256);
    const std::string scalar_input = "plain scalar value";
    const std::string document_start = "--- ";

    const YAML::RegEx plain_scalar =
        YAML::MakeRegEx<YAML::Exp::Patterns::PlainScalar>();
    const YAML::RegEx document_start_regex =
        YAML::MakeRegEx<YAML::Exp::Patterns::DocumentStart>();

    RunBenchmark("regex plain match", iterations, 1, [&]() {
      return static_cast<std::size_t>(plain_scalar.Match(scalar_input) >= 0);
    });
    RunBenchmark("regex document match", iterations, document_start.size(),
                 [&]() {
                   return static_cast<std::size_t>(
                       document_start_regex.Match(document_start) >= 0);
                 });
    RunBenchmark("regex construction", iterations, 0, [&]() {
      YAML::RegEx regex =
          YAML::MakeRegEx<YAML::CharSet<'a', 'b', 'c', 'd', 'e', 'f'>>();
      return static_cast<std::size_t>(regex.Match("a") >= 0);
    });
    RunBenchmark("parser plain payload", iterations, plain_payload.size(),
                 [&]() { return ParseWithEvents(plain_payload); });
    RunBenchmark("parser quoted payload", iterations, quoted_payload.size(),
                 [&]() { return ParseWithEvents(quoted_payload); });
    RunBenchmark("parser nested payload", iterations, nested_payload.size(),
                 [&]() { return ParseWithEvents(nested_payload); });

    const YAML::Node emitter_node = YAML::Load(plain_payload);
    RunBenchmark("emitter plain payload", iterations, plain_payload.size(),
                 [&]() { return EmitNode(emitter_node); });

    std::cerr << "sink=" << benchmark_sink << "\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "regex_benchmark: " << error.what() << "\n";
    return 1;
  }
}
