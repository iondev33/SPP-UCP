#include "space_packet_receiver.h"
#include <string.h> // For memcpy
#include <stdio.h>

int parse_space_packet(const unsigned char *packet, size_t packet_size, SpacePacketHeader *header, unsigned char *payload) {
    // Check for minimum header size
    if (packet_size < 6) {
        return -1; // Packet too short
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
        return -2; // Incomplete packet
    }

    // Copy the payload into the provided buffer
    memcpy(payload, packet + 6, header->data_len);

    return 0; // Success
}

