// tests/test_basic_api.c
// Test for build_space_packet and parse_space_packet functions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "space_packet_sender.h"
#include "space_packet_receiver.h"

#define TEST_PAYLOAD "Hello, World!"
#define TEST_APID 123
#define TEST_SEQ_COUNT 42
#define TEST_PACKET_TYPE 0  // TM
#define TEST_SEC_HEADER_FLAG 0

int test_build_and_parse() {
    printf("Testing build_space_packet and parse_space_packet...\n");
    
    // Test data
    const unsigned char payload[] = TEST_PAYLOAD;
    size_t payload_len = strlen(TEST_PAYLOAD);
    size_t packet_size = 0;
    
    // Build space packet
    char *packet = build_space_packet(TEST_APID, TEST_SEQ_COUNT, payload,
                                      TEST_PACKET_TYPE, TEST_SEC_HEADER_FLAG, 
                                      &packet_size, payload_len);
    
    if (!packet) {
        fprintf(stderr, "Failed to build space packet\n");
        return -1;
    }
    
    printf("Built packet of size: %zu\n", packet_size);
    
    // Parse the packet back
    SpacePacketHeader header;
    unsigned char parsed_payload[1024] = {0};
    
    int result = parse_space_packet((unsigned char*)packet, packet_size, &header, parsed_payload);
    
    if (result != 0) {
        fprintf(stderr, "Failed to parse space packet, error code: %d\n", result);
        free(packet);
        return -1;
    }
    
    // Verify the header
    printf("Parsed header - APID: %d, SeqCount: %d, DataLen: %zu\n", 
           header.apid, header.seq_count, header.data_len);
    
    assert(header.apid == TEST_APID);
    assert(header.seq_count == TEST_SEQ_COUNT);
    assert(header.packet_type == TEST_PACKET_TYPE);
    assert(header.sec_header_flag == TEST_SEC_HEADER_FLAG);
    assert(header.data_len == payload_len);
    
    // Verify the payload
    assert(memcmp(parsed_payload, payload, payload_len) == 0);
    
    printf("✓ Basic API test passed\n");
    
    // Free the allocated packet
    free(packet);
    return 0;
}

int test_multiple_packets() {
    printf("Testing multiple packet types...\n");
    
    // Test different packet types and parameters
    struct test_case {
        int apid;
        int seq_count;
        int packet_type;
        int sec_header_flag;
        const char* payload;
    } test_cases[] = {
        {100, 1, 0, 0, "TM packet"},
        {200, 2, 1, 1, "TC packet with sec header"},
        {300, 3, 0, 1, "Another test"},
        {2047, 16383, 1, 0, "Max values"}  // Max APID and SeqCount
    };
    
    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < num_tests; i++) {
        size_t payload_len = strlen(test_cases[i].payload);
        size_t packet_size = 0;
        
        char *packet = build_space_packet(test_cases[i].apid, test_cases[i].seq_count,
                                          (unsigned char*)test_cases[i].payload,
                                          test_cases[i].packet_type, test_cases[i].sec_header_flag,
                                          &packet_size, payload_len);
        
        if (!packet) {
            fprintf(stderr, "Failed to build packet for test case %zu\n", i);
            return -1;
        }
        
        SpacePacketHeader header;
        unsigned char parsed_payload[1024] = {0};
        
        int result = parse_space_packet((unsigned char*)packet, packet_size, &header, parsed_payload);
        
        if (result != 0) {
            fprintf(stderr, "Failed to parse packet for test case %zu\n", i);
            free(packet);
            return -1;
        }
        
        // Verify all fields
        assert(header.apid == test_cases[i].apid);
        assert(header.seq_count == test_cases[i].seq_count);
        assert(header.packet_type == test_cases[i].packet_type);
        assert(header.sec_header_flag == test_cases[i].sec_header_flag);
        assert(header.data_len == payload_len);
        assert(memcmp(parsed_payload, test_cases[i].payload, payload_len) == 0);
        
        printf("✓ Test case %zu passed (APID=%d, SeqCount=%d)\n", 
               i+1, test_cases[i].apid, test_cases[i].seq_count);
        
        free(packet);
    }
    
    return 0;
}

int main() {
    printf("=== Basic API Tests ===\n");
    
    // Initialize Python once at the beginning
    init_space_packet_sender();
    
    if (test_build_and_parse() != 0) {
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }
    
    if (test_multiple_packets() != 0) {
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }
    
    // Finalize Python once at the end
    finalize_space_packet_sender();
    
    printf("=== All Basic API Tests Passed! ===\n");
    return EXIT_SUCCESS;
}