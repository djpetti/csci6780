#!/bin/bash

set -eE

# Printing colors
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Local prefix directory.
LOCAL_PREFIX="${HOME}/.local"

function on_error() {
  echo -e ${RED}Setup script failed!${NC}

  # Cleanup anything left over.
  cleanup
}
trap on_error ERR SIGINT

# Downloads and builds GCC 9.3.
function install_gcc() {
  echo -e ${GREEN}Installing GCC...${NC}

  wget http://mirrors.concertpass.com/gcc/releases/gcc-9.3.0/gcc-9.3.0.tar.xz
  tar -xvf gcc*.tar.xz
  cd gcc*
  ./configure --prefix=${LOCAL_PREFIX} --disable-multilib
  make -j$(nproc)
  make install

  # Make sure it uses this GCC by default from now on.
  export PATH=${LOCAL_PREFIX}/bin:${PATH}
  export LD_LIBRARY_PATH=${LOCAL_PREFIX}/lib:${LOCAL_PREFIX}/lib64:${LOCAL_PREFIX}/libexec:${LD_LIBRARY_PATH}

  echo -e ${GREEN}Finished installing GCC.${NC}
}

# Downloads and builds CMake.
function install_cmake() {
  echo -e ${GREEN}Installing CMake...${NC}

  wget https://github.com/Kitware/CMake/releases/download/v3.19.5/cmake-3.19.5.tar.gz
  tar -xvf cmake*.tar.gz
  cd cmake*
  ./configure --prefix=${LOCAL_PREFIX}
  gmake -j$(nproc)
  gmake install

  echo -e ${GREEN}Finished installing CMake.${NC}
}

# Downloads and builds protobuf.
function install_protobuf() {
  echo -e ${GREEN}Installing Protobuf...${NC}

  wget https://github.com/protocolbuffers/protobuf/releases/download/v3.15.0/protobuf-cpp-3.15.0.tar.gz
  tar -xvf protobuf*.tar.gz
  cd protobuf*
  ./configure --prefix=${LOCAL_PREFIX}
  make -j$(nproc)
  make install

  echo -e ${GREEN}Finished installing Protobuf.${NC}
}

# Cleans up all the downloaded files.
function cleanup() {
  echo -e ${GREEN}Cleaning up...${NC}
  rm -rf gcc* cmake* protobuf*
}

echo -e ${GREEN}Installing to local directory: ${LOCAL_PREFIX}${NC}
mkdir -p ${LOCAL_PREFIX}

install_gcc
install_cmake
install_protobuf
cleanup
echo -e ${GREEN}Done!${NC}