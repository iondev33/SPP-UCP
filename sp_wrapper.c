#include <Python.h>
#include <stdint.h>
#include <stdlib.h>

// Wrapper function to build a space packet byte stream
char* build_space_packet(uint16_t apid, uint16_t seq_count, const uint8_t* payload, size_t payload_size, size_t* out_packet_size) {
    PyObject *pModule, *pFunc, *pArgs, *pValue;
    char *byte_stream = NULL;

    // Initialize Python Interpreter
    Py_Initialize();

    // Import the Python module where the space packet API is defined
    pModule = PyImport_ImportModule("space_packet_builder"); // Replace with your script name
    if (pModule == NULL) {
        PyErr_Print();
        fprintf(stderr, "Failed to load space_packet_builder module\n");
        Py_Finalize();
        return NULL;
    }

    // Get the function from the module
    pFunc = PyObject_GetAttrString(pModule, "build_space_packet"); // Replace with your Python function name
    if (pFunc == NULL || !PyCallable_Check(pFunc)) {
        PyErr_Print();
        fprintf(stderr, "Failed to load build_space_packet function\n");
        Py_DECREF(pModule);
        Py_Finalize();
        return NULL;
    }

    // Create Python arguments: APID, sequence count, and payload (as bytes object)
    PyObject *pyPayload = PyBytes_FromStringAndSize((const char*)payload, payload_size);
    pArgs = Py_BuildValue("(HHO)", apid, seq_count, pyPayload);

    // Call the Python function
    pValue = PyObject_CallObject(pFunc, pArgs);

    // Clean up arguments
    Py_DECREF(pArgs);
    Py_DECREF(pyPayload);

    if (pValue != NULL) {
        // Extract the byte stream from the Python bytes object
        if (PyBytes_Check(pValue)) {
            *out_packet_size = PyBytes_Size(pValue);
            byte_stream = malloc(*out_packet_size);
            if (byte_stream != NULL) {
                memcpy(byte_stream, PyBytes_AsString(pValue), *out_packet_size);
            }
        }
        Py_DECREF(pValue);
    } else {
        PyErr_Print();
        fprintf(stderr, "Python function call failed\n");
    }

    // Clean up Python references and finalize
    Py_DECREF(pFunc);
    Py_DECREF(pModule);
    Py_Finalize();

    return byte_stream;
}
