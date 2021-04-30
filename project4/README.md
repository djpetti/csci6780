# Building the Code

Project 4 code can be built on most Linux machines. See the repository README for general instructions on setting up
the build environment.

## Build Instructions

In the repository root:
```shell
mkdir build && cd build
cmake ..
make -j$(nproc)
```

The nameserver executable will be in `build/project4/name_server/nameserver`.
A single executable is used for both bootstrap and normal nameservers. The first
argument can be "simple" or "bootstrap", and indicates which type to run.
For example, a bootstrap nameserver can be started with
```
./nameserver bootstrap /path/to/config.txt
```

## Members

- Jake Chandler
- Daniel Petti
- Nicholas Stonecipher

This project was done in its entirety by Jake Chandler, Daniel Petti, and Nicholas Stonecipher. We hereby state that we have not received unauthorized help of any form.
