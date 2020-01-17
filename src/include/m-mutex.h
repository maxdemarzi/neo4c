/*
 * M*LIB - Thin Mutex & Thread wrapper
 *
 * Copyright (c) 2017-2019, Patrick Pelissier
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * + Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * + Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MSTARLIB_MUTEX_H
#define MSTARLIB_MUTEX_H

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L    \
  && !defined(__STDC_NO_THREADS__)

/****************************** C11 version ********************************/

#include <threads.h>
#include <assert.h>
#include <stdbool.h>
#include "m-core.h"

typedef mtx_t                  m_mutex_t[1];
typedef cnd_t                  m_cond_t[1];
typedef thrd_t                 m_thread_t[1];

static inline void m_mutex_init(m_mutex_t m)
{
  int rc = mtx_init(m, mtx_plain);
  M_ASSERT_INIT (rc == 0, "mutex");
}

static inline void m_mutex_clear(m_mutex_t m)
{
  mtx_destroy(m);
}

static inline void m_mutex_lock(m_mutex_t m)
{
  mtx_lock(m);
}

static inline void m_mutex_unlock(m_mutex_t m)
{
  mtx_unlock(m);
}

static inline void m_cond_init(m_cond_t c)
{
  int rc = cnd_init(c);
  M_ASSERT_INIT (rc == 0, "conditional variable");
}

static inline void m_cond_clear(m_cond_t c)
{
  cnd_destroy(c);
}

static inline void m_cond_signal(m_cond_t c)
{
  cnd_signal(c);
}

static inline void m_cond_broadcast(m_cond_t c)
{
  cnd_broadcast(c);
}

static inline void m_cond_wait(m_cond_t c, m_mutex_t m)
{
  cnd_wait(c, m);
}

static inline void m_thread_create(m_thread_t t, void (*func)(void*), void* arg)
{
  int rc = thrd_create(t, (int(*)(void*))(void(*)(void))func, arg);
  M_ASSERT_INIT (rc == thrd_success, "thread");
}

static inline void m_thread_join(m_thread_t t)
{
  int rc = thrd_join(*t, NULL);
  assert (rc == thrd_success);
  // Avoid warning about variable unused.
  (void) rc;
}

static inline void m_thread_yield(void)
{
  thrd_yield();
}

static inline bool m_thread_sleep(unsigned long long usec)
{
  struct timespec tv;
  tv.tv_sec = usec / 1000000ULL;
  tv.tv_nsec = (usec % 1000000ULL) * 1000UL;
  int retval = thrd_sleep(&tv, NULL);
  return retval == 0;
}

// Internal type, not exported.
typedef once_flag                     m_oncei_t[1];
#define M_ONCEI_INIT_VALUE            { ONCE_FLAG_INIT }
static inline void m_oncei_call(m_oncei_t o, void (*func)(void))
{
  call_once(o,func);
}

#define M_THREAD_ATTR _Thread_local


// MSYS2 doesn't define _WIN32 by default.
#elif defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)

/****************************** WIN32 version ******************************/

/* CriticalSection & ConditionVariable are available from Windows Vista */
#ifndef WINVER
#define WINVER        _WIN32_WINNT_VISTA
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT  _WIN32_WINNT_VISTA
#endif
#include <windows.h>
#include <assert.h>
#include <stdbool.h>
#include "m-core.h"

typedef HANDLE                 m_thread_t[1];
typedef CRITICAL_SECTION       m_mutex_t[1];
typedef CONDITION_VARIABLE     m_cond_t[1];

static inline void m_mutex_init(m_mutex_t m)
{
  InitializeCriticalSection(m);
}

static inline void m_mutex_clear(m_mutex_t m)
{
  DeleteCriticalSection(m);
}

static inline void m_mutex_lock(m_mutex_t m)
{
  EnterCriticalSection(m);
}

static inline void m_mutex_unlock(m_mutex_t m)
{
  LeaveCriticalSection(m);
}

static inline void m_cond_init(m_cond_t c)
{
  InitializeConditionVariable(c);
}

static inline void m_cond_clear(m_cond_t c)
{
  (void) c; // There is no destructor for this object.
}

static inline void m_cond_signal(m_cond_t c)
{
  WakeConditionVariable(c);
}

static inline void m_cond_broadcast(m_cond_t c)
{
  WakeAllConditionVariable(c);
}

static inline void m_cond_wait(m_cond_t c, m_mutex_t m)
{
  SleepConditionVariableCS(c, m, INFINITE);
}

static inline void m_thread_create(m_thread_t t, void (*func)(void*), void *arg)
{
  *t = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) func, arg, 0, NULL);
  M_ASSERT_INIT (*t != NULL, "thread");
}

static inline void m_thread_join(m_thread_t t)
{
  DWORD dwWaitResult = WaitForSingleObject(*t, INFINITE);
  (void) dwWaitResult;
  assert (dwWaitResult == WAIT_OBJECT_0);
  CloseHandle(*t);
}

static inline void m_thread_yield(void)
{
  Sleep(0);
}

static inline bool m_thread_sleep(unsigned long long usec)
{
  LARGE_INTEGER ft;
  ft.QuadPart = -(10ULL*usec);
  HANDLE hd = CreateWaitableTimer(NULL, TRUE, NULL);
  M_ASSERT_INIT (hd != NULL, "timer");
  SetWaitableTimer(hd, &ft, 0, NULL, NULL, 0);
  DWORD dwWaitResult = WaitForSingleObject(hd, INFINITE);
  CloseHandle(hd);
  return dwWaitResult == WAIT_OBJECT_0;
}


