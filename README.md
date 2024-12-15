# SPP-UCP - A CCSDS Space Packet Protocol Implementation with an UDP-based Communication Provider

12/13/2024

## Introduction

This repository contains the source code for the SPP-UCP project, which is a CCSDS Space Packet Protocol (SPP) implementation as an Underlying Communication Provider (UCP). 

This project leverages the `spacepackets` python package to build and parse CCSDS Space Packets.

This project aims to develop a SPP-UCP implementation that can be used to test the SPP CLA implementation for ION based on Annex B of the CCSDS Bundle Protocol Version 7 Orange Book. The project attempts to build the following:

## Install Dependencies

The `spacepackets` package (https://pypi.org/project/spacepackets/) requires python version >= 3.9. Check the version:

```bash
python --version
```

If you don't want to upgrade to higher python version, please install and enable a higher version but not as default. Example in RHEL:

```bash
sudo dnf module enable python39
sudo dnf install python39
```
This will install Python 3.9 as /usr/bin/python3.9 without affecting the default /usr/bin/python3 (Python 3.6).

Check python version again. Sometimes this may upgrade to a different version. In the RHEL8 example, the python version was actually upgraded to 3.11.

It might be a good idea to set up python virtual environment using required python version. For example, if the only python version on the host higher than 3.9 is 3.11, then use it to create the python environment, which will make it the default version in that environment.

```bash
python3.11 -m venv spp-ucp-env
```

Activate virtual environment:

```bash
source ./spp-ucp-env/bin/activate
```

To build actual CCSDS Space Packets, we use the `spacepackets` python package. See [documentation.](https://spacepackets.readthedocs.io/en/latest/examples.html)

Install the `spacepackets` package using pip: 

```
python -m pip install spacepackets
```

## Example of using the `spacepackets` package

The `send_space_packet_udp.py` script will construct a spacepacket and send it via UDP. The `recv_space_packet_udp.py` script will receive a packet over UDP, parse it, and display its content.

Start the receive script:

```bash
python recv_space_packet_udp.py 127.0.0.1 5000
```

Send a spacepacket:

```bash
python send_space_packet_udp.py --apid 250 --seq_count 1 --payload "01020304" 127.0.0.1 5000
```

## Build the `space_packet_sender.c` Program

Install the `space_packet_module`.

``` bash
cd ./space_packet_module
python -m pip install .
```

Confirm it is installed from any directory (other than where the modules is stored.)

```bash
# Go to a different directory and run:
python -c "import space_packet_module; print('Module found')"
```

Install the Python C development package in order to  build the C wrapper functions for python:

This must be install on the host so you need to specify the exact version. So you need to specify python3.11-dev instead of just python3-dev.

Again, from inside the virtual environment where the module is installed, run `python --version' to confirm the actual version of python in the virtual environment. For example it is 3.11. Then install the development package specific to that version:

For Ubuntu
```bash
sudo apt-get install python3.11-dev
```

For RHEL8:
```bash
sudo subscription-manager repos --enable codeready-builder-for-rhel-8-$(arch)-rpms
sudo dnf module enable python:3.11
sudo dnf install python3.11 python3.11-devel
```

Verify installation of the development header:

```bash
ls /usr/include/python3.11/Python.h
```
ls /usr

Confirm Python include path:

```bash
python3.11-config --includes
python3.11-config --ldflags
```

Take note if the `-lpython3.11` is part of the include flag output. If not, make sure the shared library is avilable:

```bash
find /usr -name "libpython3.11.so"
```

If it is available, then you just have to manually add the flag to the compiler.

## Build the `space_packet_sender.c` Program

```bash
# Compile command that manually adds -lpython3.11 at the end
gcc -g -o space_packet_sender space_packet_sender.c $(python3.11-config --includes) $(python3.11-config --ldflags) $(python3.11-config --libs) -lpython3.11
```





