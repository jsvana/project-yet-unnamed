# - Find MySQL
# Find the MySQL includes and client library
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_FOUND, If false, do not try to use MySQL.
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
# Lot of adustmens by Michal Cihar <michal@cihar.com>
# Even more adjustments by Kaleb Elwert <kelwert@mtu.edu>
# Comes from https://www.assembla.com/code/Sakar/subversion/nodes/cmake/FindMySql.cmake
#
# Redistribution and use is allowed according to the terms of the BSD license.

if (NOT DEFINED MYSQL_FOUND)
	if (UNIX)
		set(MYSQL_CONFIG_PREFER_PATH "$ENV{MYSQL_HOME}/bin" CACHE FILEPATH
			"preferred path to MySQL (mysql_config)")

		find_program(MYSQL_CONFIG mysql_config
			${MYSQL_CONFIG_PREFER_PATH}
			/usr/local/mysql/bin/
			/usr/local/bin/
			/usr/bin/
			)

		if (MYSQL_CONFIG)
			message(STATUS "Using mysql-config: ${MYSQL_CONFIG}")
			# set INCLUDE_DIR
			exec_program(${MYSQL_CONFIG}
				ARGS --include
				OUTPUT_VARIABLE MY_TMP)

			string(REGEX REPLACE "-I([^ ]*)( .*)?" "\\1" MY_TMP "${MY_TMP}")

			set(MYSQL_ADD_INCLUDE_DIR ${MY_TMP} CACHE FILEPATH INTERNAL)

			# set LIBRARY_DIR
			exec_program(${MYSQL_CONFIG}
				ARGS --libs_r
				OUTPUT_VARIABLE MY_TMP)

			set(MYSQL_ADD_LIBRARIES "")

			string(REGEX MATCHALL "-l[^ ]*" MYSQL_LIB_LIST "${MY_TMP}")
			foreach (LIB ${MYSQL_LIB_LIST})
				string(REGEX REPLACE "[ ]*-l([^ ]*)" "\\1" LIB "${LIB}")
				list(APPEND MYSQL_ADD_LIBRARIES "${LIB}")
			endforeach()

			set(MYSQL_ADD_LIBRARY_PATH "")

			string(REGEX MATCHALL "-L[^ ]*" MYSQL_LIBDIR_LIST "${MY_TMP}")
			foreach(LIB ${MYSQL_LIBDIR_LIST})
				string(REGEX REPLACE "[ ]*-L([^ ]*)" "\\1" LIB "${LIB}")
				list(APPEND MYSQL_ADD_LIBRARY_PATH "${LIB}")
			endforeach()

		else()
			set(MYSQL_ADD_LIBRARIES "")
			list(APPEND MYSQL_ADD_LIBRARIES "mysqlclient")
		endif()
	else()
		set(MYSQL_ADD_INCLUDE_DIR "c:/msys/local/include" CACHE FILEPATH INTERNAL)
		set(MYSQL_ADD_LIBRARY_PATH "c:/msys/local/lib" CACHE FILEPATH INTERNAL)
	endif()

	find_path(MYSQL_INCLUDE_DIR mysql.h
		/usr/local/include
		/usr/local/include/mysql
		/usr/local/mysql/include
		/usr/local/mysql/include/mysql
		/usr/include
		/usr/include/mysql
		${MYSQL_ADD_INCLUDE_DIR}
		)

	set(TMP_MYSQL_LIBRARIES "")

	foreach(LIB ${MYSQL_ADD_LIBRARIES})
		find_library("MYSQL_LIBRARIES_${LIB}" NAMES ${LIB}
			PATHS
			${MYSQL_ADD_LIBRARY_PATH}
			/usr/lib/mysql
			/usr/local/lib
			/usr/local/lib/mysql
			/usr/local/mysql/lib
			)
		list(APPEND TMP_MYSQL_LIBRARIES "${MYSQL_LIBRARIES_${LIB}}")
	endforeach()

	set(MYSQL_LIBRARIES ${TMP_MYSQL_LIBRARIES} CACHE FILEPATH INTERNAL)

	if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
		set(MYSQL_FOUND TRUE CACHE INTERNAL "MySQL found")
		#message(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
	else()
		set(MYSQL_FOUND FALSE CACHE INTERNAL "MySQL found")
		#message(STATUS "MySQL not found.")
	endif()

	mark_as_advanced(MYSQL_INCLUDE_DIR MYSQL_LIBRARIES)
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(MySQL DEFAULT_MSG MYSQL_LIBRARIES MYSQL_INCLUDE_DIR)
endif()
