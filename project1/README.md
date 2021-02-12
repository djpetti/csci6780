# Building the Code

Project 1 code can be built on most Linux machines.

## Requirements

- A somewhat modern compiler. This project uses C++17 features, so support for that 
  is required.
- A working [protobuf compiler](https://developers.google.com/protocol-buffers). On
  Ubuntu systems, you might also be able to install the `protobuf-compiler` package.
  This works on Ubuntu 20.04, but it will likely install too old of a version for
  older releases.
- Cmake. Most distributions have a `cmake` package that will suffice.

## Build Instructions

In the `project1` directory:
```shell
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Server and client executables will be in `build/server/server` and 
`build/client/client`, respectively.

## Members

- Jake Chandler 
- Daniel Petti
- Nicholas Stonecipher

This project was done in its entirety by Jake Chandler, Daniel Petti, and Nicholas Stonecipher. We hereby state that we have not received unauthorized help of any form.
