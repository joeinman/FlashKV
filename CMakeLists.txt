cmake_minimum_required(VERSION 3.27.0)

project(FlashKV VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(FlashKV STATIC)

target_sources(FlashKV PRIVATE
    "src/FlashKV.cpp"
)

target_include_directories(FlashKV PUBLIC
    "include"
)