cmake_minimum_required(VERSION 2.8)
project( ComputerVision )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( ComputerVision DetectarObjColor.cpp )
target_link_libraries( ComputerVision ${OpenCV_LIBS} )