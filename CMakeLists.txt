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
  fhrus_holiday
  wiesbaden_accent_20150127
  fra_pi_20150127
  fra_casino_20150127
  )

foreach(TEST ${FHRUS_TESTS})
  add_test(NAME ${TEST}
    COMMAND python ${CMAKE_SOURCE_DIR}/test/compare.py
                   --exe ./fhrus2openmensa
                   --ef=--year --ef=2015
                   --inp ${CMAKE_SOURCE_DIR}/test/in/${TEST}.xml
                   --out ${CMAKE_BINARY_DIR}/test/out/${TEST}.xml
                   --ref ${CMAKE_SOURCE_DIR}/test/ref/${TEST}.xml
  )
endforeach()

set(UNIBI_TESTS
  unibi_20150119
  unibi_20150112
  unibi_holiday
  unibi_noname
  )
foreach(TEST ${UNIBI_TESTS})
  add_test(NAME ${TEST}
    COMMAND python ${CMAKE_SOURCE_DIR}/test/compare.py
                   --exe ./unibi2openmensa
                   --inp ${CMAKE_SOURCE_DIR}/test/in/${TEST}.xml
                   --out ${CMAKE_BINARY_DIR}/test/out/${TEST}.xml
                   --ref ${CMAKE_SOURCE_DIR}/test/ref/${TEST}.xml
  )
endforeach()

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
  DEPENDS
  fhrus2openmensa
  unibi2openmensa
)
