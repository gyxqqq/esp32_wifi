# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hello_world)
file(COPY build/hello_world.bin DESTINATION output/hello_world)
file(COPY build/hello_world.elf DESTINATION output/hello_world)
file(COPY build/hello_world.map DESTINATION output/hello_world)
file(COPY build/bootloader/bootloader.bin DESTINATION output/bootloader)
file(COPY build/bootloader/bootloader.elf DESTINATION output/bootloader)
file(COPY build/bootloader/bootloader.map DESTINATION output/bootloader)
