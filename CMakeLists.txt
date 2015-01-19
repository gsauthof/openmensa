cmake_minimum_required(VERSION 2.8.10)

project(openmensa CXX)

find_package(Boost 1.53
  COMPONENTS
    system
    regex
    date_time
  REQUIRED)

include(FindPkgConfig)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-local-typedefs -std=c++11")

pkg_search_module(LIBXML REQUIRED libxml++-2.6)

add_executable(fhrus2openmensa
  fhrus2openmensa.cc
  )
set_property(TARGET fhrus2openmensa PROPERTY INCLUDE_DIRECTORIES
  ${LIBXML_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )
target_link_libraries(fhrus2openmensa
  ${LIBXML_LIBRARIES}
  ${Boost_LIBRARIES}
  )
