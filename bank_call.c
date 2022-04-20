#include <t_stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

typedef int que_t[6];
que_t q;
sem_t p;
sem_t c;

void *p_func(void *arg){
	int t = 0;
	while(1){
		sem_wait(&p);
		q[t] = rand() % 1000 + 1;
		printf("p:%d\n", q[t]);
		t = (t + 1) % 6;
		sem_post(&c);
		sleep(rand()%4+1);
	}
	return NULL;
}

void *c_func(void *arg){
	int h = 0;
	int tmp;
	while(1){
		sem_wait(&c);
		tmp = q[h];
		q[h] = -1;
		h = (h+1)%6;
		sem_post(&p);
		printf("c:%d\n", tmp);
		sleep(rand()%4+1);
	}
	return 0;
}

int main(void){
	sem_init(&p, 0, 6);
	sem_init(&c, 0, 0);
	srand(time(NULL));
		pthread_t tid, cid;
	pthread_create(&tid, NULL, p_func, NULL);
	pthread_create(&cid, NULL, c_func, NULL);
	pthread_join(tid, NULL);
	pthread_join(cid, NULL);
	sem_destroy(&p);
	sem_destroy(&c);
	return 0;
}
