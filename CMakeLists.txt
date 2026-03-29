cmake_minimum_required(VERSION 3.16)
project(Proxima-Centauri VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories for both projects
add_subdirectory(proxima)
add_subdirectory(centauri)
