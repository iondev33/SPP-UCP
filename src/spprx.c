#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "space_packet_receiver.h"

#define MAX_PACKET_SIZE 65535

void print_payload(const unsigned char *payload, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", payload[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PORT>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    unsigned char buffer[MAX_PACKET_SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Listening on port %d...\n", port);

    while (1) {
        ssize_t packet_size = recvfrom(sock, buffer, MAX_PACKET_SIZE, 0,
                                       (struct sockaddr *)&client_addr, &client_addr_len);
        if (packet_size < 0) {
            perror("recvfrom failed");
            continue;
        }

        SpacePacketHeader header;
        // Allocate payload buffer on the heap to be correctly handled by the parser
        unsigned char *payload = malloc(MAX_PACKET_SIZE);
        if (payload == NULL) {
            perror("Failed to allocate payload buffer");
            continue;
        }

        int result = parse_space_packet(buffer, packet_size, &header, payload);

        if (result == 0) {
            // Updated printf statement to show flags and count separately
            printf("Received Packet: APID=%d, SeqFlags=%d, SeqCount=%d, Len=%zu, Payload: ",
                   header.apid, header.seq_flags, header.seq_count, header.data_len);
            print_payload(payload, header.data_len);
        } else if (result == -1) {
            fprintf(stderr, "Error: Packet too short\n");
        } else if (result == -2) {
            fprintf(stderr, "Error: Incomplete packet based on header length\n");
        } else {
            fprintf(stderr, "Error: Unknown error parsing packet\n");
        }

        free(payload); // Free the allocated memory
    }

    close(sock);
    return EXIT_SUCCESS;
}

