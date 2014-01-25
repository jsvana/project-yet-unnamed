# Initial setup
cmake_minimum_required(VERSION 2.6)
project(bbs)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Including any needed deps
include(FindPkgConfig)
include(FindMySQL)

# Convenient options
option(CLANG "build application with clang" false)
option(DEBUG "Enable debugging output" true)

# Change the C and C++ compilers to clang if we want
if (CLANG)
	set(CMAKE_C_COMPILER "clang")
	set(CMAKE_CXX_COMPILER "clang++")
endif()

# Make it easy to enable debugging
if(DEBUG)
	set(CMAKE_BUILD_TYPE Debug)
endif()

# Make sure we get all warnings and that we're using C++11
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=gnu99 -Wall -Wno-sign-compare")

# Look up libraries
pkg_check_modules(LUA REQUIRED lua)
find_package(MySQL REQUIRED)

add_executable(
	bbs-server

	src/server_main.c
	src/common.c
	src/bbs_mysql.c
)

add_executable(
	bbs-client
	
	src/client_main.c
	src/common.c
	src/utils/mqueue.c
)

add_executable(
	seed
	
	src/seed.c
	src/common.c
	src/bbs_mysql.c
)

target_link_libraries(
	bbs-server

	${MYSQL_LIBRARIES}
)

target_link_libraries(
	bbs-client

	ncurses
)

target_link_libraries(
	seed

	${MYSQL_LIBRARIES}
)
