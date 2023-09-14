
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef SDK_SYS_H_
#define SDK_SYS_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "sdk_def.h"

/*!
*******************************************************************************
** \brief thread function prototype
*******************************************************************************
*/
typedef void (*SDK_ThreadFunctionT)(void *);

/*!
*******************************************************************************
** \brief init system module.
**
**
** \param[in] streamNum  the max encoding stream number
**
**
** \return SDK_OK  success
**         other   failed
**
** \sa sdk_sys_init
**
*******************************************************************************
*/
int sdk_sys_init(int streamNum);

/*!
*******************************************************************************
** \brief exit system module
**
** \return SDK_OK  success
**         other   failed
**
** \sa sdk_sys_destroy
**
*******************************************************************************
*/
int sdk_sys_destroy();

/*!
*******************************************************************************
** \brief Create an semphore with the given initial value \e initValue.
**
**
** \param[in] initValue Specifies the initial value for the semaphore.
**
**
** \return Return an valid handle value if the semaphore creating is successful.
**         Return 0 if creating failed.
**
** \sa sdk_sys_sem_destroy
**
*******************************************************************************
*/
SDK_SemHandleT sdk_sys_sem_create(SDK_U32 initValue);

/*!
*******************************************************************************
** \brief Increments(unlocks) the semaphore referred by \e sem.
**
**
** \param[in] sem   Valid semaphore handle created previously by
**                  #sdk_sys_sem_create.
**
**
** \return
** - #SDK_OK   Successful completion.
**
** \sa sdk_sys_sem_wait
** \sa sdk_sys_sem_wait_timeout
**
*******************************************************************************
*/
SDK_ERR sdk_sys_sem_post(SDK_SemHandleT sem);

/*!
*******************************************************************************
** \brief Dncrements(locks) the semaphore referred by \e sem.
**
**
** \param[in] sem   Valid semaphore handle created previously by
**                  #sdk_sys_sem_create.
**
**
** \return
** - #SDK_OK   Successful completion.
**
** \sa
**  - sdk_sys_sem_post
**  - sdk_sys_sem_wait_timeout
**
*******************************************************************************
*/
SDK_ERR sdk_sys_sem_wait(SDK_SemHandleT sem);


/*!
*******************************************************************************
** \brief This API is the same as sdk_sys_sem_wait, except that \e msecs
**        specifies a limitation on the amount of the time that the call should
**        block if the decrement can not be performed immediately.
**
**
** \param[in] sem   Valid semaphore handle created previously by
**                  #sdk_sys_sem_create.
** \param[in] msecs Timeout value in millisecond.
**
**
** \return
** - #SDK_OK   Successful completion.
**
** \sa
**      - sdk_sys_sem_post
**      - sdk_sys_sem_wait
**
*******************************************************************************
*/
SDK_ERR sdk_sys_sem_wait_timeout(SDK_SemHandleT sem, SDK_U32 msecs);


/*!
*******************************************************************************
** \brief Destroy the semaphore \e sem.
**
**
** \param[in] sem   Valid semaphore handle created previously by
**                  #sdk_sys_sem_create.
**
**
** \return
** - #SDK_OK   Successful completion.
**
** \sa sdk_sys_sem_create
**
*******************************************************************************
*/
SDK_ERR sdk_sys_sem_destroy(SDK_SemHandleT sem);



/*!
*******************************************************************************
** \brief Allocates \e size bytes and returns the pointer to the allocated memory.
**
** \param[in]  size   Memory size in bytes.
**
** \return In success, return the valid memory pointer.
**         In failure, return NULL.
**
** \sa sdk_sys_free
**
*******************************************************************************
*/
void *sdk_sys_malloc(SDK_U32 size);


void sdk_sys_free(void *ptr);


/*!
*******************************************************************************
** \brief This API copies \e size bytes from source memory \e src to the
**        destnation memory \e dest.
**
** \note  The \e dest and \e src should not overlap.
**
**
** \param[in]  dest  Destnation memory.
** \param[in]  src   Source memory.
** \param[in]  size  Size in bytes.
**
** \return
**      - #NULL memory copy fail.
**      - >0    a pointer to the destination address.
**
** \sa sdk_sys_memset
**
*******************************************************************************
*/
void* sdk_sys_memcpy(void *dest, void *src, SDK_U32 size);

