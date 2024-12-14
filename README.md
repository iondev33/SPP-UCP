# SPP-UCP - A CCSDS Space Packet Protocol Implementation with an UDP-based Communication Provider

12/13/2024

## Introduction

This repository contains the source code for the SPP-UCP project, which is a CCSDS Space Packet Protocol (SPP) implementation as an Underlying Communication Provider (UCP). 

This project leverages the `us-irs/spacepacket-py` GitHub repository to provide a Python implementation of the CCSDS Space Packet Protocol.

This project aims to develop a SPP-UCP implementation that can be used to test the SPP CLA implementation for ION based on Annex B of the CCSDS Bundle Protocol Version 7 Orange Book. The project attempts to build the following:

## Install Dependencies

To build actual CCSDS Space Packets, we need to use the `spacepackets` python package. See [documentation.](https://spacepackets.readthedocs.io/en/latest/examples.html)

Install the `spacepackets` package using pip: 

```
python3 -m pip install spacepackets
```

### Running Tests
If you want to run the tests in the `spacepacket-py` package, it is recommended to install `pytest` and `coverage`:

```
python3 -m pip install coverage pytest
```

Running tests by first change directory into the `tests` directory and run:

```
pytest .
```

Or running tests with `coverage`:

```
coverage run -m pytest
```

### Example of using the spacepackets package

See the `recv_space_packet_udp.py` and `send_space_packet_udp.py` files for examples of using the spacepackets package to send and receive space packets using UDP.

## Build the space packet sender C program

1. Install the `space_packet_module` to be globally accessible.

``` bash
sudo pip3 install .
```

2. Confirm it is installed

```bash
# Go to a different directory and run:
python3 -c "import space_packet_module; print('Module found')"
```
3. Check python version

```
python3 --version
```

3. Install the Python C development package in order to  build the C wrapper functions for python:

```
sudo apt-get install python3-dev
```

4. Verify the installlation:

```
find /usr/include -name Python.h
```

5. Confirm Python include path:

```
python3-config --includes
```

1. Build the `space_packet_sender.c` program:

```bash
gcc -o space_packet_sender space_packet_sender.c $(python3-config --cflags --ldflags)
```

If you get error that the linker cannot find the python development library because the `python3-config --ldflags` command does not include it, then you may need to explicitly to the flags. In the following example, python version was 3.10, as an example):

```bash
gcc -o space_packet_sender space_packet_sender.c $(python3-config --cflags --ldflags) -lpython3.10
```



