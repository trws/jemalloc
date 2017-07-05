#!/bin/bash
./autogen.sh
./configure --enable-debug --with-jemalloc-prefix=je --disable-coalesce --prefix=$(pwd)/install \
    CPPFLAGS='-DUSE_CUDA_UM -I/usr/local/cuda-8.0/include/'  \
    LDFLAGS=" -L/usr/local/cuda/lib64 -lcudart "  
# ./configure CPPFLAGS='-DUSE_CUDA_UM -I/opt/cudatoolkit-8.0/include/ ' LDFLAGS='-L /opt/cudatoolkit-8.0/lib64/ -lcudart ' --disable-coalesce --with-jemalloc-prefix=je

make -j
make install
# build malloc wrapper
nvcc wrap_malloc.c -shared -o libwrapmalloc.so -g -std=c++11 -cudart none

# compile cudatest: NOTE! --cudart shared is *required* so that the cuda
# libraries call the normal system malloc rather than ours
nvcc -Xlinker -rpath,$(pwd)/install/lib,-rpath,$(pwd) -L $(pwd)/install/lib -L $(pwd) cudatest.cu -lwrapmalloc --cudart shared -o cudatest -ljemalloc

./cudatest
