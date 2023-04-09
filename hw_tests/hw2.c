#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>



int a[2] = {0, 0};
void *add(void* rank){
int index = *(int *)rank;
 a[index] = a[index] + 1;
}
int main(int argc, char *argv[]){
 pthread_t tid;
pid_t pid;
 int rank = 0;
pid = fork();
if(!pid) rank = 1;
a[0] = a[0] + 1;
 pthread_create(&tid, NULL, add, (void*)&rank);
pthread_join(tid, NULL);
if (!pid) {
printf("%d %d\n", a[0], a[1]);
exit(0);
}
wait(NULL);
printf("%d %d\n", a[0], a[1]);
 return 0;
}