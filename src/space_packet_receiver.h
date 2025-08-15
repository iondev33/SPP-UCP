#ifndef SPACE_PACKET_RECEIVER_H
#define SPACE_PACKET_RECEIVER_H

#include <stdlib.h> // For size_t

// Error codes for parse_space_packet function
#define SPP_SUCCESS 0
#define SPP_ERROR_PACKET_TOO_SHORT -1
#define SPP_ERROR_INCOMPLETE_PACKET -2
#define SPP_ERROR_NULL_PACKET -3
#define SPP_ERROR_NULL_HEADER -4
#define SPP_ERROR_NULL_PAYLOAD_BUFFER -5

// Represents the header of a CCSDS Space Packet
typedef struct {
    int version;
    int packet_type;
    int sec_header_flag;
    int apid;
    int seq_flags; // Added this member
    int seq_count;
    size_t data_len;
} SpacePacketHeader;

/**
 * @brief Parses a raw byte stream into a Space Packet Header and payload.
 *
 * @param packet The raw byte stream received from the network.
 * @param packet_size The total size of the received byte stream.
 * @param header A pointer to a SpacePacketHeader struct to be populated.
 * @param payload A pointer to a buffer where the payload will be copied.
 * @return 0 on success, -1 if packet is too short, -2 if packet is incomplete.
 */
int parse_space_packet(const unsigned char *packet, size_t packet_size, SpacePacketHeader *header, unsigned char *payload);

#endif // SPACE_PACKET_RECEIVER_H

