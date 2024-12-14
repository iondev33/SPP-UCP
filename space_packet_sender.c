#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_PAYLOAD_SIZE 1024

// Wrapper function to call Python's build_space_packet
char* build_space_packet(int apid, int seq_count, const char* hex_payload, size_t* packet_size) {
    PyObject *pModule, *pFunc, *pArgs, *pValue;
    char *byte_stream = NULL;

    // Initialize Python Interpreter
    Py_Initialize();
    
    // Import the Python module
    pModule = PyImport_ImportModule("space_packet_module");  // Match the Python file name
    if (!pModule) {
        PyErr_Print();
        fprintf(stderr, "Failed to load space_packet_module\n");
        Py_Finalize();
        return NULL;
    }

    // Get the function
    pFunc = PyObject_GetAttrString(pModule, "build_space_packet");
    if (!pFunc || !PyCallable_Check(pFunc)) {
        PyErr_Print();
        fprintf(stderr, "Failed to load build_space_packet function\n");
        Py_DECREF(pModule);
        Py_Finaliz
    ModuleNotFoundError: No module named 'space_packet_module'
Failed to load space_packet_module
Failed to build space packet
    // Get the HOME environment variable
        return NULL;
    }

    // Convert HEX payload to bytes
    size_t payload_len = strlen(hex_payload) / 2;
    char* payload = malloc(payload_len);
    if (payload == NULL) {
        perror("Failed to allocate memory for payload");
        Py_DECREF(pFunc);
        Py_DECREF(pModule);
        Py_Finalize();
        return NULL;
    }

    for (size_t i = 0; i < payload_len; i++) {
        sscanf(&hex_payload[i * 2], "%2hhx", &payload[i]);
    }

    // Create Python arguments
    pArgs = Py_BuildValue("(iiy#)", apid, seq_count, payload, payload_len);

    // Call the Python function
    pValue = PyObject_CallObject(pFunc, pArgs);

    // Clean up
    free(payload);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);

    if (pValue) {
        if (PyBytes_Check(pValue)) {
            *packet_size = PyBytes_Size(pValue);
            byte_stream = malloc(*packet_size);
            memcpy(byte_stream, PyBytes_AsString(pValue), *packet_size);
        }
        Py_DECREF(pValue);
    } else {
        PyErr_Print();
        fprintf(stderr, "Python function call failed\n");
    }

    Py_Finalize();
    return byte_stream;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <IP> <PORT> <APID>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int apid = atoi(argv[3]);
    int seq_count = 0;

    // Set up UDP socket
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
        // Prompt user for payload in HEX
        printf("Enter payload in HEX (or 'exit' to quit): ");
        if (!fgets(hex_payload, sizeof(hex_payload), stdin)) break;

        // Remove trailing newline
        hex_payload[strcspn(hex_payload, "\n")] = 0;

        if (strcmp(hex_payload, "exit") == 0) break;

        // Call Python function to build the packet
        size_t packet_size;
        char* packet = build_space_packet(apid, seq_count, hex_payload, &packet_size);
        if (!packet) {
            fprintf(stderr, "Failed to build space packet\n");
            continue;
        }

        // Send the packet over UDP
        if (sendto(sock, packet, packet_size, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("Failed to send packet");
        } else {
            printf("Packet sent: %s\n", hex_payload);
        }

        free(packet);

        // Increment sequence count
        seq_count++;
    }

    close(sock);
    return EXIT_SUCCESS;
}
