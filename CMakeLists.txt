cmake_minimum_required(VERSION 3.20)
project(os_three_easy_pieces C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(os_three_easy_pieces
        19-paging-faster-translations/tlb.c
        thread_api.c cuncurrency/sloppyCounter.c)
