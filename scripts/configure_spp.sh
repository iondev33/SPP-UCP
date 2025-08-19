#!/bin/bash

# SPP-UCP Configuration Utility
# This script helps users configure and build SPP-UCP with different network settings

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}=== SPP-UCP Configuration Utility ===${NC}"
    echo ""
}

print_usage() {
    echo "Usage: $0 [preset|custom|clean|help]"
    echo ""
    echo "Commands:"
    echo "  preset    - Use predefined configuration presets"
    echo "  custom    - Configure custom IP addresses and ports"
    echo "  clean     - Clean build directory"
    echo "  help      - Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 preset                    # Use interactive preset selection"
    echo "  $0 custom                    # Configure custom addresses"
    echo "  $0 clean                     # Clean build directory"
}

configure_preset() {
    echo -e "${YELLOW}Available Configuration Presets:${NC}"
    echo ""
    echo "1) localhost        - Single machine testing (127.0.0.1:55554)"
    echo "2) localhost_split  - Single machine, separate ports (TX:55554, RX:55555)"
    echo "3) lab_network      - Lab network (TX:192.168.1.203:55554, RX:192.168.1.202:55554)"
    echo "4) custom           - Configure custom addresses"
    echo "5) exit             - Exit without configuring"
    echo ""
    
    read -p "Select preset (1-5): " choice
    
    case $choice in
        1)
            TX_IP="127.0.0.1"
            TX_PORT="55554"
            RX_IP="127.0.0.1"
            RX_PORT="55554"
            CONFIG_NAME="localhost"
            ;;
        2)
            TX_IP="127.0.0.1"
            TX_PORT="55554"
            RX_IP="127.0.0.1"
            RX_PORT="55555"
            CONFIG_NAME="localhost_split"
            ;;
        3)
            TX_IP="192.168.1.203"
            TX_PORT="55554"
            RX_IP="192.168.1.202"
            RX_PORT="55554"
            CONFIG_NAME="lab_network"
            ;;
        4)
            configure_custom
            return
            ;;
        5)
            echo "Exiting..."
            exit 0
            ;;
        *)
            echo -e "${RED}Invalid selection${NC}"
            exit 1
            ;;
    esac
    
    build_with_config "$CONFIG_NAME"
}

configure_custom() {
    echo -e "${YELLOW}Custom Configuration:${NC}"
    echo ""
    
    read -p "Enter TX IP address [127.0.0.1]: " TX_IP
    TX_IP=${TX_IP:-127.0.0.1}
    
    read -p "Enter TX port [55554]: " TX_PORT
    TX_PORT=${TX_PORT:-55554}
    
    read -p "Enter RX IP address [127.0.0.1]: " RX_IP
    RX_IP=${RX_IP:-127.0.0.1}
    
    read -p "Enter RX port [55554]: " RX_PORT
    RX_PORT=${RX_PORT:-55554}
    
    CONFIG_NAME="custom"
    build_with_config "$CONFIG_NAME"
}

validate_ip() {
    local ip=$1
    if [[ $ip =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$ ]]; then
        return 0
    else
        return 1
    fi
}

validate_port() {
    local port=$1
    if [[ $port =~ ^[0-9]+$ ]] && [ $port -ge 1024 ] && [ $port -le 65535 ]; then
        return 0
    else
        return 1
    fi
}

build_with_config() {
    local config_name=$1
    
    echo ""
    echo -e "${YELLOW}Validating configuration...${NC}"
    
    # Validate inputs
    if ! validate_ip "$TX_IP"; then
        echo -e "${RED}Error: Invalid TX IP address: $TX_IP${NC}"
        exit 1
    fi
    
    if ! validate_ip "$RX_IP"; then
        echo -e "${RED}Error: Invalid RX IP address: $RX_IP${NC}"
        exit 1
    fi
    
    if ! validate_port "$TX_PORT"; then
        echo -e "${RED}Error: Invalid TX port: $TX_PORT (must be 1024-65535)${NC}"
        exit 1
    fi
    
    if ! validate_port "$RX_PORT"; then
        echo -e "${RED}Error: Invalid RX port: $RX_PORT (must be 1024-65535)${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}Configuration valid!${NC}"
    echo ""
    echo -e "${BLUE}Configuration Summary:${NC}"
    echo "  Name: $config_name"
    echo "  Transmission: $TX_IP:$TX_PORT"
    echo "  Reception:    $RX_IP:$RX_PORT"
    echo ""
    
    read -p "Proceed with build? (y/N): " confirm
    if [[ ! $confirm =~ ^[Yy]$ ]]; then
        echo "Build cancelled."
        exit 0
    fi
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    echo ""
    echo -e "${YELLOW}Configuring CMake...${NC}"
    
    # Run CMake with configuration
    cmake \
        -DSPP_TX_IP_ADDRESS="$TX_IP" \
        -DSPP_TX_PORT="$TX_PORT" \
        -DSPP_RX_IP_ADDRESS="$RX_IP" \
        -DSPP_RX_PORT="$RX_PORT" \
        "$PROJECT_ROOT"
    
    echo ""
    echo -e "${YELLOW}Building project...${NC}"
    make -j$(nproc 2>/dev/null || echo 4)
    
    echo ""
    echo -e "${GREEN}Build completed successfully!${NC}"
    echo ""
    echo -e "${BLUE}Built with configuration:${NC}"
    echo "  TX: $TX_IP:$TX_PORT"
    echo "  RX: $RX_IP:$RX_PORT"
    echo ""
    echo -e "${BLUE}Executables available in: $BUILD_DIR${NC}"
    echo "  spptx, spprx, spptxpipe"
    echo "  libspp_protocol.so"
    echo ""
    echo -e "${YELLOW}To run tests:${NC}"
    echo "  cd $BUILD_DIR && ctest --output-on-failure"
}

clean_build() {
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
    echo -e "${GREEN}Build directory cleaned.${NC}"
}

# Main script logic
print_header

case "${1:-preset}" in
    preset)
        configure_preset
        ;;
    custom)
        configure_custom
        ;;
    clean)
        clean_build
        ;;
    help|--help|-h)
        print_usage
        ;;
    *)
        echo -e "${RED}Unknown command: $1${NC}"
        echo ""
        print_usage
        exit 1
        ;;
esac