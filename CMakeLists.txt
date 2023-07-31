cmake_minimum_required(VERSION 3.27.0)

project(FlashKV)

add_library(FlashKV STATIC)

target_sources(FlashKV PRIVATE
    "src/FlashKV.cpp"
)

target_include_directories(FlashKV PUBLIC
    "include"
)