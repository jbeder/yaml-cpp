#ifndef EXPECTED_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define EXPECTED_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif


#include <cassert>
#include <cstdint>
#include <type_traits>

namespace YAML {

struct unexpected {};

template <typename T>
struct expected {
    alignas(T) std::array<std::uint8_t, sizeof(T)> storage;
    T* ptr{};

    expected() = default;
    expected(expected const& o) = delete;
    expected(expected&& o) noexcept(std::is_nothrow_move_constructible<T>::value) {
        if (!o.ptr) return;
        emplace(*o.ptr);
    }
    template <typename... Args>
    expected(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value) {
        emplace(std::forward<Args>(args)...);
    }
    expected(unexpected const&) noexcept {}
    expected(unexpected&&) noexcept {}


    ~expected() noexcept {
        reset();
    }

    auto operator=(expected const& o) -> expected& = delete;
    auto operator=(expected&& o) noexcept(std::is_nothrow_move_assignable<T>::value) -> expected& {
        if (!o.ptr) {
            reset();
        } else if (!ptr) {
            emplace(std::move(*o.ptr));
        } else {
            *ptr = std::move(*o.ptr);
        }
        return *this;
    }
    auto operator=(unexpected const&) noexcept -> expected& {
        reset();
        return *this;
    }

    void reset() noexcept {
        if (!ptr) return;
        ptr->~T();
        ptr = nullptr;
    }

    template <typename... Args>
    auto emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value) -> T& {
        reset();
        ptr = new (storage.data()) T(std::forward<Args>(args)...);
        return *ptr;
    }

    operator bool() const noexcept {
        return ptr != nullptr;
    }

    auto operator*() noexcept -> T& {
        assert(ptr);
        return *ptr;
    }
    auto operator*() const noexcept -> T const& {
        assert(ptr);
        return *ptr;
    }
};

}

#endif  // EXPECTED_H_62B23520_7C8E_11DE_8A39_0800200C9A66
