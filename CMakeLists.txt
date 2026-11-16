cmake_minimum_required(VERSION 3.16)
project(Lincstation CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4 GIT_SHALLOW TRUE)
FetchContent_Declare(glm  GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1 GIT_SHALLOW TRUE)
# FreeType bez opcionih zavisnosti (samo TTF -> bitmapa)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
FetchContent_Declare(freetype GIT_REPOSITORY https://github.com/freetype/freetype.git GIT_TAG VER-2-13-3 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glfw glm freetype)

# GLAD (OpenGL 3.3 core), generisan jednom i ubačen u third_party
add_library(glad STATIC third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC third_party/glad/include)

add_executable(lincstation
  src/main.cpp
  src/App.cpp
  src/gfx/Shader.cpp
  src/gfx/Texture.cpp
  src/gfx/Renderer.cpp
  src/gfx/Text.cpp
  src/model/Machine.cpp
  src/model/Cashier.cpp
  src/ui/DragSystem.cpp
  src/ui/MachineView.cpp)
target_include_directories(lincstation PRIVATE src third_party/stb)
target_link_libraries(lincstation PRIVATE glad glfw glm::glm freetype)
if(MSVC)
  target_compile_options(lincstation PRIVATE /W3)
  set_target_properties(lincstation PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:lincstation>")
else()
  target_compile_options(lincstation PRIVATE -Wall -Wextra)
endif()

# shaderi i asseti idu pored izvršnog fajla (učitavaju se relativno od radnog foldera)
add_custom_command(TARGET lincstation POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:lincstation>/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets  $<TARGET_FILE_DIR:lincstation>/assets)

# headless testovi logike (Machine + Cashier, bez OpenGL-a): ctest ili ./machine_test
enable_testing()
add_executable(machine_test tests/machine_test.cpp src/model/Machine.cpp src/model/Cashier.cpp)
target_include_directories(machine_test PRIVATE src)
add_test(NAME machine COMMAND machine_test)
