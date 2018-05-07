cmake_minimum_required(VERSION 2.6)
project(workPiece)

set(PREFIX "$ENV{NUTHOME}")
set(NUT_HOME_PREFIX "$ENV{HOME}")
set(CMAKE_INSTALL_PREFIX "$ENV{NUTHOME}")

include_directories(
	./
	/usr/include/
	${PREFIX}/include/finch-inc
	${PREFIX}/include/mqtt
	${PREFIX}/include/kiv
	${PREFIX}/include
	${PROJECT_SOURCE_DIR}/json
)

link_directories(
  ${PREFIX}/lib
)


set(srcdirs
  ./
)

ADD_DEFINITIONS(-DJSON_IS_AMALGAMATION)

FIND_PACKAGE(wxWidgets REQUIRED core base adv)
INCLUDE(${wxWidgets_USE_FILE})

foreach(srcdir ${srcdirs})
  include_directories(${srcdir})
  aux_source_directory(${srcdir} source_files)
endforeach()


ADD_LIBRARY(workPiece SHARED ${source_files} )
TARGET_LINK_LIBRARIES(workPiece localMQ paho-mqtt3as finch pthread curl  ${wxWidgets_LIBRARIES})
#testauthorize
INSTALL(TARGETS workPiece DESTINATION lib/fc_plugin)