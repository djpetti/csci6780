m# Building the Code

Project 3 code can be built on most Linux machines. See the repository README for general instructions on setting up
the build environment.

## Build Instructions

In the repository root:
```shell
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Server and client executables will be in `build/project3/coordinator/coordinator` and 
`build/project3/participant/participant`, respectively.

## Members

- Jake Chandler 
- Daniel Petti
- Nicholas Stonecipher

This project was done in its entirety by Jake Chandler, Daniel Petti, and Nicholas Stonecipher. We hereby state that we have not received unauthorized help of any form.
