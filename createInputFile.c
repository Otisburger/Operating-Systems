#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(){
    FILE *file = fopen("input.bin", "wb");
    char record[100];
    srand(time(NULL));
    for (int i = 0; i < 80; ++i){
        for (int j = 0; j < 100; ++j){
            record[j] = rand() % 256;
        }
        fwrite(record, sizeof(char), 100, file);
    }
}