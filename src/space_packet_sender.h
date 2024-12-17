#ifndef SPACE_PACKET_SENDER_H
#define SPACE_PACKET_SENDER_H

#include <stdlib.h> // For size_t

int is_valid_hex(const char* hex_payload);
char* build_space_packet(int apid, int seq_count, const char* hex_payload, int packet_type, int sec_header_flag, size_t* packet_size);

#endif // SPACE_PACKET_SENDER_H
