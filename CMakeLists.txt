cmake_minimum_required(VERSION 3.15)
project(spp-ucp VERSION 1.0)

# Define CMake options
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Define virtual environment paths
if (WIN32)
    set(VENV_DIR "${CMAKE_SOURCE_DIR}/../SPPUCPENV")
    set(PIP_EXEC "${VENV_DIR}/Scripts/pip.exe")
    set(PYTHON_EXEC "${VENV_DIR}/Scripts/python.exe")
else()
    set(VENV_DIR "${CMAKE_SOURCE_DIR}/../SPPUCPENV")
    set(PIP_EXEC "${VENV_DIR}/bin/pip")
    set(PYTHON_EXEC "${VENV_DIR}/bin/python")
endif()

# Use Python executable from the virtual environment
set(Python3_EXECUTABLE ${PYTHON_EXEC})

# Find Python libraries and headers from the virtual environment
find_package(Python3 REQUIRED COMPONENTS Development)

# Add Python include directories
include_directories(${Python3_INCLUDE_DIRS})

# Add subdirectory for the C code
add_subdirectory(src)

# Build the sender executable and link it to the library and Python
add_executable(spptx src/spptx.c)
target_link_libraries(spptx PRIVATE space_packet_sender Python3::Python)

# Build the receiver application
add_executable(spprx src/spprx.c)
target_link_libraries(spprx PRIVATE space_packet_receiver Python3::Python)

# Build the sender pipe executable
add_executable(spptxpipe src/spptxpipe.c)
target_link_libraries(spptxpipe PRIVATE space_packet_sender Python3::Python)

#find_package(Python3 COMPONENTS Interpretere Development REQUIRED)
#find_package(PythonLibs REQUIRED)
#include_directories(${PYTHON_INCLUDE_DIRS})
# Build shared library
add_library(spp_protocol SHARED
    src/space_packet_sender.c
    src/space_packet_receiver.c
    src/spptxfunc.c
    src/spprxfunc.c
)

target_include_directories(spp_protocol PRIVATE Python3::Python)
target_link_libraries(spp_protocol PRIVATE Python3::Python)

find_package(Python COMPONENTS Interpreter Development REQUIRED)
target_include_directories(spp_protocol PRIVATE ${PYTHON_INCLUDE_DIRS})
target_link_libraries(spp_protocol PRIVATE ${PYTHON_LIBRARIES})


# Optional: Enable testing
enable_testing()
add_test(
    NAME SendRecvSpacePacketTest-SPP-Module
    COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${CMAKE_SOURCE_DIR}/python/tests -p "test_send_recv.py"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python
)

# CMake Tests for Space Packet Protocol Library
# Add this to your main CMakeLists.txt after the existing test

# Create a tests directory structure
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# Test 1: Basic API Test - build_space_packet and parse_space_packet
add_executable(test_basic_api tests/test_basic_api.c)
target_link_libraries(test_basic_api PRIVATE space_packet_sender space_packet_receiver Python3::Python)
target_include_directories(test_basic_api PRIVATE src)

# Test 2: Shared Library API Test - packet_request and packet_indication
add_executable(test_shared_api tests/test_shared_api.c)
target_link_libraries(test_shared_api PRIVATE spp_protocol Python3::Python)
target_include_directories(test_shared_api PRIVATE src)

# Test 3: Error handling and edge cases
add_executable(test_error_cases tests/test_error_cases.c)
target_link_libraries(test_error_cases PRIVATE space_packet_sender space_packet_receiver Python3::Python)
target_include_directories(test_error_cases PRIVATE src)

# Register the tests with CTest
add_test(
    NAME BasicAPITest
    COMMAND test_basic_api
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(
    NAME SharedAPITest
    COMMAND test_shared_api
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(
    NAME ErrorCasesTest
    COMMAND test_error_cases
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Set test properties
set_tests_properties(BasicAPITest PROPERTIES
    TIMEOUT 30
    LABELS "unit;basic"
)

set_tests_properties(SharedAPITest PROPERTIES
    TIMEOUT 30
    LABELS "unit;shared"
)

set_tests_properties(ErrorCasesTest PROPERTIES
    TIMEOUT 30
    LABELS "unit;error"
)

# Create a custom target to run all tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS test_basic_api test_shared_api test_error_cases
    COMMENT "Running all Space Packet Protocol tests"
)