#ifndef _ATM_PIPE_H_INCLUDED_
#define _ATM_PIPE_H_INCLUDED_

#include <atm_core.h>

#define ATM_PIPE_NCHAR 'a'

struct atm_pipe_s {
    int                 recv_fd;
    int                 sent_fd;
    atm_queue_t*        mqueue;
    pthread_mutex_t     mqlock;
    /* if epoll is using for msging */
    atm_event_t*        event;
};

typedef struct {
    void       *load;
    void      (*call_back)(void *load);
} atm_pipe_msg_t;

atm_pipe_t *
atm_pipe_new();

/*
 * trust event process to epoll
 */
void
atm_pipe_event_init(atm_pipe_t *pipe);

void
atm_pipe_free(void *pipe);

void
atm_pipe_notify(atm_pipe_t *pipe, void *load,
        void (*call_back)(void *load));


#endif /* _ATM_PIPE_H_INCLUDED_ */
