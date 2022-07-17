set -e

cmake \
    -B build \
    -S . \
    -G Xcode \
    -DCMAKE_TOOLCHAIN_FILE=ios.toolchain.cmake \
    -DPLATFORM=OS
    # -DDEPLOYMENT_TARGET=9.0 \
    # -DENABLE_ARC=FALSE \

