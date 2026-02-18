#ifndef DLL_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define DLL_H_62B23520_7C8E_11DE_8A39_0800200C9A66

// Definition YAML_CPP_STATIC_DEFINE using to building YAML-CPP as static
// library (definition created by CMake or defined manually)

// Definition yaml_cpp_EXPORTS using to building YAML-CPP as dll/so library
// (definition created by CMake or defined manually)

#ifndef YAML_CPP_API
#  ifdef YAML_CPP_STATIC_DEFINE
#    define YAML_CPP_API
#  else
#    ifdef _WIN32
#      ifdef yaml_cpp_EXPORTS
         /* We are building this library */
#        define YAML_CPP_API __declspec(dllexport)
#      else
         /* We are using this library */
#        define YAML_CPP_API __declspec(dllimport)
#      endif
#    else
#      define YAML_CPP_API __attribute__((visibility("default")))
#    endif /* _WIN32 */
#  endif /* YAML_CPP_STATIC_DEFINE */
#endif

#endif /* DLL_H_62B23520_7C8E_11DE_8A39_0800200C9A66 */
