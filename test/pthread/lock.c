#include <pthread.h>

//pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int x = 1;

void *t1(void *arg) {
  pthread_mutex_t* lock = arg;
  pthread_mutex_lock(lock);
  x++;
  pthread_mutex_unlock(lock);
}

int main() {
  __SMACK_code("assume (forall i:int :: $pthreadStatus[i][0] == $pthread_uninitialized);"); 

  pthread_mutex_t lock;
  pthread_mutex_init(&lock, NULL);

  pthread_t tid1;

  pthread_create(&tid1, NULL, t1, &lock);
  pthread_mutex_lock(&lock);
  x++;
  pthread_mutex_unlock(&lock);
  pthread_join(tid1, NULL);
  __SMACK_assert(x == 3);
  

}