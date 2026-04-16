cmake_minimum_required(VERSION 3.29.1)

project(skia-dev)


set(SKIA_INCLUDE_DIR "C://skia")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SKIA_LIB 
"C:\\skia\\out\\Release-windows-x64\\skia.lib")

include_directories(${SKIA_INCLUDE_DIR})


add_executable(main main.cpp)


target_link_libraries(main  
${SKIA_LIB}
gdi32
user32
opengl32)