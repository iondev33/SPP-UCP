#ifndef SPACE_PACKET_SENDER_H
#define SPACE_PACKET_SENDER_H

#include <stdlib.h> // For size_t

// Parameter validation constants
#define SPP_MAX_APID 2047
#define SPP_MAX_SEQ_COUNT 16383
#define SPP_PACKET_TYPE_TM 0
#define SPP_PACKET_TYPE_TC 1

void init_space_packet_sender(void);
void finalize_space_packet_sender(void);

/**
 * @brief Builds a CCSDS space packet with the given parameters.
 *
 * @param apid Application Process Identifier (0-2047)
 * @param seq_count Sequence count (0-16383)
 * @param payload_data Pointer to payload data (cannot be NULL if payload_len > 0)
 * @param packet_type Packet type (0=TM, 1=TC)
 * @param sec_header_flag Secondary header flag (0 or 1)
 * @param packet_size Pointer to store the resulting packet size (cannot be NULL)
 * @param payload_len Length of payload data
 * @return Pointer to allocated packet data on success, NULL on error
 *
 * @note The caller is responsible for freeing the returned packet with free()
 * @note If payload_len is 0, a minimal valid packet will be created
 */
char *build_space_packet(int apid, int seq_count, const unsigned char *payload_data,
	int packet_type, int sec_header_flag, size_t *packet_size, size_t payload_len);

#endif // SPACE_PACKET_SENDER_H
