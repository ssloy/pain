Include(FetchContent)

set(FETCHCONTENT_QUIET false)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.3.1
  GIT_SHALLOW true
  TLS_VERIFY ON
  INACTIVITY_TIMEOUT 60
  )

FetchContent_MakeAvailable(Catch2)

FILE(GLOB SRCTEST *.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GMPXX REQUIRED gmpxx)
message(STATUS "GMPXX Found: ${GMPXX_FOUND}")

add_executable(pain-test-all ${SRCTEST})

target_link_libraries(pain-test-all PRIVATE ${CMAKE_DL_LIBS} pain Catch2::Catch2WithMain ${GMPXX_LIBRARIES})

include(Catch)
catch_discover_tests(pain-test-all)

