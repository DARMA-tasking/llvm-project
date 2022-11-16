FROM ubuntu:20.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y -q && apt-get install -y -q --no-install-recommends \
    build-essential \
    cmake \
    git \
    ninja-build \
    python3 \
    lld

COPY . /llvm-project

RUN mkdir llvm-build
WORKDIR /llvm-build
RUN cmake \
    -DCLANG_TIDY_ENABLE_STATIC_ANALYZER=ON \
    "-DLLVM_ENABLE_PROJECTS=clang;clang-tools-extra" \
    -DLLVM_TARGETS_TO_BUILD=X86 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/llvm-install \
    -DLLVM_USE_LINKER=lld \
    -GNinja \
    /llvm-project/llvm
RUN cmake --build . --target install

FROM builder AS installer
WORKDIR /
COPY --from=builder /llvm-install/ /usr/local/
