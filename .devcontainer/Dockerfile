FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Update + essential build tools
RUN apt-get update && apt-get install -y \
    file \
    build-essential \
    cmake \
    ninja-build \
    gdb \
    git \
    pkg-config \
    clang \
    clang-format \
    clang-tidy \
    python3 \
    python3-pip \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    libboost-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN printf "\
set(CMAKE_SYSTEM_NAME Linux)\n\
set(CMAKE_SYSTEM_PROCESSOR aarch64)\n\
\n\
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)\n\
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)\n\
" > /aarch64-toolchain.cmake

# Create user that VS Code uses
RUN useradd -m -s /bin/bash vscode && \
    echo "vscode ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

RUN mkdir -p /workspaces && chown vscode:vscode /workspaces

USER vscode
WORKDIR /workspaces/softa
