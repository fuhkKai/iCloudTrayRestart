cmake_minimum_required(VERSION 3.21)
project(iCloudTrayRestart LANGUAGES C CXX)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "iCloudTrayRestart requires an x64 Windows toolchain")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_library(minhook STATIC
  third_party/minhook/src/buffer.c
  third_party/minhook/src/hook.c
  third_party/minhook/src/trampoline.c
  third_party/minhook/src/hde/hde32.c
  third_party/minhook/src/hde/hde64.c
)
target_include_directories(minhook PUBLIC third_party/minhook/include)

add_library(ICloudTrayMenu SHARED src/tray_menu/dllmain.cpp)
target_include_directories(ICloudTrayMenu PRIVATE
  src/common
  third_party/minhook/include
)
target_link_libraries(ICloudTrayMenu PRIVATE minhook user32 windowsapp)
set_target_properties(ICloudTrayMenu PROPERTIES PREFIX "")

add_executable(ICloudTrayBridge WIN32 src/bridge/main.cpp)
target_include_directories(ICloudTrayBridge PRIVATE src/common)
target_compile_definitions(ICloudTrayBridge PRIVATE UNICODE _UNICODE)
target_link_libraries(ICloudTrayBridge PRIVATE shell32)

# Visual Studio otherwise adds bin/Release or bin/Debug, which Install.cmd
# does not look in. DLLs count as runtime artifacts on Windows.
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  set_target_properties(ICloudTrayMenu ICloudTrayBridge PROPERTIES
    "RUNTIME_OUTPUT_DIRECTORY_${config}" "${CMAKE_SOURCE_DIR}/bin")
endforeach()
