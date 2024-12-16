cmake_minimum_required(VERSION 3.15)
project(spp-ucp VERSION 1.0)

# Define CMake options
set(CMAKE_C_STANDARD 99)  # Use C99
set(CMAKE_C_STANDARD_REQUIRED True)

# Add subdirectory for the C code
add_subdirectory(src)

# Message for Python module (informational only, not built via CMake)
message(STATUS "Required python module is located in the python/ directory.")

# src/CMakeLists.txt
add_executable(spp-ucp space_packet_sender.c)

# Add include directories (if needed)
target_include_directories(spp-ucp PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Example: Link against any required libraries
# target_link_libraries(MyProject PRIVATE some_library)

# Locate Python interpreter and libraries
find_package(Python3.9 REQUIRED COMPONENTS Interpreter)

# Example: Ensure virtual environment is active
add_custom_target(
    ActivateVirtualEnv ALL
    COMMAND ${Python3_EXECUTABLE} -m venv ${CMAKE_SOURCE_DIR}/venv
    COMMAND ${CMAKE_SOURCE_DIR}/venv/bin/pip install -r ${CMAKE_SOURCE_DIR}/python/requirements.txt
    COMMENT "Setting up Python virtual environment"
)

# Enable testing
enable_testing()

# Add a test to invoke Python
add_test(
    NAME PythonTests
    COMMAND ${CMAKE_SOURCE_DIR}/venv/bin/python -m unittest discover -s python/tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python
)
