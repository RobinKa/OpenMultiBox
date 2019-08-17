# CMakeList.txt : CMake project for OpenMultiBox, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

file(GLOB sourceFiles "source/*.cpp")

# Add source to this project's executable.
add_executable (OpenMultiBox ${sourceFiles})
include_directories("include")

target_link_libraries(OpenMultiBox Qt5::Widgets)

# TODO: Add tests and install targets if needed.

configure_file("settings.json" "settings.json" COPYONLY)