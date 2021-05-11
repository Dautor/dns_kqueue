
#include "resolver.h"
#include "common.h"

#include <err.h>
#include <netdb.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <arpa/nameser.h>
#include <netinet/in.h>
#include <resolv.h>
#include <string.h>

// TODO: LUT for enum Record to native record type format mapping

struct dns_ctx
{
    int                fd;
    struct __res_state state;

    uint8_t query_buf[PACKETSZ];
    uint8_t ans_buf[PACKETSZ];
    size_t  query_buf_cur_len;
    size_t  ans_buf_cur_len;
};

struct dns_resolver
dns_create(struct sockaddr_in *ns)
{
    struct dns_resolver result = {};
    result.fd                  = kqueue();
    if(result.fd == -1)
        return (struct dns_resolver){.fd = -1};
    struct kevent change[1];
    EV_SET(change + 0, RES_IDENT, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, NULL);
    if(kevent(result.fd, change, ArrayCount(change), NULL, 0, NULL) == -1)
    {
        perr("kevent");
    }
    result.pCtx = (struct dns_ctx *)malloc(sizeof(struct dns_ctx));
    if(!result.pCtx)
    {
        perr("malloc");
        return (struct dns_resolver){.fd = -1};
    }
    result.pCtx->fd = result.fd;

    res_ninit(&result.pCtx->state);
    if(ns)
    {
        result.pCtx->state.nsaddr_list[0] = *ns;
        result.pCtx->state.nscount        = 1;
    }

    return result;
}

int
dns_destroy(struct dns_ctx *dns)
{
    res_nclose(&dns->state);
    return close(dns->fd);
}

struct in_addr *
dns_result(struct dns_ctx *dns)
{

    struct kevent event[4];
    int           event_count = kevent(dns->fd, NULL, 0, event, ArrayCount(event), NULL);
    if(event_count == -1)
        perr("kevent");
    for(int i = 0; i < event_count; ++i)
    {
        struct kevent *e = event + i;
        switch(e->filter)
        {
            case EVFILT_USER:
            {
                if(e->ident == RES_IDENT)
                {
                    ns_msg handle;
                    ns_rr  rr;

                    if(ns_initparse(dns->ans_buf, dns->ans_buf_cur_len, &handle) < 0)
                    {
                        perror("ns_initparse");
                        return NULL;
                    }

                    // ns_s_an == Answer
                    size_t msg_count = ns_msg_count(handle, ns_s_an);
                    if(!msg_count)
                    {
                        fprintf(stderr, "ns_msg_count: no query answers found");
                        return NULL;
                    }

                    for(size_t i = 0; i < msg_count; i++)
                    {
                        if(ns_parserr(&handle, ns_s_an, i, &rr))
                        {
                            perror("ns_parserr");
                            return NULL;
                        }

                        if(ns_rr_type(rr) == ns_t_a)
                        {
                            break;
                        }
                    }

                    struct in_addr *result = (struct in_addr *)malloc(sizeof(struct in_addr));
                    if(!result)
                    {
                        perr("malloc");
                        return NULL;
                    }

                    const uint8_t *data = ns_rr_rdata(rr);
                    memcpy(&result->s_addr, data, sizeof(result->s_addr));

                    return result;
                }
            }
            break;
        }
    }
    return NULL;
}

int
dns_free(struct dns_ctx *dns, struct in_addr *result)
{
    (void)dns;
    if(result)
    {
        free(result);
    }
    return 0;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
int
dns_lookup(struct dns_ctx *dns, char const *dname, enum Record type, unsigned int flags)
{
    struct kevent change[1];

    // TODO: implement thread pool
    // TODO: check for DNS_NONBLOCK flag

    int err = res_nmkquery(&dns->state, QUERY, dname, C_IN, T_A, NULL, 0, NULL, dns->query_buf, PACKETSZ);
    if(err < 0)
    {
        herror("res_nmkquery");
        return -1;
    }

    int msg_len            = err;
    dns->query_buf_cur_len = msg_len;
    err                    = res_nsend(&dns->state, dns->query_buf, msg_len, dns->ans_buf, PACKETSZ);
    if(err < 0)
    {
        herror("res_nsendquery");
        dns->query_buf_cur_len = 0;

        return -1;
    }

    dns->ans_buf_cur_len = err;

    EV_SET(change + 0, RES_IDENT, EVFILT_USER, 0, NOTE_FFNOP | NOTE_TRIGGER, 0, NULL);

    if(kevent(dns->fd, change, ArrayCount(change), NULL, 0, NULL) == -1)
    {
        perr("kevent");
        dns->query_buf_cur_len = dns->ans_buf_cur_len = 0;
        return -1;
    }

    return 0;
}
#pragma clang diagnostic pop
