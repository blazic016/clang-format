# DVB SSU Tool

This is linux application for creating DSM-CC compatible transport streams containing SSU files for STBs.

## Building

DVB SSU Tool uses **cmake** build system and depends on **wxWidgets**, **zlib** and **openssl** external libraries. Follow next steps to build it in debian-based environment:

    $ sudo apt-get install g++ make cmake zlib1g-dev libssl-dev libwxgtk3.0-dev
    $ mkdir -p build
    $ cd build/
    $ cmake ..
    $ make

