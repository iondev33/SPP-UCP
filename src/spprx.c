#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "space_packet_receiver.h"

#define MAX_PACKET_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP> <PORT>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(sock);
        return EXIT_FAILURE;
    }

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Listening on %s:%d...\n", ip, port);

    char packet[MAX_PACKET_SIZE];
    while (1) {
        ssize_t packet_size = recv(sock, packet, MAX_PACKET_SIZE, 0);
        if (packet_size < 0) {
            perror("Receive failed");
            break;
        }

        SpacePacketHeader header;
        char *payload = NULL;
        if (parse_space_packet(packet, packet_size, &header, &payload) == 0) {
            printf("Parsed Space Packet:\n");
            printf("  APID: %d\n", header.apid);
            printf("  Sequence Count: %d\n", header.seq_count);
            printf("  Packet Type: %s\n", header.packet_type ? "TC" : "TM");
            printf("  Secondary Header Flag: %d\n", header.sec_header_flag);
            printf("  Data Length: %zu\n", header.data_len);
            printf("  Payload (hex): ");
            for (size_t i = 0; i < header.data_len; i++) {
                printf("%02x", (unsigned char)payload[i]);
            }
            printf("\n");

            free(payload);
        } else {
            fprintf(stderr, "Failed to parse space packet\n");
        }
    }

    close(sock);
    return EXIT_SUCCESS;
}
