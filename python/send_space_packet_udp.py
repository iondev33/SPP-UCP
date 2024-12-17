import argparse
import socket
from spacepackets.ccsds.spacepacket import SpacePacketHeader, PacketType, SequenceFlags

def build_space_packet(apid, seq_count, user_data, packet_type=PacketType.TC, sec_header_flag=False):
    """
    Build a CCSDS-compliant space packet.

    Args:
        apid (int): Application Process Identifier.
        seq_count (int): Sequence count.
        user_data (bytes): Payload data in bytes.
        packet_type (PacketType): Packet type, either PacketType.TC (Telecommand) or PacketType.TM (Telemetry).
        sec_header_flag (bool): Secondary header flag.

    Returns:
        bytes: Serialized CCSDS-compliant space packet.
    """
    # Create a SpacePacketHeader
    sp_header = SpacePacketHeader(
        packet_type=packet_type,
        apid=apid,
        seq_count=seq_count,
        data_len=len(user_data) - 1,
        sec_header_flag=sec_header_flag,
        seq_flags=SequenceFlags.UNSEGMENTED
    )
    # Serialize the header
    header_bytes = sp_header.pack()
    # Combine header and payload
    return header_bytes + user_data

def send_udp_packet(ip, port, packet):
    """Send a packet over UDP."""
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
    parser.add_argument("--packet_type", type=int, choices=[0, 1], default=1,
                        help="Packet type: 0 for Telemetry (TM), 1 for Telecommand (TC) (default: 1)")
    parser.add_argument("--sec_header_flag", type=int, choices=[0, 1], default=0,
                        help="Secondary header flag: 0 for no secondary header, 1 for secondary header (default: 0)")

    # Parse arguments
    args = parser.parse_args()

    # Convert payload from hex string to bytes
    try:
        user_data = bytes.fromhex(args.payload)
    except ValueError:
        print("Invalid payload format. Must be a hex string.")
        exit(1)

    # Convert packet_type and sec_header_flag
    packet_type = PacketType.TM if args.packet_type == 0 else PacketType.TC
    sec_header_flag = bool(args.sec_header_flag)

    # Build the space packet
    space_packet = build_space_packet(args.apid, args.seq_count, user_data, packet_type, sec_header_flag)

    # Print the packet in HEX format for verification
    print(f"Space packet (hex): {space_packet.hex()}")

    # Send the packet via UDP
    send_udp_packet(args.ip, args.port, space_packet)

