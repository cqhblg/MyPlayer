# CMakeList.txt : CMake project for FFmpegCMake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("VideoPlayer" VERSION 0.2.0)

# --------------------------------------------------------------------------
# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")


############################################
# Add source to this project's executable.
############################################
add_executable (${PROJECT_NAME} 
	main.cpp
)


target_include_directories( ${PROJECT_NAME}  
	PRIVATE 
		${PROJECT_SOURCE_DIR}/include

)
target_link_libraries( ${PROJECT_NAME}  
	PRIVATE 
		${PROJECT_SOURCE_DIR}/lib/avcodec.lib
		${PROJECT_SOURCE_DIR}/lib/avformat.lib
		${PROJECT_SOURCE_DIR}/lib/avutil.lib
		${PROJECT_SOURCE_DIR}/lib/avdevice.lib
		${PROJECT_SOURCE_DIR}/lib/avfilter.lib
		${PROJECT_SOURCE_DIR}/lib/postproc.lib
		${PROJECT_SOURCE_DIR}/lib/swresample.lib
		${PROJECT_SOURCE_DIR}/lib/swscale.lib
		${PROJECT_SOURCE_DIR}/lib/SDL2.lib

)