/*!
*******************************************************************************
** \brief This API fills the first \e n bytes of the memory area pointed to by
**        the pointer of \e ptr, with the constant byte \e c.
**
**
** \param[in]  ptr  Pointer to the memory area to be filled.
** \param[in]  c    Constant byte value used to fill the specified memory.
** \param[in]  n    Number of bytes to fill.
**
** \return
**      - #NULL memory set fail.
**      - >0    a pointer to the destination address.
**
** \sa sdk_sys_memcpy
**
*******************************************************************************
*/
void* sdk_sys_memset(void *ptr, SDK_U8 c, SDK_U32 n);

/*!
*******************************************************************************
** \brief Starts a new thread in the call process.
**
**
** \param[in]  func     The routine executed in the created thread.
** \param[in]  arg      The argument passed to the \e func.
** \param[in]  priority Priority setting.
** \param[in]  ssize    Stack size in bytes.
**                      In RTOS environment, this parameter is ignored.
** \param[in]  name     Thread name.
** \param[out] thread   Thread identifier or handle.
**
** \return
** - #SDK_OK
**
** \sa
**      - sdk_sys_thread_destroy
**
*******************************************************************************
*/
SDK_ERR sdk_sys_thread_create(SDK_ThreadFunctionT func,
                            void                   *arg,
                            SDK_S32                priority,
                            SDK_S32                ssize,
                            const char             *name,
                            SDK_ThreadHandleT *thread);


/*!
*******************************************************************************
** \brief Stop and destroy the given thread.
**
**
** \param[in]  thread The identifier of the thread to be destroyed.
**
** \return
** - #SDK_OK
**
** \sa
**      - sdk_sys_thread_create
**
*******************************************************************************
*/
SDK_ERR sdk_sys_thread_destroy(SDK_ThreadHandleT thread);


/*!
*******************************************************************************
** \brief wait thread auto exit and destroy the given thread.
**
**
** \param[in]  thread The identifier of the thread to be destroyed.
**
** \return
** - #SDK_OK
**
** \sa
**      - sdk_sys_thread_create
**
*******************************************************************************
*/
SDK_ERR sdk_sys_thread_join(SDK_ThreadHandleT thread);
/*!
*******************************************************************************
** \brief Stop and destroy the calling thread.
**
**
** \sa
**      - sdk_sys_thread_destroy
**
*******************************************************************************
*/
void sdk_sys_thread_self_destroy(void);

/*!
*******************************************************************************
** \brief Makes the call thread to sleep until the given milliseconds have passed.
**
**
** \param[in] msecs Sleep time in milliseconds.
**
** \return
**      - #SDK_OK
**
*******************************************************************************
*/
void sdk_sys_thread_sleep(SDK_U32 msecs);

void sdk_sys_thread_set_name(char *name);


SDK_MutexHandleT sdk_sys_mutex_create(void);

SDK_ERR sdk_sys_mutex_lock(SDK_MutexHandleT mutex);

SDK_ERR sdk_sys_mutex_lock_timeout(SDK_MutexHandleT mutex, SDK_U32 msecs);

SDK_ERR sdk_sys_mutex_unlock(SDK_MutexHandleT mutex);

SDK_ERR sdk_sys_mutex_destroy(SDK_MutexHandleT mutex);


SDK_ConHandleT sdk_sys_cond_create(void);

SDK_ERR sdk_sys_cond_wait(SDK_ConHandleT cond, SDK_MutexHandleT mutex);

SDK_ERR sdk_sys_cond_signal(SDK_ConHandleT cond);

SDK_ERR sdk_sys_cond_timedwait(SDK_ConHandleT cond, SDK_MutexHandleT mutex, const SDK_U32 msecs); //ms

SDK_ERR sdk_sys_cond_broadcast(SDK_ConHandleT cond);

SDK_ERR sdk_sys_cond_destroy(SDK_ConHandleT cond);




#ifdef __cplusplus
};
#endif
#endif //SDK_H_

