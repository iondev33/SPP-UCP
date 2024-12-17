cmake_minimum_required(VERSION 3.15)
project(spp-ucp VERSION 1.0)

# Define CMake options
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Define virtual environment paths
if (WIN32)
    set(VENV_DIR "${CMAKE_SOURCE_DIR}/spp-ucp-env")
    set(PIP_EXEC "${VENV_DIR}/Scripts/pip.exe")
    set(PYTHON_EXEC "${VENV_DIR}/Scripts/python.exe")
else()
    set(VENV_DIR "${CMAKE_SOURCE_DIR}/spp-ucp-env")
    set(PIP_EXEC "${VENV_DIR}/bin/pip")
    set(PYTHON_EXEC "${VENV_DIR}/bin/python")
endif()

# Ensure virtual environment is created and dependencies are installed
add_custom_target(ActivateVirtualEnv
    COMMAND ${CMAKE_COMMAND} -E echo "Setting up virtual environment..."
    COMMAND ${CMAKE_COMMAND} -E sh -c
        "if [ ! -d \"${VENV_DIR}\" ]; then \
            echo 'Creating virtual environment...'; \
            python3 -m venv \"${VENV_DIR}\"; \
        else \
            echo 'Virtual environment already exists.'; \
        fi"
    COMMAND ${PIP_EXEC} install --upgrade pip
    COMMAND ${PIP_EXEC} install --quiet --requirement ${CMAKE_SOURCE_DIR}/python/requirements.txt
    COMMENT "Virtual environment setup and dependency installation"
)

# Use Python executable from the virtual environment
set(Python3_EXECUTABLE ${PYTHON_EXEC})

# Find Python libraries and headers from the virtual environment
find_package(Python3 REQUIRED COMPONENTS Development)

# Add Python include directories
include_directories(${Python3_INCLUDE_DIRS})

# Add subdirectory for the C code
add_subdirectory(src)

# Build the executable and link it to the library and Python
add_executable(spptx src/spptx.c)
target_link_libraries(spptx PRIVATE space_packet_sender Python3::Python)

# Optional: Enable testing
enable_testing()
add_test(
    NAME SendRecvSpacePacketTest
    COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${CMAKE_SOURCE_DIR}/python/tests -p "test_send_recv.py"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python
)



