import argparse
import socket
from spacepackets.ccsds.spacepacket import SpacePacketHeader, PacketType, SequenceFlags

def build_space_packet(apid, seq_count, user_data):
    # Create a SpacePacketHeader
    sp_header = SpacePacketHeader(
        packet_type=PacketType.TC,  # Telecommand packet
        apid=apid,
        seq_count=seq_count,
        data_len=len(user_data) - 1,
        sec_header_flag=False,
        seq_flags=SequenceFlags.UNSEGMENTED
    )
    # Serialize the header
    header_bytes = sp_header.pack()
    # Combine header and payload
    return header_bytes + user_data

def send_udp_packet(ip, port, packet):
    # Create a UDP socket
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
        udp_socket.sendto(packet, (ip, port))
        print(f"Packet sent to {ip}:{port}")

if __name__ == "__main__":
    # Set up argument parser
    parser = argparse.ArgumentParser(description="Send a CCSDS space packet over UDP")
    parser.add_argument("ip", help="Destination IP address")
    parser.add_argument("port", type=int, help="Destination port")
    parser.add_argument("--apid", type=int, default=0x02, help="APID of the packet (default: 0x02)")
    parser.add_argument("--seq_count", type=int, default=0, help="Sequence count (default: 0)")
    parser.add_argument("--payload", type=str, default="01020304", help="Payload data in hex format (default: '01020304')")

    # Parse arguments
    args = parser.parse_args()

    # Convert payload from hex string to bytes
    try:
        user_data = bytes.fromhex(args.payload)
    except ValueError:
        print("Invalid payload format. Must be a hex string.")
        exit(1)

    # Build the space packet
    space_packet = build_space_packet(args.apid, args.seq_count, user_data)

    # Print the packet in HEX format for verification
    print(f"Space packet (hex): {space_packet.hex()}")

    # Send the packet via UDP
    send_udp_packet(args.ip, args.port, space_packet)


