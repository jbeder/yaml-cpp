# yaml-cpp

yaml-cpp is a [YAML](http://www.yaml.org/) parser and emitter in C++ matching the [YAML 1.2 spec](http://www.yaml.org/spec/1.2/spec.html).

To get a feel for how it can be used, see the [Tutorial](TODO). For the old API (version < 0.5.0), see [How To Parse A Document](TODO) or [How to Emit YAML](TODO).


# Problems? #

If you find a bug, post an [issue](https://github.com/jbeder/yaml-cpp/issues)! If you have questions about how to use yaml-cpp, please post it on http://stackoverflow.com and tag it `yaml-cpp`.

# How to Build #

yaml-cpp uses [CMake](http://www.cmake.org) to support cross-platform building. The basic steps to build are:

1. Download and install [CMake](http://www.cmake.org) (Resources -> Download).

**Note:** If you don't use the provided installer for your platform, make sure that you add CMake's bin folder to your path.

2. Navigate into the source directory, and type:

```
mkdir build
cd build
```

3. Run CMake. The basic syntax is:

```
cmake [-G generator] [-DBUILD_SHARED_LIBS=ON|OFF] ..
```

  * The `generator` is whatever type of build system you'd like to use. To see a full list of generators on your platform, just run `cmake` (with no arguments). For example:
    * On Windows, you might use "Visual Studio 12 2013" to generate a Visual Studio 2013 solution
    * On OS X, you might use "Xcode" to generate an Xcode project
    * On a UNIX-y system, simply omit the option to generate a makefile

  * yaml-cpp defaults to building a static library, but you may build a shared library by specifying `-DBUILD_SHARED_LIBS=ON`.

  * For more options on customizing the build, see the [CMakeLists.txt](https://github.com/jbeder/yaml-cpp/blob/master/CMakeLists.txt) file.

4. Build it!

5. To clean up, just remove the `build` directory.

# Recent Release #

[yaml-cpp 0.5.2](TODO) has been released! This is a bug fix release.

[yaml-cpp 0.3.0](TODO) is still available if you want the old API.

**The old API will continue to be supported, and will still receive bugfixes!** The 0.3.x and 0.4.x versions will be old API releases, and 0.5.x and above will all be new API releases.
