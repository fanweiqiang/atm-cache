#include <atm_core.h>


/*
 * Private 
 * */


static void
atm_event_process_ev();

static void
atm_event_process_events();


static int                   ep = -1;
static struct epoll_event   *event_list = NULL;
static atm_uint_t            nevents = 0;


/* ---------------------IMPLEMENTATIONS--------------------------- */
/*
 * Private
 * */
static void
atm_event_process_ev(atm_event_t *ev,uint32_t evs)
{

   if (evs & (EPOLLERR|EPOLLHUP)) {
       evs |= (EPOLLIN | EPOLLOUT);
   }

   if ((evs & EPOLLIN) && ev->active) {
       if (ev->handle_read != NULL) {
           ev->rdy_read = ATM_TRUE;
           ev->handle_read(ev);
       }
   }
   if ((evs & EPOLLOUT) && ev->active) {
       if (ev->handle_write != NULL) {
           ev->rdy_write = ATM_TRUE;
           ev->handle_write(ev);
       }
   }
}


static void
atm_event_process_events()
{
    int             events,i = 0;
    uint32_t        evs = 0; 
    atm_event_t    *ev = NULL;

    events = epoll_wait(ep,event_list,
            (int) nevents,ATM_EVENT_BLOCK);
    
    if (events > 0) {
       for (i=0; i<events; ++i) {
           /* TODO thunder herd problem solved? */
           ev = event_list[i].data.ptr;
           if (ev->fd == -1) {
               continue; 
           }
           evs = event_list[i].events;
           atm_event_process_ev(ev, evs);
       }
    }
}


/*
 * Public
 * */
void
atm_event_init()
{
    /* set up event engine. */
    ep = epoll_create(ATM_EVENT_SIZE);
    if (event_list) {
        atm_free(event_list);
    }
    nevents = ATM_EVENT_LIST_SIZE;
    event_list = atm_alloc(
        sizeof(struct epoll_event) * nevents);
}


/* get new event instance*/
atm_event_t *
atm_event_new(void *load, int fd, 
        void (*handle_read)(void *ev),
        void (*handle_write)(void *ev))
{
    atm_event_t *res = NULL;
    res = atm_alloc(sizeof(atm_event_t));
    res->fd = fd;
    res->active = ATM_FALSE;
    res->load = load;
    res->rdy_read = ATM_FALSE;
    res->rdy_write = ATM_FALSE;
    res->handle_read = handle_read;
    res->handle_write = handle_write;
    return res;
}


void
atm_event_free(void *e)
{
    atm_free(e);
}


/* public funcs */
void
atm_event_routine()
{
    atm_event_process_events();
}


void
atm_event_add_listen(atm_listen_t *l)
{
    int sfd = -1;
    atm_event_t *le = NULL;
    int events = ATM_EVENT_NONE;

    if (l != NULL) {
        sfd = l->ssck->fd;
        le = atm_event_new(l,sfd,
                l->handle_accept,NULL); 
        l->event = le;

        events = EPOLLIN|EPOLLHUP;
        atm_event_add_event(le, events);
    }
}


void
atm_event_add_conn(atm_conn_t *c)
{
    int cfd = -1;
    atm_event_t *ce = NULL;
    int events = ATM_EVENT_NONE;

    if (c != NULL) {
        cfd = c->sock->fd;
        ce = atm_event_new(c, cfd, 
                c->handle_read, 
                c->handle_write); 
        c->event = ce;

        events = EPOLLIN|EPOLLHUP|EPOLLET;
        atm_event_add_event(ce, events);
    }
}


/* 
 * 1. if e not managed by epoll then add it
 * 2. if the event and old mask then merge it
 * 3. mask the events's fd's bits in epoll
 * 4. activate it.
 */
void
atm_event_add_event(atm_event_t *e, int mask)
{
   int fd =-1;
   int op = 0;
   struct epoll_event ee;

   if (e != NULL) {
       fd = e->fd;
       if (e->active) {
           op = EPOLL_CTL_MOD;
       } else {
           op = EPOLL_CTL_ADD;
       }
       e->events = e->events | mask;
       if (e->events != ATM_EVENT_NONE) {
           ee.events = e->events;
           ee.data.ptr = e;
           epoll_ctl(ep, op, fd, &ee);
           e->active = ATM_TRUE;
       }
   }
}


/*
 * 1. unmsk the event's fd's bits in epoll 
 * 2. if fd's bits is empty then del it from epoll
 *    and unactivate it.
 */
void
atm_event_del_event(atm_event_t *e, int unmask)
{
   int fd =-1;
   int op = 0;
   struct epoll_event ee;

   if (e != NULL) {
       fd = e->fd;
       e->events = e->events & (~unmask);
       if (e->events != ATM_EVENT_NONE) {
           op = EPOLL_CTL_MOD;
       } else {
           op = EPOLL_CTL_DEL;
       }
       if (e->active) {
           ee.events = e->events;
           ee.data.ptr = e;
           epoll_ctl(ep, op, fd, &ee);
           if (op == EPOLL_CTL_DEL) {
               e->active = ATM_FALSE;
           }
       }
   }
}