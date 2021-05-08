
#include "common.h"
#include "resolver.h"

#include <sys/event.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <err.h>

struct dns
{
	s32 fd;
	struct addrinfo *result;
	char name[32];
};

struct dns_resolver
dns_create(void)
{
	struct dns_resolver result;
	result.fd = kqueue();
	if(result.fd == -1) return (struct dns_resolver){.fd=-1};
	struct kevent change[1];
	EV_SET(change+0, 0x0123456789, EVFILT_USER, EV_ADD|EV_CLEAR, 0, 0, NULL);
	if(kevent(result.fd, change, ArrayCount(change), NULL, 0, NULL) == -1)
	{
		perr("kevent");
	}
	result.data = (struct dns *)malloc(sizeof(struct dns));
	result.data->fd = result.fd;
	return result;
}

s32
dns_destroy(struct dns *dns)
{
	return close(dns->fd);
}

struct addrinfo *
dns_result(struct dns *dns)
{
	struct kevent event[4];
	s32 event_count = kevent(dns->fd, NULL, 0, event, ArrayCount(event), NULL);
	if(event_count == -1) perr("kevent");
	for(s32 i = 0; i < event_count; ++i)
	{
		struct kevent *e = event+i;
		switch(e->filter)
		{
			case EVFILT_USER:
			{
				if(e->ident == 0x0123456789)
				{
					return dns->result;
				}
			} break;
		}
	}
	return NULL;
}

s32
dns_free(struct dns *dns, struct addrinfo *result)
{
	(void)dns;
	if(result)
	{
		freeaddrinfo(result);
	}
	return 0;
}

s32
dns_lookup(struct dns *dns, char const *name)
{
	{
		struct addrinfo hints = {};
		hints.ai_family   = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		s32 error = getaddrinfo(name, NULL, &hints, &dns->result);
		if(error)
		{
			errx(1, "%s", gai_strerror(error));
		}
	}
	{
		struct kevent change[1];
		EV_SET(change+0, 0x0123456789, EVFILT_USER, 0, NOTE_FFNOP|NOTE_TRIGGER, 0, NULL);
		if(kevent(dns->fd, change, ArrayCount(change), NULL, 0, NULL) == -1) perr("kevent");
	}
	return 0;
}
