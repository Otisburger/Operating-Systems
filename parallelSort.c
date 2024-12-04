#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

//arguments for the sort function
typedef struct{
    char *file_memory;
    size_t numRecords;
} SortArgs;

//heap node structure
typedef struct {
    char *record;
    int chunkIndex;
} HeapNode;

//compares records by the first 4 bytes
int comp(const void *string, const void *string2) {
    return memcmp(string, string2, 4);
}

//compares heap nodes
int heapComp(const void *a, const void *b) {
    HeapNode *nodeA = (HeapNode *)a;
    HeapNode *nodeB = (HeapNode *)b;
    return memcmp(nodeA->record, nodeB->record, 4);
}

//sorts records
void *sort(void *arg) {
    SortArgs *args = (SortArgs *)arg;
    qsort(args->file_memory, args->numRecords, 100, comp);
    return NULL;
}
void mergeSortedChunks(char *file_memory, int numChunks, size_t recordsPerChunk[], size_t numRecords, int outputFile){
    HeapNode *heap = malloc(numChunks * sizeof(HeapNode));
    size_t *chunkOffsets = calloc(numChunks, sizeof(size_t));
    size_t totalRecords = numRecords;
    size_t heapSize = 0;
    for (int i = 0; i < numChunks; ++i) {
        if (recordsPerChunk[i] > 0) {
            heap[heapSize].record = file_memory + (i * recordsPerChunk[i] * 100);
            heap[heapSize].chunkIndex = i;
            heapSize++;
        }
    }

    //sorts the heap
    qsort(heap, heapSize, sizeof(HeapNode), heapComp);

    //merges chunks
    while (heapSize > 0) {
        HeapNode smallest = heap[0];
        write(outputFile, smallest.record, 100);
        int chunkIndex = smallest.chunkIndex;
        chunkOffsets[chunkIndex]++;
        if (chunkOffsets[chunkIndex] < recordsPerChunk[chunkIndex]) {
            smallest.record = file_memory + (chunkIndex * recordsPerChunk[chunkIndex] * 100) + (chunkOffsets[chunkIndex] * 100);
            heap[0] = smallest;
        } else {
            heap[0] = heap[heapSize - 1];
            heapSize--;
        }
        qsort(heap, heapSize, sizeof(HeapNode), heapComp);
    }
    free(heap);
    free(chunkOffsets);
}
int main(int argc, char *argv[]){
    int numProcs = get_nprocs();
    SortArgs *args[numProcs];
    pthread_t threads[numProcs];
    struct stat file_stat;
    stat(argv[1], &file_stat);
    int fileSize = file_stat.st_size;
    int numRecords = fileSize / 100;
    int recordsPerThread = numRecords / numProcs;
    int recordsRemainder = numRecords % numProcs;
    int fd = open(argv[1], O_RDWR);

    //memory maps the input file
    char *file_memory = mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    //sets the arguments for each thread
    for(int i = 0; i < numProcs; ++i){
        args[i] = malloc(sizeof(SortArgs));
        args[i]->file_memory = file_memory + (100 * recordsPerThread * i);
        if(i == numProcs-1){
            args[i]->numRecords = recordsPerThread + recordsRemainder;
        }
        else{
            args[i]->numRecords = recordsPerThread;
        }
    }

    //creates threads
    for(int i = 0; i < numProcs; ++i){
        pthread_create(&threads[i], NULL, sort, (void*)args[i]);
    }
    
    //waits for threads to finish
    for(int i = 0; i < numProcs; ++i){
        pthread_join(threads[i], NULL);
    }
    int outputFile = open(argv[2], "wb");
    size_t recordsPerChunk[numProcs];

    //sets the number of records for each chunk
    for (int i = 0; i < numProcs; ++i) {
        if(i == numProcs - 1){
            recordsPerChunk[i] = recordsPerThread + recordsRemainder;
        }
        else{
            recordsPerChunk[i] = recordsPerThread;
        }
    }
    mergeSortedChunks(file_memory, numProcs, recordsPerChunk, numRecords, outputFile);
    for(int i = 0; i < numProcs; ++i){
        free(args[i]);
    }
    fsync(outputFile);
    close(outputFile);
    munmap(file_memory, file_stat.st_size);
    close(fd);
    return 0;
}