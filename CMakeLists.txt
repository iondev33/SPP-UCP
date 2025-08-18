cmake_minimum_required(VERSION 3.15)
project(spp-ucp VERSION 1.0)

# =============================================================================
# SPP-UCP Network Configuration Options
# =============================================================================

# Define configuration options with default values
set(SPP_TX_IP_ADDRESS "127.0.0.1" CACHE STRING "IP address for packet transmission (packet_request)")
set(SPP_TX_PORT "55554" CACHE STRING "Port for packet transmission (packet_request)")
set(SPP_RX_IP_ADDRESS "127.0.0.1" CACHE STRING "IP address for packet reception (packet_indication)")
set(SPP_RX_PORT "55554" CACHE STRING "Port for packet reception (packet_indication)")

# Configuration presets for common scenarios
option(SPP_CONFIG_LOCALHOST "Use localhost configuration (127.0.0.1)" ON)
option(SPP_CONFIG_SEPARATE_PORTS "Use separate ports for TX/RX on same host" OFF)

# Apply configuration presets
if(SPP_CONFIG_LOCALHOST)
    set(SPP_TX_IP_ADDRESS "127.0.0.1")
    set(SPP_RX_IP_ADDRESS "127.0.0.1")
    if(SPP_CONFIG_SEPARATE_PORTS)
        set(SPP_TX_PORT "55554")
        set(SPP_RX_PORT "55555")
    else()
        set(SPP_TX_PORT "55554")
        set(SPP_RX_PORT "55554")
    endif()
endif()

# Validation functions
function(validate_ip_address ip_var ip_value)
    # Basic IPv4 format validation
    string(REGEX MATCH "^([0-9]{1,3}\\.){3}[0-9]{1,3}$" valid_ip ${ip_value})
    if(NOT valid_ip)
        message(FATAL_ERROR "${ip_var} '${ip_value}' is not a valid IPv4 address")
    endif()
endfunction()

function(validate_port port_var port_value)
    # Validate port range
    if(port_value LESS 1024 OR port_value GREATER 65535)
        message(FATAL_ERROR "${port_var} '${port_value}' must be between 1024 and 65535")
    endif()
endfunction()

# Validate configuration
validate_ip_address("SPP_TX_IP_ADDRESS" ${SPP_TX_IP_ADDRESS})
validate_ip_address("SPP_RX_IP_ADDRESS" ${SPP_RX_IP_ADDRESS})
validate_port("SPP_TX_PORT" ${SPP_TX_PORT})
validate_port("SPP_RX_PORT" ${SPP_RX_PORT})

# Generate build metadata
string(TIMESTAMP SPP_BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC" UTC)
set(SPP_CONFIG_VERSION "1.0")

# Generate configuration header
configure_file(
    "${CMAKE_SOURCE_DIR}/src/spp_config.h.in"
    "${CMAKE_BINARY_DIR}/include/spp_config.h"
    @ONLY
)

# Add include directory for generated headers
include_directories("${CMAKE_BINARY_DIR}/include")

# Display configuration summary
message(STATUS "=== SPP-UCP Network Configuration ===")
message(STATUS "Transmission: ${SPP_TX_IP_ADDRESS}:${SPP_TX_PORT}")
message(STATUS "Reception:    ${SPP_RX_IP_ADDRESS}:${SPP_RX_PORT}")
message(STATUS "Build Time:   ${SPP_BUILD_TIMESTAMP}")
message(STATUS "=====================================")

# Create configuration presets for common scenarios
function(create_config_preset preset_name tx_ip tx_port rx_ip rx_port description)
    message(STATUS "Available preset: ${preset_name}")
    message(STATUS "  Description: ${description}")
    message(STATUS "  Usage: cmake -DSPP_CONFIG_PRESET=${preset_name} ..")
    message(STATUS "  TX: ${tx_ip}:${tx_port}, RX: ${rx_ip}:${rx_port}")
endfunction()

# Show available presets
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR VERBOSE_CONFIG)
    message(STATUS "")
    message(STATUS "=== Available Configuration Presets ===")
    create_config_preset("localhost" "127.0.0.1" "55554" "127.0.0.1" "55554" "Single machine testing")
    create_config_preset("localhost_split" "127.0.0.1" "55554" "127.0.0.1" "55555" "Single machine with separate ports")
    create_config_preset("lab_network" "192.168.1.203" "55554" "192.168.1.202" "55554" "Lab network configuration")
    message(STATUS "========================================")
endif()

# Define CMake options
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Define virtual environment paths
# The user is responsible for creating this directory
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