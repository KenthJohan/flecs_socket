#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *printme(void *msg) {
  while (1)
  {
	  usleep(1000);
	  printf("%s\n", (char*)msg);
  }

}

int main()
{
	pthread_t thr;
	pthread_create(&thr, NULL, printme, "hello123456789");
	pthread_create(&thr, NULL, printme, "goodbye123456789");
	int i = 0;
	while(1)
	{
		usleep(10000);
		printf("i: %d\n", i++);
	}
	pthread_join(thr, NULL);
}
