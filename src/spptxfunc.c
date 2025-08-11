#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "space_packet_sender.h"

#define MAX_PAYLOAD_SIZE 1024

static int seq_count = 0;

int packet_request(unsigned char *hex_payload, int apid, int seq_count, int packet_type, int sec_header_flag, size_t to_send_bytes)
{
    int port = 55554;
    char* packet = NULL;
    size_t packet_size = 0;
    size_t bytes_written = 0;

    const char ip[] = "192.168.1.203";    

    // Initialize Python interpreter
    init_space_packet_sender();

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
        return -1;
    }

    packet = build_space_packet(apid, seq_count, (const char*)hex_payload,
    				packet_type, sec_header_flag, &packet_size, to_send_bytes);


    if (!packet) 
    {
	fprintf(stderr, "Failed to build space packet\n");
	return -1;
    }

    if ((bytes_written = sendto(sock, packet, to_send_bytes, 0,
	       (struct sockaddr *) &server_addr,
			       sizeof(server_addr))) < 0) 
    {
	perror("Failed to send packet");
    }

    free(packet);
    seq_count++;

    close(sock);
    // Finalize Python interpreter
    finalize_space_packet_sender();

    return bytes_written;
}
