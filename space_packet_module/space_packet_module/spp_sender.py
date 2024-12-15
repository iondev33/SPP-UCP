from spacepackets.ccsds.spacepacket import SpacePacketHeader, PacketType, SequenceFlags

def build_space_packet(apid, seq_count, user_data, packet_type=PacketType.TM, sec_header_flag=False):
    """
    Build a CCSDS-compliant space packet.

    Parameters:
        apid (int): Application Process Identifier.
        seq_count (int): Sequence count.
        user_data (bytes): User data payload.
        packet_type (PacketType): Packet type, default is PacketType.TM (Telemetry).
        sec_header_flag (bool): Secondary header flag, default is False.
    
    Returns:
        bytes: The constructed CCSDS-compliant space packet.
    """
    # Validate input types
    if not isinstance(user_data, (bytes, bytearray)):
        raise TypeError("user_data must be of type 'bytes' or 'bytearray'")
    
    if not isinstance(packet_type, PacketType):
        raise ValueError("packet_type must be an instance of PacketType")

    sp_header = SpacePacketHeader(
        packet_type=packet_type,      # Packet type: TC or TM
        apid=apid,                   # Application Process Identifier
        seq_count=seq_count,         # Sequence count
        data_len=len(user_data) - 1, # Data length (minus 1 for CCSDS compatibility)
        sec_header_flag=sec_header_flag,  # Secondary header flag
        seq_flags=SequenceFlags.UNSEGMENTED  # Default sequence flag
    )
    header_bytes = sp_header.pack()
    return bytes(header_bytes + user_data)  # Ensure the return is always 'bytes'


