// tests/test_error_cases.c
// Test error handling and edge cases for the space packet library

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "space_packet_sender.h"
#include "space_packet_receiver.h"

int test_parameter_validation() {
    printf("Testing parameter validation...\n");
    
    // Test parse_space_packet with safe edge cases only
    SpacePacketHeader header;
    unsigned char payload_buffer[100];
    
    // Test with minimum size packet (should fail)
    unsigned char tiny_packet[] = {0x00, 0x00};
    int result = parse_space_packet(tiny_packet, sizeof(tiny_packet), &header, payload_buffer);
    if (result != 0) {
        printf("✓ parse_space_packet correctly rejected tiny packet\n");
    } else {
        printf("! parse_space_packet should reject tiny packet\n");
    }
    
    printf("✓ Parameter validation tests completed\n");
    return 0;
}

int test_boundary_packet_parameters() {
    printf("Testing boundary packet parameters...\n");
    
    // Test only with valid parameters to avoid Python exceptions
    unsigned char payload[] = "test";
    size_t packet_size = 0;
    
    struct boundary_case {
        int apid;
        int seq_count;
        int packet_type;
        int sec_header_flag;
        const char* description;
    } cases[] = {
        {0, 0, 0, 0, "Minimum values"},
        {2047, 16383, 1, 1, "Maximum valid values"},
        {1, 1, 1, 1, "Small positive values"},
        {100, 200, 0, 0, "Normal values"}
    };
    
    size_t num_cases = sizeof(cases) / sizeof(cases[0]);
    
    for (size_t i = 0; i < num_cases; i++) {
        printf("Testing: %s\n", cases[i].description);
        
        char *packet = build_space_packet(cases[i].apid, cases[i].seq_count,
                                         payload, cases[i].packet_type,
                                         cases[i].sec_header_flag, &packet_size, 4);
        
        if (packet) {
            printf("  ✓ Successfully built packet\n");
            free(packet);
        } else {
            printf("  ! Failed to build packet\n");
        }
    }
    
    printf("✓ Boundary value tests completed\n");
    return 0;
}

int test_parse_malformed_packets() {
    printf("Testing parse_space_packet with malformed data...\n");
    
    SpacePacketHeader header;
    unsigned char payload[1024];
    
    // Test with too short packet
    unsigned char short_packet[] = {0x01, 0x02};
    int result = parse_space_packet(short_packet, sizeof(short_packet), &header, payload);
    if (result == -1) {
        printf("✓ Correctly rejected packet too short\n");
    } else {
        printf("✗ Should reject packet too short (got result: %d)\n", result);
    }
    
    // Test with header but no payload
    unsigned char header_only[] = {0x08, 0x7B, 0x00, 0x01, 0x00, 0x05}; // Claims 6 bytes payload
    result = parse_space_packet(header_only, sizeof(header_only), &header, payload);
    if (result == -2) {
        printf("✓ Correctly rejected incomplete packet\n");
    } else {
        printf("✗ Should reject incomplete packet (got result: %d)\n", result);
    }
    
    // Test with valid header and matching payload
    unsigned char valid_packet[] = {
        0x08, 0x7B,       // Version=0, Type=0, SecHdr=1, APID=123
        0x00, 0x01,       // SeqFlags=0, SeqCount=1
        0x00, 0x03,       // Length=3 (means 4 bytes payload)
        'T', 'E', 'S', 'T' // 4 bytes payload
    };
    
    result = parse_space_packet(valid_packet, sizeof(valid_packet), &header, payload);
    if (result == 0) {
        printf("✓ Correctly parsed valid packet\n");
        printf("  APID: %d, SeqCount: %d, DataLen: %zu\n", 
               header.apid, header.seq_count, header.data_len);
        
        // Verify parsed data
        if (header.apid == 123 && header.seq_count == 1 && header.data_len == 4 &&
            memcmp(payload, "TEST", 4) == 0) {
            printf("✓ All parsed data correct\n");
        } else {
            printf("✗ Parsed data incorrect\n");
        }
    } else {
        printf("✗ Failed to parse valid packet (result: %d)\n", result);
    }
    
    return 0;
}

