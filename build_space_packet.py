from spacepackets.ccsds.spacepacket import SpacePacketHeader, PacketType, SequenceFlags

def build_space_packet(apid, seq_count, payload):
    sp_header = SpacePacketHeader(
        packet_type=PacketType.TC,  # Telecommand packet
        apid=apid,
        seq_count=seq_count,
        data_len=len(payload) - 1,
        sec_header_flag=False,
        seq_flags=SequenceFlags.UNSEGMENTED
    )
    header_bytes = sp_header.pack()
    return header_bytes + payload
