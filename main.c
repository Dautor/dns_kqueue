
#include "common.h"
#include "resolver.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

s32
main(void)
{
    struct dns_resolver resolver = dns_create(NULL);
    s32                 queue    = kqueue();
    {
        struct kevent change[2];
        EV_SET(change + 0, STDIN_FILENO, EVFILT_READ, EV_ADD, 0, 0, NULL);
        EV_SET(change + 1, resolver.fd, EVFILT_READ, EV_ADD, 0, 0, resolver.context);
        if(kevent(queue, change, ArrayCount(change), NULL, 0, NULL) == -1)
        {
            perr("kevent");
        }
    }

    bool running = true;
    while(running)
    {
        s32           event_count;
        struct kevent event[32];
        event_count = kevent(queue, NULL, 0, event, ArrayCount(event), NULL);
        if(event_count == -1)
            perr("kevent");
        for(s32 i = 0; i < event_count; ++i)
        {
            struct kevent *e = event + i;
            switch(e->filter)
            {
                case EVFILT_READ:
                {
                    s32 fd = e->ident;
                    if(fd == STDIN_FILENO)
                    {
                        char    buffer[BUFSIZ + 1];
                        ssize_t len = read(STDIN_FILENO, buffer, BUFSIZ);
                        if(len == -1)
                            perr("read");
                        if(buffer[len - 1] == '\n')
                            --len;
                        buffer[len] = 0;
                        if(strcmp(buffer, "quit") == 0)
                        {
                            running = false;
                            break;
                        }
                        write(STDIN_FILENO, buffer, len);
                        write(STDIN_FILENO, "\n", 1);

                        dns_lookup(resolver.context, 0, buffer, 0);

                    } else if(fd == resolver.fd)
                    {
                        struct in_addr *result = dns_result(e->udata, 0);
                        if(!result)
                        {
                            fprintf(stderr, "dns_result returned NULL\n");
                            continue;
                        }
                        printf("%s\n", inet_ntoa(*result));
                        dns_free(resolver.context, result);
                    }
                }
                break;
            }
        }
    }
    dns_destroy(resolver.context);
    return 0;
}
