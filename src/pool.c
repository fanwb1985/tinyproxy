#include "Pool.h"

void
pool_init (int max_thread_num)
{
     pool = (CThread_pool *) malloc (sizeof (CThread_pool));

     pthread_mutex_init (&(pool->queue_lock), NULL);
     pthread_cond_init (&(pool->queue_ready), NULL);

     pool->queue_head = NULL;

     pool->max_thread_num = max_thread_num;
     pool->cur_queue_size = 0;

     pool->shutdown = 0;

     pool->threadid =
         (pthread_t *) malloc (max_thread_num * sizeof (pthread_t));
     int i = 0;
     for (i = 0; i < max_thread_num; i++)
     {
         pthread_create (&(pool->threadid[i]), NULL, thread_routine,
                 NULL);
     }
}


/*add task to pool*/
int
pool_add_worker (void *(*process) (void *arg), void *arg)
{
     /*new task*/
     CThread_worker *newworker =
         (CThread_worker *) malloc (sizeof (CThread_worker));
     newworker->process = process;
     newworker->arg = arg;
     newworker->next = NULL;

     pthread_mutex_lock (&(pool->queue_lock));
    /*add task to queue*/
     CThread_worker *member = pool->queue_head;
    if (member != NULL)
     {
        while (member->next != NULL)
             member = member->next;
         member->next = newworker;
     }
    else
     {
         pool->queue_head = newworker;
     }

     assert (pool->queue_head != NULL);

     pool->cur_queue_size++;
     pthread_mutex_unlock (&(pool->queue_lock));
     /*Now. it has task.active a waiting thread.If all threads are busy, there is no use for this */
     pthread_cond_signal (&(pool->queue_ready));
     return 0;
}

/*destroy pool. The waiting task will not be performed. And the running task will quit after done.*/
int
pool_destroy ()
{
    if (pool->shutdown)
        return -1;
     pool->shutdown = 1;

    /*active all threads. And the pool will be destroyed*/
     pthread_cond_broadcast (&(pool->queue_ready));

    /*block all thread*/
    int i;
    for (i = 0; i < pool->max_thread_num; i++)
         pthread_join (pool->threadid[i], NULL);
     free (pool->threadid);

    /*destory task*/
     CThread_worker *head = NULL;
    while (pool->queue_head != NULL)
     {
         head = pool->queue_head;
         pool->queue_head = pool->queue_head->next;
         free (head);
     }
    /*destroy mutex and conditin*/
     pthread_mutex_destroy(&(pool->queue_lock));
     pthread_cond_destroy(&(pool->queue_ready));
    
     free (pool);
    /**/
     pool=NULL;
    return 0;
}


void *
thread_routine (void *arg)
{
     printf ("starting thread 0x%x\n", pthread_self ());
    while (1)
     {
         pthread_mutex_lock (&(pool->queue_lock));
        /*if not shutdown and queue is 0, the thread will wait*/
        while (pool->cur_queue_size == 0 && !pool->shutdown)
         {
             printf ("thread 0x%x is waiting\n", pthread_self ());
             pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock));
         }

        /*if destory*/
        if (pool->shutdown)
         {
            /*thread exit*/
             pthread_mutex_unlock (&(pool->queue_lock));
             printf ("thread 0x%x will exit\n", pthread_self ());
             pthread_exit (NULL);
         }
		/*else begin to work*/
         printf ("thread 0x%x is starting to work\n", pthread_self ());

        /**/
         assert (pool->cur_queue_size != 0);
         assert (pool->queue_head != NULL);
        
        /*queue minus 1 and get the worker*/
         pool->cur_queue_size--;
         CThread_worker *worker = pool->queue_head;
         pool->queue_head = worker->next;
         pthread_mutex_unlock (&(pool->queue_lock));

        /*process*/
         (*(worker->process)) (worker->arg);
         free (worker);
         worker = NULL;
     }
    /*it should not go here*/
     pthread_exit (NULL);
}

