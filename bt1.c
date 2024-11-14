
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void handle_sigint(int sig) {
    printf("\ncount.sh has stopped! Goodbye!\n");
    exit(0);
}

int main() {

    printf("Welcome to IT007, I am 23521475!\n");


    signal(SIGINT, handle_sigint);


    pid_t pid = fork();
    if (pid == 0) {

        execl("./count.sh", "./count.sh", "120", NULL);
    } else if (pid > 0) {

        wait(NULL);
    } else {
        perror("fork failed");
        return 1;
    }

    return 0;
}
