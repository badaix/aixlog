#   __   __  _  _  __     __    ___ 
#  / _\ (  )( \/ )(  )   /  \  / __)
# /    \ )(  )  ( / (_/\(  O )( (_ \
# \_/\_/(__)(_/\_)\____/ \__/  \___/

# This file is part of aixlog
# Copyright (C) 2017  Johannes Pohl

# This software may be modified and distributed under the terms
# of the MIT license.  See the LICENSE file for details.

cmake_minimum_required(VERSION 2.4) 

project(aixlog_test)

set(CMAKE_CXX_STANDARD 11)

include_directories(
	"include"
)

add_executable(aixlog_test
	aixlog_test.cpp
)


SET(CMAKE_INSTALL_INCLUDEDIR include CACHE PATH "Output directory for header files")
		
install(FILES include/aixlog.hpp DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
