#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void PrintRecord(const unsigned char *record, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%02x", record[i]);
    }
}

int main(int argc, char *argv[]) {
    FILE* file = fopen(argv[1], "rb");
    size_t recordSize = 100;
    size_t numRecords = 80;
    unsigned char record[recordSize];

    //prints each record
    for (int i = 0; i < numRecords; i++) {
        fread(record, 1, recordSize, file);
        printf("Record %d: ", i+1);
        PrintRecord(record, recordSize);
        printf("\n");
        printf("\n");
    }
    fclose(file);
    return 0;
}
