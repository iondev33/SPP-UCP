#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "space_packet_receiver.h"

#define MAX_PACKET_SIZE 65535

size_t packet_indication(char *buffer, int apid) {

    char packet[MAX_PACKET_SIZE];
    const char *ip = "192.168.1.202";
    int port = 55554;
    int payload_length = -1;
    int enable = 1;

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

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock);
        return EXIT_FAILURE;
    }

    ssize_t packet_size = recv(sock, packet, MAX_PACKET_SIZE, 0);
    if (packet_size < 0) {
	//perror("Receive failed");
	printf("Receive failed\n");
    }

    SpacePacketHeader header;
    char *payload = NULL;
    if (parse_space_packet(packet, packet_size, &header, &payload) == 0) {
	apid = header.apid;
	memcpy(buffer, payload, header.data_len);
	/* Leave this for a later conditional debug flag
	printf("Parsed Space Packet:\n");
	printf("  APID: %d\n", apid);
	printf("  Sequence Count: %d\n", header.seq_count);
	printf("  Packet Type: %s\n", header.packet_type ? "TC" : "TM");
	printf("  Secondary Header Flag: %d\n", header.sec_header_flag);
	printf("  Data Length: %zu\n", header.data_len);
	printf("  Payload (hex): ");
	*/
	for (size_t i = 0; i < header.data_len; i++) {
	    printf("%02x", (unsigned char)payload[i]);
	}
	printf("\n");
	
	free(payload);
    } else {
	fprintf(stderr, "Failed to parse space packet\n");
    }

    close(sock);

    if (packet_size < 6)
	payload_length = -1;
    else
	payload_length = header.data_len;
    
    return payload_length;
}
