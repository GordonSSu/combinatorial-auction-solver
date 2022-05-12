cmake_minimum_required(VERSION 3.2)
project(gurobitest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(gurobitest ${SOURCE_FILES})

if (GUROBI_INCLUDE_DIR)
# in cache already
set(GUROBI_FOUND TRUE)
set(GUROBI_INCLUDE_DIRS "${GUROBI_INCLUDE_DIR}" )
set(GUROBI_LIBRARIES "${GUROBI_LIBRARY};${GUROBI_CXX_LIBRARY}" )
else (GUROBI_INCLUDE_DIR)

find_path(
	GUROBI_INCLUDE_DIR
	NAMES gurobi_c++.h
	PATHS "/Library/gurobi604/mac64/include" "$ENV{GUROBI_HOME}/include" "C:\\libs\\gurobi604\\include"
	#CMAKE_ROOT_PATH "/Library/gurobi604/mac64"
	)

find_library(
	GUROBI_LIBRARY
	NAMES gurobi gurobi45 gurobi46 gurobi50 gurobi51 gurobi52 gurobi55 gurobi56 gurobi60
	libgurobi_c++.a
	libgurobi60.so
				PATHS "$ENV{GUROBI_HOME}/lib"
							"/Library/gurobi604/mac64/lib"
							"/Library/gurobi604/mac64/lib"
							"C:\\libs\\gurobi604\\lib"
				)

find_library( GUROBI_CXX_LIBRARY
				NAMES gurobi_c++
				libgurobi60.so
				libgurobi_c++.a
				PATHS "$ENV{GUROBI_HOME}/lib"
							"/Library/gurobi604/mac64/lib"
							"C:\\libs\\gurobi604\\lib"

				)

set(GUROBI_INCLUDE_DIRS "${GUROBI_INCLUDE_DIR}" )
set(GUROBI_LIBRARIES "${GUROBI_LIBRARY};${GUROBI_CXX_LIBRARY}" )


endif(GUROBI_INCLUDE_DIR)

include_directories(${GUROBI_INCLUDE_DIR})
target_link_libraries(gurobitest ${GUROBI_CXX_LIBRARY})
target_link_libraries(gurobitest ${GUROBI_LIBRARY})
