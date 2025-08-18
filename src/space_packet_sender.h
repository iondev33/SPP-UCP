#ifndef SPACE_PACKET_SENDER_H
#define SPACE_PACKET_SENDER_H

#include <stdlib.h> // For size_t

// Parameter validation constants
#define SPP_MAX_APID 2047
#define SPP_MAX_SEQ_COUNT 16383
#define SPP_PACKET_TYPE_TM 0
#define SPP_PACKET_TYPE_TC 1

/**
 * @brief Initialize the SPP sender subsystem.
 * 
 * This function initializes the Python interpreter and must be called once
 * before any calls to build_space_packet() or packet_request().
 * 
 * @note This function is NOT thread-safe and should be called from the main thread.
 * @note Call finalize_space_packet_sender() to clean up resources before program exit.
 * 
 * @warning Do not call this function multiple times without calling 
 *          finalize_space_packet_sender() in between.
 */
void init_space_packet_sender(void);

/**
 * @brief Finalize the SPP sender subsystem.
 * 
 * This function cleans up the Python interpreter and should be called once
 * at program shutdown after all SPP operations are complete.
 * 
 * @note This function is NOT thread-safe and should be called from the main thread.
 * @note After calling this function, build_space_packet() and packet_request() 
 *       will fail until init_space_packet_sender() is called again.
 */
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
 * @note init_space_packet_sender() must be called before using this function
 * 
 * @warning This function is NOT thread-safe due to Python interpreter usage
 */
char *build_space_packet(int apid, int seq_count, const unsigned char *payload_data,
    int packet_type, int sec_header_flag, size_t *packet_size, size_t payload_len);

/**
 * @brief Send a space packet using the configured UDP transport.
 * 
 * This function builds a space packet and transmits it to the compile-time
 * configured destination IP address and port.
 * 
 * @param byte_payload Pointer to payload data
 * @param apid Application Process Identifier (0-2047)  
 * @param seq_count Sequence count (0-16383)
 * @param packet_type Packet type (0=TM, 1=TC)
 * @param sec_header_flag Secondary header flag (0 or 1)
 * @param to_send_bytes Length of payload data
 * @return Number of bytes sent on success, -1 on error
 * 
 * @note init_space_packet_sender() must be called before using this function
 * @note The destination IP and port are configured at compile time
 * @note This function creates and manages its own UDP socket
 * 
 * @warning This function is NOT thread-safe due to Python interpreter usage
 * @warning The calling application is responsible for calling init_space_packet_sender()
 *          before using this function and finalize_space_packet_sender() at shutdown
 */
int packet_request(unsigned char *byte_payload, int apid, int seq_count, 
                   int packet_type, int sec_header_flag, size_t to_send_bytes);

#endif // SPACE_PACKET_SENDER_H