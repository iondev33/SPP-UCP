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
If you want to run the tests, it is recommended to install `pytest` and `coverage` (optional) first: (Optionally, they can also be installed using conda.)

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

