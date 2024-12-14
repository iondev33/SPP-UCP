import argparse
import socket
from spacepackets.ccsds.spacepacket import SpacePacketHeader

def receive_udp_packet(ip, port):
    """Set up a UDP socket to receive a packet."""
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
        udp_socket.bind((ip, port))
        print(f"Listening on {ip}:{port}...")
        data, addr = udp_socket.recvfrom(1024)  # Adjust buffer size if necessary
        print(f"Received packet from {addr}")
        return data

def parse_space_packet(packet):
    """Parse the space packet and extract the header and payload."""
    # Parse the header (first 6 bytes)
    try:
        sp_header = SpacePacketHeader.unpack(packet[:6])
        payload = packet[6:]  # Remaining bytes are the payload
        return sp_header, payload
    except Exception as e:
        print(f"Error parsing space packet: {e}")
        return None, None

if __name__ == "__main__":
    # Set up argument parser
    parser = argparse.ArgumentParser(description="Receive and parse a CCSDS space packet over UDP")
    parser.add_argument("ip", help="IP address to listen on")
    parser.add_argument("port", type=int, help="Port to listen on")
    args = parser.parse_args()

    # Receive the packet
    packet = receive_udp_packet(args.ip, args.port)

    # Parse the space packet
    sp_header, payload = parse_space_packet(packet)

    if sp_header is not None:
        # Print the header fields
        print("Space Packet Header Information:")
        print(f"  Packet Type: {'TC' if sp_header.packet_type == 0 else 'TM'}")
        print(f"  Secondary Header Flag: {sp_header.sec_header_flag}")
        print(f"  APID: {sp_header.apid}")
        print(f"  Sequence Flags: {sp_header.seq_flags}")
        print(f"  Sequence Count: {sp_header.seq_count}")
        print(f"  Data Length: {sp_header.data_len + 1}")

        # Print the payload
        print(f"Payload (hex): {payload.hex()}")
    else:
        print("Failed to parse the space packet.")
