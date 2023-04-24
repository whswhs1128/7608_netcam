// ***************************************************************
//  csl_sem   version:  1.0   ? date: 08/04/2010
//  -------------------------------------------------------------
//	定义信号量类CSemaphore、互斥类CMutex
//  Author:Itman Lee
//  -------------------------------------------------------------
//  Copyright (C) 2010 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************
#if !defined(__CSL_SEM_H__)
#define __CSL_SEM_H__
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

class CMutex {
private:
	pthread_mutex_t m_Mutex;
public:
	inline CMutex( ) { pthread_mutex_init(&m_Mutex, NULL); }
	inline ~CMutex( ) { pthread_mutex_destroy(&m_Mutex); }
	inline void Lock( ) { pthread_mutex_lock(&m_Mutex); }
	inline void Unlock( ) { pthread_mutex_unlock(&m_Mutex); }
};

class CSemaphore {
private:
	sem_t m_Semaphore;
public:
	inline CSemaphore(unsigned count = 1) {sem_init(&m_Semaphore, 0, count); }
	inline ~CSemaphore( ) {sem_destroy(&m_Semaphore); }
	inline void Signal( ) {sem_post(&m_Semaphore); }
	inline bool Wait(unsigned timeout) {
		// Apologies to Posix enthusiasts, but this is ridiculous!
		struct timespec when;
		clock_gettime(CLOCK_REALTIME, &when);
		when.tv_nsec += timeout*1000000;
		if (when.tv_nsec >= 1000000000) {
			when.tv_nsec -= 1000000000;
			when.tv_sec++;
		}
		return sem_timedwait(&m_Semaphore, &when) == 0;
	}
	inline void Wait() {
		// Apologies to Posix enthusiasts, but this is ridiculous!
		
		 sem_wait(&m_Semaphore);
	}
};

class CMutexedSection {
private:
	CMutex & m_Mutex;
public:
	CMutexedSection(CMutex & mutex) : m_Mutex(mutex) { mutex.Lock();}	
	~CMutexedSection( ){ m_Mutex.Unlock( ); }
};

#endif
