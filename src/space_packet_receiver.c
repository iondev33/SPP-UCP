#include "space_packet_receiver.h"
#include <stdlib.h>
#include <string.h>

int parse_space_packet(const char *packet, size_t packet_size, SpacePacketHeader *header, char **payload) {
    if (packet_size < 6) {
        return -1; // Packet too small
    }

    // Extract header fields
    header->packet_type = (packet[0] >> 4) & 0x01; // 1 bit for packet type
    header->sec_header_flag = (packet[0] >> 3) & 0x01; // 1 bit for secondary header flag
    header->apid = ((packet[0] & 0x07) << 8) | (unsigned char)packet[1];
    header->seq_count = ((unsigned char)packet[2] << 8) | (unsigned char)packet[3];
    header->data_len = ((unsigned char)packet[4] << 8) | (unsigned char)packet[5] + 1;

    if (packet_size < header->data_len + 6) {
        return -2; // Incomplete packet
    }

    // Extract payload
    *payload = malloc(header->data_len);
    if (!*payload) {
        return -3; // Memory allocation failed
    }
    memcpy(*payload, packet + 6, header->data_len);

    return 0; // Success
}
