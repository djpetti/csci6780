# Distributed Computing Systems Projects

This repository contains project code for
Distributed Computing Systems (CSCI 6780).

# Requirements

- A somewhat modern compiler. These projects use C++17 features, so support for that
  is required.
- A working [protobuf compiler](https://developers.google.com/protocol-buffers). On
  Ubuntu systems, you might also be able to install the `protobuf-compiler` package.
  This works on Ubuntu 20.04, but it will likely install too old of a version for
  older releases.
- Cmake. Most distributions have a `cmake` package that will suffice.

## Using a local toolchain

If, for some reason, you cannot meet the requirements above because you have an
old OS and cannot install packages (e.g. on Odin or Nike), it is possible to install
all the dependencies locally in your home directory.

To do so, run the convenient script:
```shell
./setup_local_env.sh
```

Next, add the following lines to your `.bashrc`:
```shell
LOCAL_PREFIX="${HOME}/.local"
export PATH=${LOCAL_PREFIX}/bin:${PATH}
export LD_LIBRARY_PATH=${LOCAL_PREFIX}/lib:${LOCAL_PREFIX}/lib64:${LOCAL_PREFIX}/libexec:${LD_LIBRARY_PATH}
```

Either log out and log back in, or run `source .bashrc` to apply these changes.

Now you should be able to follow the build instructions below without modifications.

## Members

- Jake Chandler
- Daniel Petti
- Nicholas Stonecipher
