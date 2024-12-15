from setuptools import setup, find_packages

setup(
    name='space_packet_module',
    version='0.1.0',
    packages=find_packages(),
    install_requires=[
        'spacepackets',  # Add any dependencies here
    ],
    author='iondev33',
    author_email='jgao@jpl.caltech.edu',
    description='A module to build and process CCSDS-compliant space packets',
    classifiers=[
        'Programming Language :: Python :: 3',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
    ],
    python_requires='>=3.9',
)
