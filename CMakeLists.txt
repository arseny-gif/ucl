cmake_minimum_required(VERSION 3.5.0)
project(pvzucl VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(/W0)

file(GLOB PVZCLASS_ROOT pvzclass/pvzclass/*.cpp)
file(GLOB PVZCLASS_CLASSES pvzclass/pvzclass/Classes/*.cpp)
file(GLOB PVZCLASS_ENUMS pvzclass/pvzclass/Enums/*.cpp)
file(GLOB PVZCLASS_EVENTS pvzclass/pvzclass/Events/*.cpp)

add_library(pvzclass
    ${PVZCLASS_ROOT}
    ${PVZCLASS_CLASSES}
    ${PVZCLASS_ENUMS}
    ${PVZCLASS_EVENTS}
)

include_directories(pvzucl
    pvzclass/pvzclass
)

set(wxUSE_REGEX OFF)
set(wxUSE_ZLIB builtin)
set(wxUSE_EXPAT builtin)
set(wxUSE_LIBJPEG OFF)
set(wxUSE_LIBPNG builtin)
set(wxUSE_LIBTIFF OFF)
set(wxUSE_NANOSVG OFF)

add_subdirectory("C:\\Program Files\\wxWidgets-3.2.6" "wxWidgets")

add_executable(pvzucl WIN32
    src/main.cpp
    src/app.cpp
    build/app.o
)

target_link_libraries(pvzucl
    pvzclass
    wx::core wx::base
)