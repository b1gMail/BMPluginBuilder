cmake_minimum_required(VERSION 3.0)
project(BMPluginBuilder CXX C)

find_package(ZLIB REQUIRED)

add_executable(bmpluginbuilder
  bmplugincreator.cpp
  main.cpp
  md5.c
)

target_include_directories(bmpluginbuilder PRIVATE ${ZLIB_INCLUDE_DIRS})
target_link_libraries(bmpluginbuilder PRIVATE ${ZLIB_LIBRARIES})
