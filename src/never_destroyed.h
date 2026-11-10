#ifndef NEVER_DESROYED_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define NEVER_DESROYED_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#include <cstdint>
#include <utility>

namespace YAML {

// Wraps an underlying type T such that its storage is a direct member field of
// this object (i.e., without any indirection into the heap), but *unlike* most
// member fields T's destructor is never invoked. This is especially useful for
// function-local static variables that are not trivially destructable, because
// we shouldn't call their destructor at program exit because of the "static
// initialization order fiasco" (https://en.cppreference.com/cpp/language/siof)
// where the same logic applies to the indeterminate order of destruction.
template <typename T>
class never_destroyed {
 public:
  // Passes the constructor arguments along to T using perfect forwarding.
  template <typename... Args>
  explicit never_destroyed(Args&&... args) {
    new (&storage_) T(std::forward<Args>(args)...);
  }

  // Non-copyable.
  never_destroyed(const never_destroyed&) = delete;
  void operator=(const never_destroyed&) = delete;
  never_destroyed(never_destroyed&&) = delete;
  void operator=(never_destroyed&&) = delete;

  // No-op.
  ~never_destroyed() = default;

  // Returns the underlying T reference.
  operator const T&() const { return *reinterpret_cast<const T*>(&storage_); }

 private:
  alignas(T) std::uint8_t storage_[sizeof(T)];
};

}  // namespace YAML

#endif  // NEVER_DESROYED_H_62B23520_7C8E_11DE_8A39_0800200C9A66
