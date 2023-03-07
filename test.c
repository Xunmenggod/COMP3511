#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, void* argv[])
{
    pid_t pid, id;
    for (int i = 0; i < 5; i++)
    {
        id = fork();
        if (id == 0)
            break;
    }
    pid = getpid();
    if (pid == id)
        printf("true \n");
    else
        printf("false \n");
    fflush(stdout);

    return 0;
}