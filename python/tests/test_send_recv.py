import unittest
import socket
import threading
import time
from send_space_packet_udp import build_space_packet, send_udp_packet
from recv_space_packet_udp import parse_space_packet

class TestSpacePacketSendRecv(unittest.TestCase):
    def setUp(self):
        # Setup server to receive packets
        self.server_ip = "127.0.0.1"
        self.server_port = 9000
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.server_socket.bind((self.server_ip, self.server_port))

        self.stop_event = threading.Event()
        self.server_ready = threading.Event()  # Synchronization event
        self.received_packet = None

        def udp_server():
            """UDP server thread to listen for incoming packets."""
            print("Server thread: Waiting to start...")
            self.server_socket.settimeout(0.5)  # Set timeout for graceful exit
            self.server_ready.set()  # Signal that the server is ready
            print("Server thread: Ready and listening for packets.")
            while not self.stop_event.is_set():
                try:
                    self.received_packet, _ = self.server_socket.recvfrom(1024)
                    print("Server thread: Packet received.")
                except socket.timeout:
                    continue  # Timeout allows checking stop_event
                except OSError:
                    print("Server thread: Socket closed, exiting.")
                    break

        # Start the server thread
        self.server_thread = threading.Thread(target=udp_server)
        self.server_thread.start()

        # Wait until the server is ready
        self.server_ready.wait(timeout=1)
        print("Main thread: Server is ready.")

    def tearDown(self):
        # Stop the server thread and clean up the socket
        print("Main thread: Stopping server...")
        self.stop_event.set()
        if self.server_socket:
            self.server_socket.close()
        self.server_thread.join()
        print("Main thread: Server stopped.")

    def test_send_receive_space_packet(self):
        # Build and send a space packet
        apid = 0x01
        seq_count = 0
        payload = bytes.fromhex("01020304")
        packet = build_space_packet(apid, seq_count, payload)
        print("Main thread: Built space packet.")

        # Retry sending the packet to ensure it is received
        for attempt in range(3):
            print(f"Main thread: Sending packet (attempt {attempt + 1})...")
            send_udp_packet(self.server_ip, self.server_port, packet)
            time.sleep(0.5)  # Give the server time to receive the packet

            if self.received_packet:
                print("Main thread: Packet received by server.")
                break
        else:
            self.fail("No packet was received after multiple attempts.")

        # Parse the received packet
        print("Main thread: Parsing the received packet.")
        sp_header, received_payload = parse_space_packet(self.received_packet)
        self.assertIsNotNone(sp_header, "Failed to parse space packet")
        self.assertEqual(received_payload, payload, "Payload mismatch")
        self.assertEqual(sp_header.apid, apid, "APID mismatch")
        print("Main thread: Test passed successfully.")

if __name__ == '__main__':
    unittest.main()
