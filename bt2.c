
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <positive integer>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        printf("Please enter a positive integer.\n");
        return 1;
    }


    int *collatz_seq = mmap(NULL, 100 * sizeof(int), PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (collatz_seq == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {  
        int index = 0;
        while (n != 1) {
            collatz_seq[index++] = n;
            n = (n % 2 == 0) ? n / 2 : 3 * n + 1;
        }
        collatz_seq[index] = 1;  
        exit(0);
    } else if (pid > 0) {  
        wait(NULL); 
        printf("Collatz sequence: ");
        for (int i = 0; collatz_seq[i] != 0; i++) {
            printf("%d ", collatz_seq[i]);
        }
        printf("\n");
        munmap(collatz_seq, 100 * sizeof(int));
    } else {
        perror("fork failed");
        return 1;
    }

    return 0;
}
