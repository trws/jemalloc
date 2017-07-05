#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
__global__ void add_one(int *a) {
    *a += 1;
}

void test(int * a) {
    add_one<<<1,1>>>(a);
    cudaDeviceSynchronize();

    printf("%d\n", *a);
}

int main(int argc, char *argv[])
{
    int *a2 = (int*)calloc(1, sizeof(int));
    test(a2);

    if (*a2 == 1)
        printf("success!\n");
    else
        printf("failure!\n");



    return 0;
}
