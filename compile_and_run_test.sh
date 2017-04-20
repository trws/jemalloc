#!/bin/bash
./autogen.sh
./configure CPPFLAGS='-DUSE_CUDA_UM -I/opt/cudatoolkit-8.0/include/ ' LDFLAGS='-L /opt/cudatoolkit-8.0/lib64/ -lcudart ' --disable-coa
lesce --with-jemalloc-prefix=je

# build malloc wrapper
gcc -L $(pwd)/lib wrap_malloc.c -shared -o lib/libwrapmalloc.so -fPIC -l jemalloc

# compile cudatest: NOTE! --cudart shared is *required* so that the cuda
# libraries call the normal system malloc rather than ours
nvcc -Xlinker --wrap=malloc -Xlinker --wrap=calloc -Xlinker --wrap=realloc -Xlinker --wrap=free -Xlinker \
     -rpath,$(pwd)/lib -L $(pwd)/lib cudatest.cu -lwrapmalloc --cudart shared -o cudatest

./cudatest
