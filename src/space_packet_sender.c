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

int is_valid_hex(const char *hex_payload)
{
    size_t len = strlen(hex_payload);
    if (len % 2 != 0)
	{
        fprintf(stderr, "Payload length must be even\n");
        return 0;
    }
    for (size_t i = 0; i < len; i++)
	{
        if (!isxdigit(hex_payload[i]))
		{
            fprintf(stderr, "Invalid character in payload: %c\n",
				hex_payload[i]);
            return 0;
        }
    }
    return 1;
}

char *build_space_packet(int apid, int seq_count, const unsigned char *payload_data,
			 int packet_type, int sec_header_flag, size_t *packet_size, size_t payload_len)
{
    PyObject *pModule = NULL, *pFunc = NULL, *pArgs = NULL, *pValue = NULL,
             *pPacketType = NULL;
    char *byte_stream = NULL;
    init_space_packet_sender();

    // Import Python Module
    pModule = PyImport_ImportModule("space_packet_module");
    if (!pModule) {
        PyErr_Print();
        fprintf(stderr, "Failed to load space_packet_module\n");
        goto cleanup;
    }

    // Import PacketType Enum
    PyObject *pPacketTypeEnum = PyObject_GetAttrString(pModule, "PacketType");
    if (!pPacketTypeEnum)
	{
        PyErr_Print();
        fprintf(stderr, "Failed to load PacketType enum\n");
        goto cleanup;
    }

    // Convert packet_type to Python PacketType Enum
    if (packet_type == 1)
	{
        pPacketType = PyObject_GetAttrString(pPacketTypeEnum,
			"TC"); // Telecommand
    }
	 else if (packet_type == 0)
	{
        pPacketType = PyObject_GetAttrString(pPacketTypeEnum,
			 "TM"); // Telemetry
    }
	else
	{
        fprintf(stderr, "Invalid packet_type value: %d\n", packet_type);
        goto cleanup;
    }

    if (!pPacketType)
	{
        PyErr_Print();
        fprintf(stderr, "Failed to convert packet_type to PacketType enum\n");
        goto cleanup;
    }

    // Build Python Arguments using the raw payload data
    pArgs = Py_BuildValue("(iiy#Oi)", apid, seq_count, payload_data, payload_len,
                          pPacketType, sec_header_flag);
    if (!pArgs)
	{
        PyErr_Print();
        fprintf(stderr, "Failed to build Python arguments\n");
        goto cleanup;
    }

    // Call the Python Function
    pFunc = PyObject_GetAttrString(pModule, "build_space_packet");
    if (!pFunc || !PyCallable_Check(pFunc))
	{
        PyErr_Print();
        fprintf(stderr, "Failed to load build_space_packet function\n");
        goto cleanup;
    }

    pValue = PyObject_CallObject(pFunc, pArgs);
    if (!pValue)
	{
        PyErr_Print();
        fprintf(stderr, "Python function call failed\n");
        goto cleanup;
    }

    // Extract Byte Stream
    if (PyBytes_Check(pValue))
	{
        *packet_size = PyBytes_Size(pValue);
        byte_stream = malloc(*packet_size);
        if (byte_stream)
		{
            memcpy(byte_stream, PyBytes_AsString(pValue), *packet_size);
        }
		else
		{
            perror("Failed to allocate memory for byte stream");
        }
    }
	else
	{
        fprintf(stderr, "Python function did not return a bytes object\n");
    }

cleanup:
    Py_XDECREF(pArgs);
    Py_XDECREF(pPacketType);
    Py_XDECREF(pPacketTypeEnum);
    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);
    Py_XDECREF(pValue);
    return byte_stream;
}