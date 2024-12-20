#ifndef SPACE_PACKET_RECEIVER_H
#define SPACE_PACKET_RECEIVER_H

#include <stddef.h>

// Space Packet Header structure
typedef struct {
    int apid;
    int seq_count;
    int packet_type;
    int sec_header_flag;
    size_t data_len;
} SpacePacketHeader;

// Function to parse space packets
int parse_space_packet(const char *packet, size_t packet_size, SpacePacketHeader *header, char **payload);

#endif // SPACE_PACKET_RECEIVER_H
