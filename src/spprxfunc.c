#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "space_packet_receiver.h"

#define MAX_PACKET_SIZE 65535

/**
 * @brief Receives a single UDP packet and parses it as a CCSDS Space Packet.
 * * @param buffer A buffer provided by the caller to store the packet's payload.
 * @param apid A pointer to an integer that will be populated with the packet's APID.
 * @return The length of the received payload on success, or -1 on failure.
 */
size_t packet_indication(char *buffer, int *apid) {

    unsigned char packet[MAX_PACKET_SIZE];
    const char *ip = "192.168.1.202"; // This should likely be configurable
    int port = 55554;
    int payload_length = -1;
    int enable = 1;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(sock);
        return -1;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock);
        return -1;
    }

    ssize_t packet_size = recv(sock, packet, MAX_PACKET_SIZE, 0);
    if (packet_size < 0) {
        perror("Receive failed");
        close(sock);
        return -1;
    }

    SpacePacketHeader header;
    // The parse function will now copy the payload directly into the user-provided 'buffer'.
    if (parse_space_packet(packet, packet_size, &header, (unsigned char*)buffer) == 0) {
        // Correctly update the APID using the pointer
        *apid = header.apid;
        payload_length = header.data_len;

        /* // Conditional debug output
        printf("Parsed Space Packet:\n");
        printf("  APID: %d\n", *apid);
        printf("  Seq Flags: %d\n", header.seq_flags);
        printf("  Sequence Count: %d\n", header.seq_count);
        printf("  Packet Type: %s\n", header.packet_type ? "TC" : "TM");
        printf("  Secondary Header Flag: %d\n", header.sec_header_flag);
        printf("  Data Length: %zu\n", header.data_len);
        printf("  Payload (hex): ");
        for (size_t i = 0; i < header.data_len; i++) {
            printf("%02x", (unsigned char)buffer[i]);
        }
        printf("\n");
        */
        
    } else {
        fprintf(stderr, "Failed to parse space packet\n");
        payload_length = -1;
    }

    close(sock);
    
    return payload_length;
}
