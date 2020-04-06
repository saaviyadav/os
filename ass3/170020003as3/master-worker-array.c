#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

int total_items,num_workers,max_buf_size;
int curr_buf_size;
int *buf;
int item_to_produce;
int item_to_consume;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buf_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t buf_full = PTHREAD_COND_INITIALIZER;
// declare any global data structures, variables, etc that are required
// e.g buffer to store items, pthread variables, etc

void print_produced(int num) {
  printf("Produced %d\n", num);
}

void print_consumed(int num, int worker) {
  printf("Consumed %d by worker %d\n", num, worker);
}


/* produce items and place in buffer (array or linked list)
 * add more code here to add items to the buffer (these items will be consumed
 * by worker threads)
 * use locks and condvars suitably
 */
void *generate_requests_loop(void *data) {
  int thread_id = *((int *)data);

  while(1) { 
    pthread_mutex_lock(&lock);
    if(item_to_produce >= total_items){
      pthread_cond_broadcast(&buf_empty);
      pthread_mutex_unlock(&lock);
      break;}
    while(max_buf_size <= curr_buf_size) pthread_cond_wait(&buf_full, &lock);
    print_produced(item_to_produce);
    buf[curr_buf_size] = item_to_produce;
    item_to_produce++;
    curr_buf_size++;
    //printf("%d\n",curr_buf_size);
    pthread_cond_broadcast(&buf_empty);
    //printf("Producer : Signal not empty sent \n");
    pthread_mutex_unlock(&lock); 
  }
  return 0;
}

//write function to be run by worker threads
//ensure that the workers call the function print_consumed when they consume an item
void *worker_consumed(void *data){
  int thread_id = *((int *)data);
  while(1) {
    //printf("COnsumer : Before Mutex %d \n",thread_id);
    pthread_mutex_lock(&lock);
    //printf("COnsumer : After Mutex %d \n",thread_id);
    while(item_to_consume >= total_items){
      //printf("p:%d\n",item_to_consume);
      pthread_cond_broadcast(&buf_empty);
      pthread_mutex_unlock(&lock);
      //printf("COnsumer : After Mutex %d \n",thread_id);
      return data;
    }
    //printf("entering\n");
    while(curr_buf_size <= 0) {
      //printf("COnsumer : Went to sleep %d \n",thread_id);
      pthread_cond_wait(&buf_empty, &lock);
      if(item_to_consume >= total_items){
      //printf("p:%d\n%d\n",item_to_consume,curr_buf_size);
      pthread_mutex_unlock(&lock);
      //printf("COnsumer : After Mutex %d \n",thread_id);
      return data;
    }
      //printf("COnsumer : Woke up %d \n",thread_id);
    }

    curr_buf_size--;
    print_consumed(buf[curr_buf_size],thread_id);
    //printf("%d\n",item_to_consume);
    buf[curr_buf_size] = 0;
    pthread_cond_broadcast(&buf_full);
    item_to_consume++;
    pthread_mutex_unlock(&lock); 
  }
}

int main(int argc, char *argv[])
{
 
  int master_thread_id = 0;
  pthread_t master_thread;
  item_to_produce = 0;
  item_to_consume = 0;
  
  if (argc < 4) {
    printf("./master-worker #total_items #max_buf_size #num_workers e.g. ./exe 10000 1000 4\n");
    exit(1);
  }
  else {
    num_workers = atoi(argv[3]);
    total_items = atoi(argv[1]);
    max_buf_size = atoi(argv[2]);
  }
   
  // Initlization code for any data structures, variables, etc
  pthread_t worker_thread[num_workers];
  curr_buf_size = 0;
  item_to_produce = 0;
  buf = (int*) malloc(max_buf_size * sizeof(int));
  //create master producer thread
  pthread_create(&master_thread, NULL, generate_requests_loop, (void *)&master_thread_id);
  int worker_thread_id[num_workers];
  //create worker consumer threads
  for(int i=0; i<num_workers; i++){
    worker_thread_id[i] = i+1;
    pthread_create(&worker_thread[i], NULL, worker_consumed, (void *)&(worker_thread_id[i]));
  } 


  //wait for all threads to complete
  pthread_join(master_thread, NULL);
  printf("master joined\n");
  for(int i=0; i<num_workers; i++)  pthread_join(worker_thread[i], NULL);
  //printf("worker joined\n");

  //deallocate and free up any memory you allocated
  
  return 0;
}