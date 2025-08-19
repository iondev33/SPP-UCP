#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "space_packet_sender.h"

// Verify HEX input
int is_valid_hex(const char *hex_payload)
{
    size_t len = strlen(hex_payload);
    if (len % 2 != 0)
	{
        fprintf(stderr, "Payload length must be even\n");
        return 0;
    }
    for (size_t i = 0; i < len; i++)
	{
        if (!isxdigit(hex_payload[i]))
		{
            fprintf(stderr, "Invalid character in payload: %c\n",
				hex_payload[i]);
            return 0;
        }
    }
    return 1;
}

// Function to convert a single hex character to its integer value
int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Function to convert a hex string to a byte array
int hex_string_to_bytes(const char *hex_string, unsigned char *byte_array, size_t max_len) {
    size_t hex_len = strlen(hex_string);
    // Allow empty input to pass through, will result in a zero-filled payload
    if (hex_len == 0) {
        return 0;
    }
    if (hex_len % 2 != 0) {
        fprintf(stderr, "Hex string must have an even number of characters.\n");
        return -1;
    }
    size_t byte_len = hex_len / 2;
    if (byte_len > max_len) {
        byte_len = max_len; // Truncate if the input is too long
    }

    for (size_t i = 0; i < byte_len; i++) {
        int high = hex_char_to_int(hex_string[2 * i]);
        int low = hex_char_to_int(hex_string[2 * i + 1]);
        if (high == -1 || low == -1) {
            fprintf(stderr, "Invalid hex character found in input.\n");
            return -1;
        }
        byte_array[i] = (high << 4) | low;
    }
    return byte_len;
}

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        fprintf(stderr, "Usage: %s <IP> <PORT> <APID> <PACKET_TYPE> <SEC_HEADER_FLAG> <PAYLOAD_SIZE>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    int apid = atoi(argv[3]);
    int packet_type = atoi(argv[4]);
    int sec_header_flag = atoi(argv[5]);
    size_t payload_size = atoi(argv[6]);
    int seq_count = 0;

    // Initialize Python interpreter
    init_space_packet_sender();

    char hex_input[2 * payload_size + 2]; // Buffer for hex string input
    unsigned char *payload_buffer = calloc(1, payload_size);
    if (!payload_buffer) {
        perror("Failed to allocate payload buffer");
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        free(payload_buffer);
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
        free(payload_buffer);
        finalize_space_packet_sender();
        return EXIT_FAILURE;
    }

    // Read from stdin until EOF (end-of-file)
    while (fgets(hex_input, sizeof(hex_input), stdin))
    {
        // Check if the input was too long and flush the remainder of the line from stdin
        if (strchr(hex_input, '\n') == NULL) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        } else {
            // If input fit, just remove the newline character
            hex_input[strcspn(hex_input, "\n")] = 0;
        }

        // Check for exit condition AFTER cleaning the input
        if (strcmp(hex_input, "exit") == 0) {
            break;
        }

        // --- THIS IS THE KEY ---
        // Manually truncate the string if it's longer than the max allowed hex characters
        size_t max_hex_len = 2 * payload_size;
        if (strlen(hex_input) > max_hex_len) {
            hex_input[max_hex_len] = '\0';
            printf("Input truncated to: %s\n", hex_input); // Optional: inform the user
        }
        // --- END KEY ---

        if (!is_valid_hex(hex_input)) {
            continue;
        }
        
        // Clear the buffer with zeros before copying new data
        memset(payload_buffer, 0, payload_size);

        // Convert hex string to bytes and copy into the buffer
        int bytes_converted = hex_string_to_bytes(hex_input, payload_buffer, payload_size);
        if (bytes_converted < 0) {
            continue;
        }

        size_t packet_size;
        char *packet = build_space_packet(apid, seq_count, (const unsigned char*)payload_buffer,
                                          packet_type, sec_header_flag, &packet_size, payload_size);

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
            // This print statement might be too verbose for a pipe utility,
            // but I'm leaving it for now. It can be removed if you want a "silent" tool.
            fprintf(stderr, "Packet sent with payload size %zu\n", payload_size);
        }

        free(packet);
        seq_count++;
    }

    close(sock);
    free(payload_buffer);
    // Finalize Python interpreter
    finalize_space_packet_sender();
    return EXIT_SUCCESS;
}