// Internal type, not exported.
typedef INIT_ONCE                     m_oncei_t[1];
#define M_ONCEI_INIT_VALUE            { INIT_ONCE_STATIC_INIT }
static inline BOOL CALLBACK m_oncei_callback( PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext)
{
    void (*func)(void);
    (void) InitOnce;
    (void) lpContext;
    func = (void (*)(void))(uintptr_t) Parameter;
    (*func)();
    return TRUE;
}
static inline void m_oncei_call(m_oncei_t o, void (*func)(void))
{
  InitOnceExecuteOnce(o, m_oncei_callback, (void*)(intptr_t)func, NULL);
}

#if defined(_MSC_VER)
# define M_THREAD_ATTR __declspec( thread )
#else
# define M_THREAD_ATTR __thread
#endif

#else

/**************************** PTHREAD version ******************************/

#include <pthread.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
#include <sched.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "m-core.h"

typedef pthread_mutex_t        m_mutex_t[1];
typedef pthread_cond_t         m_cond_t[1];
typedef pthread_t              m_thread_t[1];

static inline void m_mutex_init(m_mutex_t m)
{
  int _rc = pthread_mutex_init(m, NULL);
  M_ASSERT_INIT (_rc == 0, "mutex");
}

static inline void m_mutex_clear(m_mutex_t m)
{
  pthread_mutex_destroy(m);
}

static inline void m_mutex_lock(m_mutex_t m)
{
  pthread_mutex_lock(m);
}

static inline void m_mutex_unlock(m_mutex_t m)
{
  pthread_mutex_unlock(m);
}

#define M_MUTEXI_INIT_VALUE    { PTHREAD_MUTEX_INITIALIZER }
static inline void m_mutexi_lazy_lock(m_mutex_t m)
{
  pthread_mutex_lock(m);
}

static inline void m_cond_init(m_cond_t c)
{
  int _rc = pthread_cond_init(c, NULL);
  M_ASSERT_INIT (_rc == 0, "conditional variable");
}

static inline void m_cond_clear(m_cond_t c)
{
  pthread_cond_destroy(c);
}

static inline void m_cond_signal(m_cond_t c)
{
  pthread_cond_signal(c);
}

static inline void m_cond_broadcast(m_cond_t c)
{
  pthread_cond_broadcast(c);
}

static inline void m_cond_wait(m_cond_t c, m_mutex_t m)
{
  pthread_cond_wait(c, m);
}

static inline void m_thread_create(m_thread_t t, void (*func)(void*), void *arg)
{
  int _rc = pthread_create(t, NULL, (void*(*)(void*))(void(*)(void))func, arg);
  M_ASSERT_INIT (_rc == 0, "thread");
}

static inline void m_thread_join(m_thread_t t)
{
  int _rc = pthread_join(*t, NULL);
  (void)_rc; // Avoid warning about variable unused.
  assert (_rc == 0);
}

static inline void m_thread_yield(void)
{
#ifdef _POSIX_PRIORITY_SCHEDULING
  sched_yield();
#endif
}

static inline bool m_thread_sleep(unsigned long long usec)
{
  struct timeval tv;
  /* We don't want to use usleep or nanosleep so that
     we remain compatible with strict C99 build */
  tv.tv_sec = usec / 1000000ULL;
  tv.tv_usec = usec % 1000000ULL;
  int retval = select(1, NULL, NULL, NULL, &tv);
  return retval == 0;
}

// Internal type, not exported.
typedef pthread_once_t                m_oncei_t[1];
#define M_ONCEI_INIT_VALUE            { PTHREAD_ONCE_INIT }
static inline void m_oncei_call(m_oncei_t o, void (*func)(void))
{
  pthread_once(o,func);
}

#define M_THREAD_ATTR __thread

#endif

// TODO: m_thread_sleep doesn't yield thread during waiting, making it a poor choice for active waiting.
// TODO: Obsolete M_LOCK macro.

/* M_LOCK macro. Allow simple locking encapsulation.
   USAGE:
    static M_LOCK_DECL(name);
    int f(int n) {
      M_LOCK(name) {
        // Exclusive access
      }
    }
*/
/* NOTE: Either using direct support by the OS (WIN32/PTHREAD)
   or using C11's ONCE mechanism */
#ifdef M_MUTEXI_INIT_VALUE
# define M_LOCK_DECL(name) m_mutex_t name = M_MUTEXI_INIT_VALUE
# define M_LOCK(name)                                                   \
  M_LOCKI_DO(name, M_C(local_cont_, __LINE__), m_mutexi_lazy_lock, m_mutex_unlock)
#else
# define M_LOCK_DECL(name)                                      \
  m_mutex_t name;                                               \
  static void M_C(m_mutex_init_, name)(void) {                  \
    m_mutex_init(name);                                         \
  }                                                             \
  m_oncei_t M_C(m_once_, name) = M_ONCEI_INIT_VALUE
# define M_LOCKI_BY_ONCE(name)                                          \
  (m_oncei_call(M_C(m_once_, name), M_C(m_mutex_init_, name)),          \
   m_mutex_lock(name), (void) 0 )
# define M_LOCK(name)                                                   \
  M_LOCKI_DO(name, M_C(local_cont_, __LINE__), M_LOCKI_BY_ONCE, m_mutex_unlock)
#endif

#define M_LOCKI_DO(name, cont, lock_func, unlock_func)                \
  for(bool cont = true                                                \
        ; cont && (lock_func (name), true);                           \
      (unlock_func (name), cont = false))

#endif
