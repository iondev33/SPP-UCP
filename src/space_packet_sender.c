#define PY_SSIZE_T_CLEAN
#include "space_packet_sender.h"
#include <Python.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize Python interpreter (call this once at program start)
void init_space_packet_sender()
{
    Py_Initialize();
}

// Finalize Python interpreter (call this once at program end)
void finalize_space_packet_sender()
{
    Py_Finalize();
}

char *build_space_packet(int apid, int seq_count, const unsigned char *payload_data,
    int packet_type, int sec_header_flag, size_t *packet_size, size_t payload_len)
{
// Parameter validation - check packet_size pointer first
if (packet_size == NULL) {
fprintf(stderr, "Error: packet_size parameter cannot be NULL\n");
return NULL;
}

// Initialize packet_size to 0 in case of early return
*packet_size = 0;

// Validate APID range
if (apid < 0 || apid > SPP_MAX_APID) {
fprintf(stderr, "Error: APID %d out of range (0-%d)\n", apid, SPP_MAX_APID);
return NULL;
}

// Validate sequence count range
if (seq_count < 0 || seq_count > SPP_MAX_SEQ_COUNT) {
fprintf(stderr, "Error: Sequence count %d out of range (0-%d)\n", seq_count, SPP_MAX_SEQ_COUNT);
return NULL;
}

// Validate packet type
if (packet_type != SPP_PACKET_TYPE_TM && packet_type != SPP_PACKET_TYPE_TC) {
fprintf(stderr, "Error: Invalid packet type %d (must be %d=TM or %d=TC)\n", 
       packet_type, SPP_PACKET_TYPE_TM, SPP_PACKET_TYPE_TC);
return NULL;
}

// Validate secondary header flag
if (sec_header_flag != 0 && sec_header_flag != 1) {
fprintf(stderr, "Error: Invalid secondary header flag %d (must be 0 or 1)\n", sec_header_flag);
return NULL;
}

// Handle payload validation
if (payload_len > 0 && payload_data == NULL) {
fprintf(stderr, "Error: payload_data cannot be NULL when payload_len > 0 (%zu)\n", payload_len);
return NULL;
}

// Handle zero-length payload case
const unsigned char *actual_payload = payload_data;
size_t actual_payload_len = payload_len;
static const unsigned char placeholder_payload[] = {0x00};

if (payload_len == 0) {
// Python module requires non-empty payload, so use a placeholder
actual_payload = placeholder_payload;
actual_payload_len = 1;
printf("Info: Zero-length payload converted to 1-byte placeholder\n");
}

PyObject *pModule = NULL, *pFunc = NULL, *pArgs = NULL, *pValue = NULL,
    *pPacketType = NULL;
char *byte_stream = NULL;

// Import Python Module
pModule = PyImport_ImportModule("space_packet_module");
if (!pModule) {
if (PyErr_Occurred()) {
   PyErr_Print();
   PyErr_Clear(); // Clear the error to prevent segfault
}
fprintf(stderr, "Failed to load space_packet_module\n");
return NULL;
}

// Import PacketType Enum
PyObject *pPacketTypeEnum = PyObject_GetAttrString(pModule, "PacketType");
if (!pPacketTypeEnum) {
if (PyErr_Occurred()) {
   PyErr_Print();
   PyErr_Clear();
}
fprintf(stderr, "Failed to load PacketType enum\n");
goto cleanup;
}

// Convert packet_type to Python PacketType Enum
if (packet_type == SPP_PACKET_TYPE_TC) {
pPacketType = PyObject_GetAttrString(pPacketTypeEnum, "TC"); // Telecommand
} else if (packet_type == SPP_PACKET_TYPE_TM) {
pPacketType = PyObject_GetAttrString(pPacketTypeEnum, "TM"); // Telemetry
} else {
// This should never happen due to validation above, but just in case
fprintf(stderr, "Internal error: Invalid packet_type value: %d\n", packet_type);
goto cleanup;
}

if (!pPacketType) {
if (PyErr_Occurred()) {
   PyErr_Print();
   PyErr_Clear();
}
fprintf(stderr, "Failed to convert packet_type to PacketType enum\n");
goto cleanup;
}

// Build Python Arguments using the actual payload data
pArgs = Py_BuildValue("(iiy#Oi)", apid, seq_count, actual_payload, actual_payload_len,
                 pPacketType, sec_header_flag);
if (!pArgs) {
if (PyErr_Occurred()) {
   PyErr_Print();
   PyErr_Clear();
}
fprintf(stderr, "Failed to build Python arguments\n");
goto cleanup;
}

// Call the Python Function
pFunc = PyObject_GetAttrString(pModule, "build_space_packet");
if (!pFunc || !PyCallable_Check(pFunc)) {
if (PyErr_Occurred()) {
   PyErr_Print();
   PyErr_Clear();
}
fprintf(stderr, "Failed to load build_space_packet function\n");
goto cleanup;
}

pValue = PyObject_CallObject(pFunc, pArgs);
if (!pValue) {
if (PyErr_Occurred()) {
   PyErr_Print();
   PyErr_Clear();
}
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
// Always clear any remaining Python errors to prevent issues
if (PyErr_Occurred()) {
PyErr_Clear();
}

Py_XDECREF(pArgs);
Py_XDECREF(pPacketType);
Py_XDECREF(pPacketTypeEnum);
Py_XDECREF(pFunc);
Py_XDECREF(pModule);
Py_XDECREF(pValue);
return byte_stream;
}