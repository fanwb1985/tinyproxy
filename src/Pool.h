/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 1998 Steven Young <sdyoung@miranda.org>
 * Copyright (C) 1999 Robert James Kaes <rjkaes@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* See 'reqs.c' for detailed information. */

#ifndef _TINYPROXY_POOL_H_
#define _TINYPROXY_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>

/*
task struct
*/
typedef struct worker
{
    /*callback function*/
    void *(*process) (void *arg);
	/*callback function args*/
	void *arg;
	/*pointer to next task*/
    struct worker *next;

} CThread_worker;


/*pool struct*/
typedef struct
{
     pthread_mutex_t queue_lock;
     pthread_cond_t queue_ready;

    /*task head pointer*/
     CThread_worker *queue_head;

    /*flag whether to destroy pool*/
    int shutdown;
	/*thread id array*/
    pthread_t *threadid;
    /*max active thread numer*/
    int max_thread_num;
    /*current queue size*/
    int cur_queue_size;

} CThread_pool;


extern int pool_add_worker (void *(*process) (void *arg), void *arg);
extern void *thread_routine (void *arg);

extern void pool_init (int);

/*add task to pool*/
extern int
pool_add_worker (void *(*process) (void *arg), void *arg);
/*destroy pool. The waiting task will not be performed. And the running task will quit after done.*/
extern int
pool_destroy ();


extern void *
thread_routine (void *arg);

static CThread_pool *pool = NULL;



#endif

