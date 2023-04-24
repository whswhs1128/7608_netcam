#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "work_queue.h"

int testwork(void* arg)
{
	int i = *(int*)arg;
	time_t t = time(NULL);
	printf("[%s]time for test work, i=%d\n", ctime(&t), i);
	return 0;
}

int count = 0;
int testwork1(void* arg)
{
	int i = *(int*)arg;
	time_t t = time(NULL);
	printf("[%s]time for test work, i=%d\n", ctime(&t), i);

	count++;
	printf("\ncount=%d\n", count);
	if (count == 5)
		return 0;
	else
		return -1;
}

int main(int argc, char* argv[])
{
	int i = 101, value;
	time_t t = time(NULL);
	start_default_workqueue();

	printf("[%s]\n", ctime(&t));
	//CREATE_WORK(mywork, EVENT_TIMER_WORK, testwork);
	//INIT_WORK(mywork, 5*10, &i);   //5sec

	//CREATE_WORK(mywork, TIME_TIMER_WORK, testwork);
	//INIT_WORK(mywork, 2*10, &i);     //2sec

	CREATE_WORK(mywork, CONDITION_TIMER_WORK, testwork1);
	INIT_WORK(mywork, 10, &i);     //2sec

	//CREATE_WORK(mywork, EVERY_MON_TIMER_WORK, testwork);
	//value = COMPUTE_FIXED_TIME(25, 19, 0, 0, 0);
	//value = COMPUTE_FIXED_TIME(25, 18, 10, 30, 9);
	//INIT_WORK(mywork, value, &i);

	//CREATE_WORK(mywork, EVERY_WEEK_TIMER_WORK, testwork);
	//value = COMPUTE_FIXED_TIME(5, 19, 0, 0, 0);
	//value = COMPUTE_FIXED_TIME(5, 18, 10, 30, 9);
	//value = COMPUTE_FIXED_TIME(2, 18, 10, 30, 9);
	//value = COMPUTE_FIXED_TIME(6, 18, 10, 30, 9);
	//INIT_WORK(mywork, value, &i);
	
	//CREATE_WORK(mywork, EVERY_DAY_TIMER_WORK, testwork);
	//value = COMPUTE_FIXED_TIME(0, 0, 0, 0, 0);
	//value = COMPUTE_FIXED_TIME(0, 20, 10, 30, 9);
	//INIT_WORK(mywork, value, &i);

	//CREATE_WORK(mywork, EVERY_HOUR_TIMER_WORK, testwork);
	//value = COMPUTE_FIXED_TIME(0, 0, 56, 55, 0);
	//value = COMPUTE_FIXED_TIME(0, 0, 30, 30, 9);
	//INIT_WORK(mywork, value, &i);

	//CREATE_WORK(mywork, EVERY_MIN_TIMER_WORK, testwork);
	//value = COMPUTE_FIXED_TIME(0, 0, 0, 55, 0);
	//value = COMPUTE_FIXED_TIME(0, 0, 0, 30, 9);
	//INIT_WORK(mywork, value, &i);

	SCHEDULE_DEFAULT_WORK(mywork);
	
	while (1) sleep(1);
  return 0;
}
