#ifndef VALUE_PTR_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define VALUE_PTR_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml-cpp/dll.h"

namespace YAML {
namespace detail {

template <typename T>
struct ref_holder {
  ~ref_holder<T>() {
    if (m_ptr)
      m_ptr->release();
  }
  ref_holder<T>(T* ptr) {
    if (ptr) {
      ptr->hold();
    }
    m_ptr = ptr;
  }

  ref_holder<T>(const ref_holder<T>& ref) {
    if (ref.m_ptr) {
      ref.m_ptr->hold();
    }
    m_ptr = ref.m_ptr;
  }

  ref_holder<T>(ref_holder<T>&& ref) {
    m_ptr = ref.m_ptr;
    ref.m_ptr = nullptr;
  }

  ref_holder<T>& operator=(const ref_holder<T>& ref) {
    if (ref.m_ptr == m_ptr) {
      return *this;
    }
    if (ref.m_ptr) {
      ref.m_ptr->hold();
    }
    if (m_ptr) {
      m_ptr->release();
    }
    m_ptr = ref.m_ptr;
    return *this;
  }

  ref_holder<T>& operator=(ref_holder<T>&& ref) {
    if (ref.m_ptr == m_ptr) {
      return *this;
    }

    if (m_ptr) {
      m_ptr->release();
    }
    m_ptr = ref.m_ptr;
    ref.m_ptr = nullptr;
    return *this;
  }

  bool operator==(const ref_holder<T>& ref) const { return m_ptr == ref.m_ptr; }
  bool operator!=(const ref_holder<T>& ref) const { return m_ptr != ref.m_ptr; }

  const T* operator->() const { return m_ptr; }
  T* operator->() { return m_ptr; }

  const T& operator*() const { return *m_ptr; }
  T& operator*() { return *m_ptr; }

  const T* get() { return m_ptr; }

  void reset(T* ptr) {
    if (ptr == m_ptr) {
      return;
    }
    if (m_ptr) {
      m_ptr->release();
    }
    if (ptr) {
      ptr->hold();
    }
    m_ptr = ptr;
  }

  operator bool() const { return m_ptr != nullptr; }

 private:
  T* m_ptr;
};

struct ref_counted {

  void hold() { m_refs++; }
  void release() {
    if (--m_refs == 0) {
      delete this;
    }
  }
  virtual ~ref_counted() {}

 private:
  std::size_t m_refs = 0;
};
}
}

#endif  // VALUE_PTR_H_62B23520_7C8E_11DE_8A39_0800200C9A66
