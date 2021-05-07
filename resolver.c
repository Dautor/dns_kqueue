
#include "common.h"

#include <sys/event.h>
#include <unistd.h>

s32
ResolverCreate(void)
{
	// IMPORTANT(dautor): How are we supposed to track resolver-specific data?
	// We could pass a pointer to a struct that we use and require user to pass it as udata in kevent,
	// but that is not good enough... the user might want to store their own data... They could chain
	// us in their struct but that feels kinda silly... We should probably remain opaque.
	s32 FD = kqueue();
	if(FD == -1) return -1;
	struct kevent Change[1];
	EV_SET(Change+0, 0x0123456789, EVFILT_USER, EV_ADD|EV_CLEAR, 0, 0, NULL);
	if(kevent(FD, Change, ArrayCount(Change), NULL, 0, NULL) == -1) perr("kevent");
	return FD;
}

s32
ResolverDestroy(s32 FD)
{
	return close(FD);
}

struct dns_result *
Resolve(s32 FD)
{
	s32 EventCount;
	struct kevent Event[4];
	EventCount = kevent(FD, NULL, 0, Event, ArrayCount(Event), NULL);
	if(EventCount == -1) perr("kevent");
	for(s32 i = 0; i < EventCount; ++i)
	{
		struct kevent *E = Event+i;
		switch(E->filter)
		{
            case EVFILT_USER:
            {
                if(E->ident == 0x0123456789)
				{
					printf("yay\n");
				}
			};
		}
	}
    return NULL;
}

void
ResolverFree(struct dns_result *Result)
{
	if(Result != NULL) free(Result);
}

void
ResolverLookup(s32 FD, char const *Name)
{
	struct kevent Change[1];
	EV_SET(Change+0, 0x0123456789, EVFILT_USER, 0, NOTE_FFNOP|NOTE_TRIGGER, 0, NULL);
	if(kevent(FD, Change, ArrayCount(Change), NULL, 0, NULL) == -1) perr("kevent");
	(void)Name;
}

