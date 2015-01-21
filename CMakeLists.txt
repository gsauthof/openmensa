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
  utility.cc
  )
set_property(TARGET fhrus2openmensa PROPERTY INCLUDE_DIRECTORIES
  ${LIBXML_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )
target_link_libraries(fhrus2openmensa
  ${LIBXML_LIBRARIES}
  ${Boost_LIBRARIES}
  )

add_executable(unibi2openmensa
  unibi2openmensa.cc
  utility.cc
  )
set_property(TARGET unibi2openmensa PROPERTY INCLUDE_DIRECTORIES
  ${LIBXML_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )
target_link_libraries(unibi2openmensa
  ${LIBXML_LIBRARIES}
  ${Boost_LIBRARIES}
  )

enable_testing()

set(FHRUS_TESTS
  fhrus_20150112
  fhrus_20150119
  )

foreach(TEST ${FHRUS_TESTS})
  add_test(NAME ${TEST}
    COMMAND bash ${CMAKE_SOURCE_DIR}/test/compare.sh
                   ./fhrus2openmensa
                   ${CMAKE_SOURCE_DIR}/test/in/${TEST}.xml
                   ${CMAKE_SOURCE_DIR}/test/out/${TEST}.xml
                   ${CMAKE_SOURCE_DIR}/test/ref/${TEST}.xml
  )
endforeach()

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
  DEPENDS
  fhrus2openmensa
)
