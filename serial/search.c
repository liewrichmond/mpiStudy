#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int* generateRandomNumbers(size_t nNumbers)
{
    srand(time(NULL));
    int *arr = malloc(sizeof(int) * nNumbers);
    for (int i = 0; i < nNumbers; i++) {
        arr[i] = rand();
    }
    return arr;
}

int getTarget() {
    srand(time(NULL));
    return rand();
}

void linearSearch(int target, int* numbers, int* buffer, size_t nNumbers) {
    for (int i = 0 ; i < nNumbers; i++) {
        if(numbers[i] == target) {
            *buffer = i;
            buffer++;
        }
    }
}

int main() {
    size_t nNumbers = 100;
    int* randomNumbers = generateRandomNumbers(nNumbers);
    int target = getTarget();
    int matchingIndexes[nNumbers];
    linearSearch(target, randomNumbers, matchingIndexes, nNumbers);
}
