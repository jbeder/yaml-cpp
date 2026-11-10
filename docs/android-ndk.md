# Android NDK notes (shared STL)

## Problem

Mixing a prebuilt `libyaml-cpp.so` with an Android app that uses NDK `ANDROID_STL=c++_shared` can load **two C++ runtimes** in one process ("double `libc++`"). This often causes load/link failures or crashes when STL types cross `.so` boundaries.

## Recommended rebuild

Build `yaml-cpp` as a shared library with the **same** shared STL as the host app:

```bash
cmake -S . -B build-android \
  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-30 \
  -DANDROID_STL=c++_shared \
  -DYAML_BUILD_SHARED_LIBS=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-fvisibility=hidden -fvisibility-inlines-hidden"

cmake --build build-android --config Release
```

## Notes

1. Ship only **one** `libc++_shared.so` in the APK (prefer the NDK/app copy).
2. With `-fvisibility=hidden`, rely on `include/yaml-cpp/dll.h` (`YAML_CPP_API` → default visibility) so public symbols remain exported.
3. Verified against tag `yaml-cpp-0.6.3`; the same `ANDROID_STL=c++_shared` approach applies to newer releases.

Related discussion: https://github.com/jbeder/yaml-cpp/issues/1496
