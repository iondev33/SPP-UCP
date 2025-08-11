#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "space_packet_sender.h"

#define MAX_PAYLOAD_SIZE 1024

int main(int argc, char *argv[]) 
{
    if (argc != 6) 
	{
        fprintf(stderr, "Usage: %s <IP> <PORT> <APID> <PACKET_TYPE> <SEC_HEADER_FLAG>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    int apid = atoi(argv[3]);
    int packet_type = atoi(argv[4]);
    int sec_header_flag = atoi(argv[5]);
    int seq_count = 0;

    // Initialize Python interpreter
    init_space_packet_sender();

    char hex_payload[MAX_PAYLOAD_SIZE];
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
	{
        perror("Socket creation failed");
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = { 0 };
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) 
	{
        perror("Invalid IP address");
        close(sock);
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }

    while (1) 
	{
        printf("Enter payload in HEX (or 'exit' to quit): ");
        if (!fgets(hex_payload, sizeof(hex_payload), stdin))
            break;
        hex_payload[strcspn(hex_payload, "\n")] = 0;

        if (strcmp(hex_payload, "exit") == 0)
            break;

        if (!is_valid_hex(hex_payload))
            continue;

        size_t packet_size;
        char *packet = build_space_packet(apid, seq_count, hex_payload,
                                        packet_type, sec_header_flag, &packet_size);
        if (!packet) 
		{
            fprintf(stderr, "Failed to build space packet\n");
            continue;
        }

        if (sendto(sock, packet, packet_size, 0,
                   (struct sockaddr *) &server_addr,
                   sizeof(server_addr)) < 0) 
		{
            perror("Failed to send packet");
        }
		else 
		{
            printf("Packet sent: %s\n", hex_payload);
        }

        free(packet);
        seq_count++;
    }

    close(sock);
    // Finalize Python interpreter
    finalize_space_packet_sender();
    return EXIT_SUCCESS;
}
