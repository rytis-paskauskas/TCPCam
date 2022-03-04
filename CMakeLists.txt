cmake_minimum_required(VERSION 3.5)


include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(TCPCam)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "resources/test/rose.jpeg" TEXT)
