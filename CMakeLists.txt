cmake_minimum_required(VERSION 3.7)
project(config)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES
        main.c
        config/config.c
        config/config.h
        config/list.c
        config/list.h
        config/nvs.c config/nvs.h)
add_executable(config ${SOURCE_FILES})

#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")