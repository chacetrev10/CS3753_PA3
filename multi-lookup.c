#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include "util.h"




void *readFile(void* buffer);
void *writeFile(void *buffer);


struct params{
  int front;
  int end;
  int size;
  int capacity;
  char ** buf;
  pthread_mutex_t qLock;
  pthread_mutex_t writeLock;
  pthread_mutex_t readLock;
  pthread_mutex_t threadLock;
  pthread_cond_t threadID;
  pthread_cond_t qEmpty;
  pthread_cond_t qFull;
  char ** files;
  int fileCount;
  int fileSetHead;
  int filesCompleted;
  int filesServiced[10];
  int threadNum;
  int count;
  int numberRequester;
  FILE * openFileArray[10];
  FILE *fd;
  FILE *serviced;
};




int main(){

  clock_t begin = clock();
  FILE * perf = fopen("performance.txt", "a");


  int numberResolver=11;

  char * input = malloc(40);
  char * result = malloc(1025);

  struct params *param;
  param = (struct params*)(malloc(sizeof(struct params)));
  param->fileCount= 11;
  param->front = 0;
  param->size = 0;
  param->capacity = 20;
  param->end = 0;
  param->buf = malloc(param->capacity*sizeof(char*)) ;
  param->files = malloc(40*sizeof(char*));
  param->fileSetHead = 0;
  param->filesCompleted = 0;
  param->threadNum = 0;
  param->count =0;
  param->numberRequester = 11;
  param->fd = fopen("results.txt","w");
  param->serviced = fopen("serviced.txt", "w");
  for(int i =0;i<10;i++){
    param->filesServiced[i] = 0;
  }
  for(int j = 0;j<10;j++){
    param->openFileArray[j] = NULL;
  }



  pthread_cond_init(&(param->qFull),NULL);
  pthread_cond_init(&(param->qEmpty),NULL);
  pthread_cond_init(&(param->threadID),NULL);
  pthread_mutex_init(&(param->threadLock),NULL);
  pthread_mutex_init(&(param->qLock),NULL);
  pthread_mutex_init(&(param->readLock),NULL);
  pthread_mutex_init(&(param->writeLock),NULL);

  char c = 'a';

  while(param->numberRequester >10 || param->numberRequester < 1){
    printf("How many requester threads would you like? Choose in the range 1-5\n");
    scanf(" %d", &param->numberRequester);
    if(param->numberRequester >10|| param->numberRequester < 1){
      printf("Not a valid input\n");
      do{
        c = getchar();
      }while(c != '\n');
    }
  }


  while(param->fileCount >10 || param->fileCount < 1){
    printf("How many files would you like to service? Choose in the range 1-10\n");
    scanf(" %d", &(param->fileCount));
    if(param->fileCount>10 || param->fileCount < 1){
      printf("Not a valid input\n");
      do{
        c = getchar();
      }while(c != '\n');
    }else{
      for(int i = 0; i< param->fileCount;i++){
        printf("Type file %d's file name with input/ before it\n", i);
        scanf(" %s",input);
        param->files[i] = input;
        input = malloc(40);

      }

    }

}


  while(numberResolver >10 || numberResolver< 1){
    printf("How many resolver threads would you like? Choose in the range 1-10\n");
    scanf(" %d", &numberResolver);
    if(numberResolver >10 || numberResolver < 1){
      printf("Not a valid input\n");
      do{
        c = getchar();
      }while(c != '\n');
    }
  }
  pthread_t reqThreads[param->numberRequester];
  for(int i = 0;i <param->numberRequester;i ++){
    //pthread_t* th1 = (pthread_t*) malloc(sizeof(pthread_t));
    if(pthread_create(&reqThreads[i],NULL, readFile, (void*)param)){
      perror("error creating thread");
      exit(1);
    }

    printf("Requester thread created\n");

  }
  pthread_t resThreads[numberResolver];
  for(int j = 0;j <numberResolver;j ++){
    //pthread_t* th1 = (pthread_t*) malloc(sizeof(pthread_t));
    if(pthread_create(&resThreads[j],NULL, writeFile, (void*)param)){
      perror("error creating thread");
      exit(1);
    }
    printf("Resolver thread created\n");

  }


  for(int i = 0;i <param->numberRequester;i++){

    pthread_join(reqThreads[i],NULL);
  }
  for(int j = 0;j <numberResolver;j++){


    pthread_join(resThreads[j],NULL);
  }

  pthread_cond_destroy(&(param->qFull));
  pthread_cond_destroy(&(param->qEmpty));
  pthread_cond_destroy(&(param->threadID));
  pthread_mutex_destroy(&(param->threadLock));
  pthread_mutex_destroy(&(param->qLock));
  pthread_mutex_destroy(&(param->readLock));
  pthread_mutex_destroy(&(param->writeLock));



  clock_t end = clock();
  for(int j = 0; j<param->fileCount;j++){
    free(param->files[j]);

  }
  free(input);
  free(param->files);
  free(param->buf);
  fclose(param->serviced);
  fclose(param->fd);
  double timeSpent = (double)(end-begin);
  sprintf(result ,"Num requester threads: %d\nNum resolver threads: %d\nTotal runtime: %lf\n",param->numberRequester,numberResolver, timeSpent);
  printf("%lf\n", timeSpent);
  fputs(result,perf);
  fclose(perf);
  free(result);
  free(param);
return 0;

}

