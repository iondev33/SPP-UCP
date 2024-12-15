#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAX_PAYLOAD_SIZE 1024

// Validate HEX input payload
int is_valid_hex(const char* hex_payload) {
    size_t len = strlen(hex_payload);
    if (len % 2 != 0) {
        fprintf(stderr, "Payload length must be even\n");
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        if (!isxdigit(hex_payload[i])) {
            fprintf(stderr, "Invalid character in payload: %c\n", hex_payload[i]);
            return 0;
        }
    }
    return 1;
}

char* build_space_packet(int apid, int seq_count, const char* hex_payload, int packet_type, int sec_header_flag, size_t* packet_size) {
    PyObject *pModule = NULL, *pFunc = NULL, *pArgs = NULL, *pValue = NULL, *pPacketType = NULL;
    char *byte_stream = NULL;

    // Initialize Python Interpreter
    Py_Initialize();

    // Import Python Module
    pModule = PyImport_ImportModule("space_packet_module");
    if (!pModule) {
        PyErr_Print();
        fprintf(stderr, "Failed to load space_packet_module\n");
        goto cleanup;
    }

    // Import PacketType Enum
    PyObject *pPacketTypeEnum = PyObject_GetAttrString(pModule, "PacketType");
    if (!pPacketTypeEnum) {
        PyErr_Print();
        fprintf(stderr, "Failed to load PacketType enum\n");
        goto cleanup;
    }

    // Convert packet_type to Python PacketType Enum
    if (packet_type == 1) {
        pPacketType = PyObject_GetAttrString(pPacketTypeEnum, "TC");  // Telecommand
    } else if (packet_type == 0) {
        pPacketType = PyObject_GetAttrString(pPacketTypeEnum, "TM");  // Telemetry
    } else {
        fprintf(stderr, "Invalid packet_type value: %d\n", packet_type);
        goto cleanup;
    }

    if (!pPacketType) {
        PyErr_Print();
        fprintf(stderr, "Failed to convert packet_type to PacketType enum\n");
        goto cleanup;
    }

    // Convert HEX payload to bytes
    size_t payload_len = strlen(hex_payload) / 2;
    char* payload = malloc(payload_len);
    if (!payload) {
        perror("Failed to allocate memory for payload");
        goto cleanup;
    }
    for (size_t i = 0; i < payload_len; i++) {
        sscanf(&hex_payload[i * 2], "%2hhx", &payload[i]);
    }

    // Build Python Arguments
    pArgs = Py_BuildValue("(iiy#Oi)", apid, seq_count, payload, payload_len, pPacketType, sec_header_flag);
    free(payload);
    if (!pArgs) {
        PyErr_Print();
        fprintf(stderr, "Failed to build Python arguments\n");
        goto cleanup;
    }

    // Call the Python Function
    pFunc = PyObject_GetAttrString(pModule, "build_space_packet");
    if (!pFunc || !PyCallable_Check(pFunc)) {
        PyErr_Print();
        fprintf(stderr, "Failed to load build_space_packet function\n");
        goto cleanup;
    }

    pValue = PyObject_CallObject(pFunc, pArgs);
    if (!pValue) {
        PyErr_Print();
        fprintf(stderr, "Python function call failed\n");
        goto cleanup;
    }

    // Extract Byte Stream
    if (PyBytes_Check(pValue)) {
        *packet_size = PyBytes_Size(pValue);
        byte_stream = malloc(*packet_size);
        if (byte_stream) {
            memcpy(byte_stream, PyBytes_AsString(pValue), *packet_size);
        } else {
            perror("Failed to allocate memory for byte stream");
        }
    } else {
        fprintf(stderr, "Python function did not return a bytes object\n");
    }

cleanup:
    Py_XDECREF(pArgs);
    Py_XDECREF(pPacketType);
    Py_XDECREF(pPacketTypeEnum);
    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);
    Py_XDECREF(pValue);

    Py_Finalize();
    return byte_stream;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <IP> <PORT> <APID> <PACKET_TYPE> <SEC_HEADER_FLAG>\n", argv[0]);
        fprintf(stderr, "PACKET_TYPE: 1 for Telecommand (TC), 0 for Telemetry (TM)\n");
        fprintf(stderr, "SEC_HEADER_FLAG: 0 for No Secondary Header, 1 for Secondary Header\n");
        return EXIT_FAILURE;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int apid = atoi(argv[3]);
    int packet_type = atoi(argv[4]);
    int sec_header_flag = atoi(argv[5]);
    int seq_count = 0;

    if (packet_type != 0 && packet_type != 1) {
        fprintf(stderr, "Invalid PACKET_TYPE: must be 0 (TC) or 1 (TM)\n");
        return EXIT_FAILURE;
    }
    if (sec_header_flag != 0 && sec_header_flag != 1) {
        fprintf(stderr, "Invalid SEC_HEADER_FLAG: must be 0 or 1\n");
        return EXIT_FAILURE;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(sock);
        return EXIT_FAILURE;
    }

    char hex_payload[MAX_PAYLOAD_SIZE];
    while (1) {
        printf("Enter payload in HEX (or 'exit' to quit): ");
        if (!fgets(hex_payload, sizeof(hex_payload), stdin)) break;
        hex_payload[strcspn(hex_payload, "\n")] = 0;

        if (strcmp(hex_payload, "exit") == 0) break;

        if (!is_valid_hex(hex_payload)) continue;

        size_t packet_size;
        char* packet = build_space_packet(apid, seq_count, hex_payload, packet_type, sec_header_flag, &packet_size);
        if (!packet) {
            fprintf(stderr, "Failed to build space packet\n");
            continue;
        }

        if (sendto(sock, packet, packet_size, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("Failed to send packet");
        } else {
            printf("Packet sent: %s\n", hex_payload);
        }

        free(packet);
        seq_count++;
    }

    close(sock);
    return EXIT_SUCCESS;
}

