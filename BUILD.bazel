cc_library(
    name = "yaml-cpp_internal",
    hdrs = glob(["src/**/*.h"]),
    strip_include_prefix = "src",
    visibility = ["//:__subpackages__"],
)

cc_library(
    name = "yaml-cpp",
    srcs = glob([
        "src/**/*.cpp",
        "src/**/*.h",
    ]),
    hdrs = glob(["include/**/*.h"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
