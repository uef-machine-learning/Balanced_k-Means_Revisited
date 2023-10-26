

cmake_minimum_required(VERSION 3.10)
project(bkmeans)    

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the source files and include directories
set(SOURCES
    sources.cpp
    contrib/argtable/argtable3.c
)

# set(INCLUDE_DIRS
#     include
# )

# Create the executable
add_executable(bkmeans ${SOURCES})

# Specify the include directories
target_include_directories(bkmeans PRIVATE ${INCLUDE_DIRS})
