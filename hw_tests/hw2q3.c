#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int a[10] = {1,5,3,9,2,6,8,4,9,0};
int target = 9;
int answer;
void *func(void* value){
 int rank = *(int *)value;
 for (int i = rank * 5; i < (rank + 1) * 5; i++){
 if (a[i] == target){
 answer = i;
 }
 }
 pthread_exit(0);
}
int main(int argc,char *argv[]){
 int i;
 int rank[2] = {0, 1};
 pthread_t tid[2];
 for (i = 0; i < 2; i++){
 pthread_create(&tid[i], NULL, func, (void*)&rank[i]);
 }
 for (i = 0; i < 2; i++){
 pthread_join(tid[i], NULL);
 }
 printf("%d\n", answer);
 return 0;
}