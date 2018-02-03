import qbs
import qbs.File

Product {
    type: "staticlibrary"

    name: "Yaml"
    targetName: "yaml"

    Depends { name: "cpp" }

    cpp.archiverName: {
        // If is used not system default compiler
        return File.exists(cpp.toolchainPathPrefix + "/gcc-ar") ? "gcc-ar" : "ar";
    }
    cpp.cxxFlags: [
        "-std=c++11",
        "-ggdb3",
        "-Wall",
        "-Wextra",
        "-Wno-unused-parameter",
    ]
    cpp.includePaths: ["include"]

    files: [
        "include/yaml-cpp/*.h",
        "include/yaml-cpp/contrib/*.h",
        "include/yaml-cpp/node/*.h",
        "include/yaml-cpp/node/detail/*.h",
        "src/*.cpp",
        "src/*.h",
    ]
    Export {
        Depends { name: "cpp" }
        cpp.systemIncludePaths: ["include"]
    }
}