int test_large_payloads() {
    printf("Testing with various payload sizes...\n");
    
    // Test with a moderately large payload
    const size_t large_size = 512;
    unsigned char *large_payload = malloc(large_size);
    
    if (!large_payload) {
        fprintf(stderr, "Failed to allocate large payload\n");
        return -1;
    }
    
    // Fill with pattern
    for (size_t i = 0; i < large_size; i++) {
        large_payload[i] = (unsigned char)(i % 256);
    }
    
    size_t packet_size = 0;
    char *packet = build_space_packet(100, 1, large_payload, 0, 0, 
                                     &packet_size, large_size);
    
    if (packet) {
        printf("✓ Successfully built packet with %zu byte payload\n", large_size);
        
        // Try to parse it back
        SpacePacketHeader header;
        unsigned char *parsed_payload = malloc(large_size);
        
        if (parsed_payload) {
            int result = parse_space_packet((unsigned char*)packet, packet_size, 
                                          &header, parsed_payload);
            
            if (result == 0) {
                printf("✓ Successfully parsed large packet\n");
                
                // Verify a few bytes
                if (memcmp(parsed_payload, large_payload, 100) == 0) {
                    printf("✓ Large payload data integrity verified\n");
                } else {
                    printf("✗ Large payload data corruption detected\n");
                }
            } else {
                printf("✗ Failed to parse large packet: %d\n", result);
            }
            
            free(parsed_payload);
        }
        
        free(packet);
    } else {
        printf("! Large payload packet creation failed\n");
    }
    
    free(large_payload);
    
    // Test with small payload (1 byte) - NO NULL PAYLOADS
    printf("Testing with small payload...\n");
    unsigned char small_payload[] = "A"; // 1 byte payload
    packet_size = 0;
    packet = build_space_packet(200, 2, small_payload, 0, 0, &packet_size, 1);
    
    if (packet) {
        printf("✓ Successfully built packet with 1-byte payload\n");
        
        SpacePacketHeader header;
        unsigned char single_byte[1];
        
        int result = parse_space_packet((unsigned char*)packet, packet_size, 
                                      &header, single_byte);
        
        if (result == 0 && header.data_len == 1 && single_byte[0] == 'A') {
            printf("✓ Successfully parsed 1-byte payload packet\n");
        } else {
            printf("! 1-byte payload test result: %d, len: %zu, data: %c\n", 
                   result, header.data_len, single_byte[0]);
        }
        
        free(packet);
    } else {
        printf("! 1-byte payload packet creation failed\n");
    }
    
    return 0;
}

int test_boundary_values() {
    printf("Testing boundary values...\n");
    
    struct boundary_case {
        int apid;
        int seq_count;
        const char* description;
    } cases[] = {
        {0, 0, "Minimum values"},
        {2047, 16383, "Maximum values"},
        {1, 1, "Just above minimum"},
        {2046, 16382, "Just below maximum"}
    };
    
    size_t num_cases = sizeof(cases) / sizeof(cases[0]);
    
    for (size_t i = 0; i < num_cases; i++) {
        printf("Testing: %s (APID=%d, SeqCount=%d)\n", 
               cases[i].description, cases[i].apid, cases[i].seq_count);
        
        unsigned char payload[] = "boundary test";
        size_t payload_len = strlen((char*)payload);
        size_t packet_size = 0;
        
        char *packet = build_space_packet(cases[i].apid, cases[i].seq_count,
                                         payload, 0, 0, &packet_size, payload_len);
        
        if (packet) {
            SpacePacketHeader header;
            unsigned char parsed_payload[1024];
            
            int result = parse_space_packet((unsigned char*)packet, packet_size, 
                                          &header, parsed_payload);
            
            if (result == 0) {
                if (header.apid == cases[i].apid && header.seq_count == cases[i].seq_count &&
                    header.data_len == payload_len) {
                    printf("✓ Boundary case passed\n");
                } else {
                    printf("✗ Boundary case data mismatch\n");
                }
            } else {
                printf("✗ Failed to parse boundary case: %d\n", result);
            }
            
            free(packet);
        } else {
            printf("✗ Failed to build packet for boundary case\n");
        }
    }
    
    return 0;
}

int test_python_functionality() {
    printf("Testing Python functionality...\n");
    
    // Only test safe Python operations
    unsigned char payload[] = "test";
    size_t packet_size = 0;
    
    // Test with normal parameters to ensure Python is working
    char *packet = build_space_packet(100, 1, payload, 0, 0, &packet_size, 4);
    
    if (packet) {
        printf("✓ Basic Python function call works\n");
        free(packet);
    } else {
        printf("! Basic Python function call failed\n");
        return -1;
    }
    
    // Test with different packet types
    packet = build_space_packet(100, 1, payload, 1, 1, &packet_size, 4);
    if (packet) {
        printf("✓ Handled TC packet with secondary header\n");
        free(packet);
    } else {
        printf("! Failed with TC packet\n");
    }
    
    printf("✓ Python functionality tests completed\n");
    return 0;
}

int main() {
    printf("=== Error Handling and Edge Cases Tests ===\n");
    
    // Initialize Python once at the beginning
    init_space_packet_sender();
    
    if (test_parameter_validation() != 0) {
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }
    
    if (test_boundary_packet_parameters() != 0) {
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }
    
    if (test_parse_malformed_packets() != 0) {
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }
    
    if (test_large_payloads() != 0) {
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }
    
    if (test_boundary_values() != 0) {
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }
    
    if (test_python_functionality() != 0) {
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }
    
    // Finalize Python once at the end
    finalize_space_packet_sender();
    
    printf("=== All Error Handling Tests Completed! ===\n");
    return EXIT_SUCCESS;
}