void *readFile(void* buffer){
  struct params* param = (struct params*)buffer;

  char *current=malloc(50) ;
  char result[50] = "";


  while(1){

    //pthread_mutex_lock(&(param->readLock));
    pthread_mutex_lock(&(param->threadLock));
    if(param->count < param->numberRequester){
      if(param->count ==0){
        param->threadNum = (int)syscall(SYS_gettid);
      }else if((int)syscall(SYS_gettid)< param->threadNum){
        param->threadNum = (int)syscall(SYS_gettid);
      }
        param->count++;
        if(param->count != param->numberRequester) {
          pthread_cond_wait(&(param->threadID),&(param->threadLock));
        }
        pthread_cond_signal(&(param->threadID));
        pthread_mutex_unlock(&(param->threadLock));

    }
    pthread_mutex_unlock(&(param->threadLock));

    //pthread_mutex_unlock(&(param->readLock));
    pthread_mutex_lock(&(param->readLock));
    if (param->fileSetHead < param->fileCount){

      if(param->openFileArray[abs(param->threadNum-(int)syscall(SYS_gettid))] != NULL){
        fclose(param->openFileArray[abs(param->threadNum-(int)syscall(SYS_gettid))]);
      }
      param->openFileArray[abs(param->threadNum-(int)syscall(SYS_gettid))] = NULL;

      while(param->openFileArray[abs(param->threadNum-(int)syscall(SYS_gettid))] == NULL
        && param->fileSetHead < (param->fileCount) ){
        param->openFileArray[abs(param->threadNum-(int)syscall(SYS_gettid))] = fopen(param->files[param->fileSetHead], "r");
        printf("%d serviced %s \n", (int)syscall(SYS_gettid),param->files[param->fileSetHead]);
        param->fileSetHead++;
        if(param->openFileArray[abs(param->threadNum-(int)syscall(SYS_gettid))]==NULL){
          printf("error opening file %s\n", param->files[param->fileSetHead-1]);
          param->filesCompleted ++;
        }
      }
    }
    else{
      sprintf(result ,"Thread %d serviced %d files\n",(int)syscall(SYS_gettid),param->filesServiced[ abs(param->threadNum-(int)syscall(SYS_gettid))]);
      fputs(result,param->serviced);
      free(current);
      if(param->openFileArray[abs(param->threadNum-(int)syscall(SYS_gettid))] != NULL){
        fclose(param->openFileArray[abs(param->threadNum-(int)syscall(SYS_gettid))]);
      }
      pthread_mutex_unlock(&(param->readLock));
      pthread_cond_signal(&(param->qEmpty));

      return NULL;
    }

    if(param->openFileArray[abs(param->threadNum-(int)syscall(SYS_gettid))]==NULL){
      sprintf(result ,"Thread %d serviced %d files\n",(int)syscall(SYS_gettid),param->filesServiced[ abs(param->threadNum-(int)syscall(SYS_gettid))]);
      fputs(result,param->serviced);
      free(current);
      pthread_mutex_unlock(&(param->readLock));
      pthread_cond_signal(&(param->qEmpty));

      return NULL;

    }
    pthread_mutex_unlock(&(param->readLock));

    while(fscanf(param->openFileArray[abs(param->threadNum -(int)syscall(SYS_gettid))],"%s",current)!=-1){

      pthread_mutex_lock(&(param->qLock));
      //pthread_mutex_lock(&(param->qLock));



      while(param->capacity == param->size){
        pthread_cond_wait(&(param->qFull),&(param->qLock));
      }


      param->buf[param->end] = current;
      param->end = (param->end+1)%param->capacity;

      param->size++;
      pthread_mutex_lock(&(param->readLock));

      param->filesServiced[abs(param->threadNum-(int)syscall(SYS_gettid))] +=1;
      pthread_mutex_unlock(&(param->readLock));


      current = malloc(50);
      pthread_cond_signal(&(param->qEmpty));
      pthread_mutex_unlock(&(param->qLock));


  }


  pthread_mutex_lock(&(param->readLock));
  param->filesCompleted ++;
  //printf("%d\n", param->filesCompleted);
  pthread_mutex_unlock(&(param->readLock));

}
return NULL;
}


void *writeFile(void *buffer){
  struct params* param = (struct params*)buffer;


  char * ip = malloc(1025);
  char result[50];
  int res;

  //FILE *fd = fopen("results.txt","a");
  while(1){
    pthread_mutex_lock(&(param->qLock));
    while(param->size == 0 ){

      pthread_mutex_lock(&(param->readLock));

      if ( param->filesCompleted >= param->fileCount){
        pthread_mutex_unlock(&(param->readLock));
        pthread_mutex_unlock(&(param->qLock));
        free(ip);
        return NULL;
      }
        pthread_mutex_unlock(&(param->readLock));
        pthread_cond_wait(&(param->qEmpty),&(param->qLock));

    }
      //printf("%d\n",param->size);

      pthread_mutex_lock(&(param->writeLock));
      char * current = param->buf[param->front];
      param->size --;



      res = dnslookup(current,ip,100);
      param->front = (param->front+1)%param->capacity;
      pthread_cond_signal(&(param->qFull));




      if(res == 0){

        sprintf(result ,"%s,%s\n",current,ip);
      }else{
        sprintf(result ,"%s,\n",current);
        fprintf(stderr, "%s is a bogus hostname\n", current);
      }

      fputs(result,param->fd);


      pthread_mutex_unlock(&(param->writeLock));
      free(current);
    pthread_mutex_unlock(&(param->qLock));


  }


  return NULL;

}
