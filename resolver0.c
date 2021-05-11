
#include "common.h"
#include "resolver.h"

#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct dns_ctx
{
    s32              fd;
    struct addrinfo *result;
};

struct dns_resolver
dns_create(void *unused)
{
    (void)unused;
    struct dns_resolver result = {};
    result.fd                  = kqueue();
    if(result.fd == -1)
        return (struct dns_resolver){.fd = -1};
    struct kevent change[1];
    EV_SET(change + 0, 0x0123456789, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, NULL);
    if(kevent(result.fd, change, ArrayCount(change), NULL, 0, NULL) == -1)
    {
        perr("kevent");
    }
    result.context     = (struct dns_ctx *)malloc(sizeof(struct dns_ctx));
    result.context->fd = result.fd;
    return result;
}

s32
dns_destroy(struct dns_ctx *dns)
{
    return close(dns->fd);
}

struct in_addr *
dns_result(struct dns_ctx *dns, u64 flags)
{
    (void)flags;
    struct kevent event[4];
    s32           event_count = kevent(dns->fd, NULL, 0, event, ArrayCount(event), NULL);
    if(event_count == -1)
        perr("kevent");
    for(s32 i = 0; i < event_count; ++i)
    {
        struct kevent *e = event + i;
        switch(e->filter)
        {
            case EVFILT_USER:
            {
                if(e->ident == 0x0123456789)
                {
                    return &((struct sockaddr_in *)dns->result->ai_addr)->sin_addr;
                }
            }
            break;
        }
    }
    return NULL;
}

s32
dns_free(struct dns_ctx *dns, struct in_addr *result)
{
    if(result != &((struct sockaddr_in *)dns->result->ai_addr)->sin_addr)
    {
        // indicate result is not vaild via errno
        return -1;
    }
    if(dns->result)
    {
        freeaddrinfo(dns->result);
        dns->result = NULL;
    }
    return 0;
}

s32
dns_lookup(struct dns_ctx *dns, u64 flags, char const *name, enum record_type type)
{
    (void)flags;
    (void)type;
    {
        struct addrinfo hints = {};
        hints.ai_family       = AF_UNSPEC;
        hints.ai_socktype     = SOCK_STREAM;
        s32 error             = getaddrinfo(name, NULL, &hints, &dns->result);
        if(error)
        {
            errx(1, "%s", gai_strerror(error));
        }
    }
    {
        struct kevent change[1];
        EV_SET(change + 0, 0x0123456789, EVFILT_USER, 0, NOTE_FFNOP | NOTE_TRIGGER, 0, NULL);
        if(kevent(dns->fd, change, ArrayCount(change), NULL, 0, NULL) == -1)
            perr("kevent");
    }
    return 0;
}
