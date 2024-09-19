// Your First C++ Program

#include <chrono>
#include <unistd.h>
#include <iostream>
using namespace std;

int main() {
    printf("Starting the program... (pid:%d)\n", (int) getpid());
    int child = fork();  //parent process creates a child process
    int child2 = fork();  //parent process creates a child process
    if (child < 0) {
        // child process failed, exit the program
        fprintf(stderr, "Child process was not created");
        exit(1);
    else if(child2 == 0){
        // child2 process created successfully
        printf("Child2 Process (pid:%d): Hello world!", (int) getpid());
    }
    } else if (child == 0) {
        // child process created successfully
        printf("Child Process (pid:%d): Hello world!", (int) getpid());
    } else {
        // parent process runs this branch
        printf("Parent Process (pid:%d): Hello world!", child, (int) getpid());
    }
}