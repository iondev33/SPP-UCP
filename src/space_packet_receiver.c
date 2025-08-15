#include "space_packet_receiver.h"
#include <string.h> // For memcpy
#include <stdio.h>

int parse_space_packet(const unsigned char *packet, size_t packet_size, SpacePacketHeader *header, unsigned char *payload) {
    // Parameter validation - check for NULL pointers
    if (packet == NULL) {
        fprintf(stderr, "Error: packet parameter is NULL\n");
        return SPP_ERROR_NULL_PACKET;
    }
    
    if (header == NULL) {
        fprintf(stderr, "Error: header parameter is NULL\n");
        return SPP_ERROR_NULL_HEADER;
    }
    
    if (payload == NULL) {
        fprintf(stderr, "Error: payload parameter is NULL\n");
        return SPP_ERROR_NULL_PAYLOAD_BUFFER;
    }
    
    // Check for minimum header size
    if (packet_size < 6) {
        fprintf(stderr, "Error: packet too short (%zu bytes, minimum 6 required)\n", packet_size);
        return SPP_ERROR_PACKET_TOO_SHORT;
    }

    // Parse the primary header fields
    header->version = (packet[0] >> 5) & 0x07;
    header->packet_type = (packet[0] >> 4) & 0x01;
    header->sec_header_flag = (packet[0] >> 3) & 0x01;
    header->apid = ((packet[0] & 0x07) << 8) | packet[1];

    // Extract sequence flags and count
    header->seq_flags = (packet[2] >> 6) & 0x03;
    header->seq_count = ((packet[2] & 0x3F) << 8) | packet[3];

    // Get the data length
    header->data_len = (packet[4] << 8) | packet[5];
    header->data_len += 1; // Per CCSDS standard, length is encoded as N-1

    // Integrity check: ensure the received packet size matches the expected size
    if (packet_size < header->data_len + 6) {
        fprintf(stderr, "Error: incomplete packet - expected %zu bytes, got %zu bytes\n", 
                header->data_len + 6, packet_size);
        return SPP_ERROR_INCOMPLETE_PACKET;
    }

    // Copy the payload into the provided buffer
    memcpy(payload, packet + 6, header->data_len);

    return SPP_SUCCESS;
}