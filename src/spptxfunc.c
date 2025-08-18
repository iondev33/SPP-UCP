#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "space_packet_sender.h"
#include "spp_config.h"  // Include generated configuration

#define MAX_PAYLOAD_SIZE 1024

int packet_request(unsigned char *byte_payload, int apid, int seq_count, int packet_type, int sec_header_flag, size_t to_send_bytes)
{
    char* packet = NULL;
    size_t packet_size = 0;
    size_t bytes_written = 0;

    // Use compile-time configured values instead of hardcoded ones
    const char ip[] = SPP_TX_IP_ADDRESS;
    const int port = SPP_TX_PORT;

    // NOTE: the calling application should initialize Python interpreter: init_space_packet_sender();

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
    {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = { 0 };
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) 
    {
        perror("Invalid IP address");
        close(sock);
        return -1;
    }

    packet = build_space_packet(apid, seq_count, (const char*)byte_payload,
                                packet_type, sec_header_flag, &packet_size, to_send_bytes);

    if (!packet) 
    {
        fprintf(stderr, "Failed to build space packet\n");
        close(sock);
        return -1;
    }

    if ((bytes_written = sendto(sock, packet, packet_size, 0,
                               (struct sockaddr *) &server_addr,
                               sizeof(server_addr))) < 0) 
    {
        perror("Failed to send packet");
    }
    else
    {
        // Optional: Add debug information about configuration
        #ifdef DEBUG_SPP_CONFIG
        printf("DEBUG: Sent packet to %s:%d (configured at compile time)\n", ip, port);
        #endif
    }

    free(packet);
    close(sock);

    return bytes_written;
}