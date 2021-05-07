
#include "common.h"
#include "resolver.h"

#include <sys/event.h>
#include <unistd.h>
#include <string.h>

s32
main(void)
{
	s32 ResolverFD = ResolverCreate();
	s32 Q          = kqueue();
	{
		struct kevent Change[2];
		EV_SET(Change+0, STDIN_FILENO, EVFILT_READ, EV_ADD, 0, 0, NULL);
		EV_SET(Change+1, ResolverFD,   EVFILT_READ, EV_ADD, 0, 0, NULL);
		if(kevent(Q, Change, ArrayCount(Change), NULL, 0, NULL) == -1)
		{
			perr("kevent");
		}
	}

	bool Running = true;
	while(Running)
	{
		s32 EventCount;
		struct kevent Event[32];
		EventCount = kevent(Q, NULL, 0, Event, ArrayCount(Event), NULL);
		if(EventCount == -1) perr("kevent");
		for(s32 i = 0; i < EventCount; ++i)
		{
			struct kevent *E = Event+i;
			switch(E->filter)
			{
				case EVFILT_READ:
				{
					s32 FD = E->ident;
					if(FD == STDIN_FILENO)
					{
						char Buffer[BUFSIZ+1];
						ssize_t Len = read(STDIN_FILENO, Buffer, BUFSIZ);
						if(Len == -1) perr("read");
						if(Buffer[Len-1] == '\n') --Len;
						Buffer[Len] = 0;
						if(strcmp(Buffer, "quit") == 0)
						{
							Running = false;
							break;
						}
						write(STDIN_FILENO, Buffer, Len);
						write(STDIN_FILENO, "\n", 1);

						ResolverLookup(ResolverFD, Buffer);

					} else if(FD == ResolverFD)
					{
						struct dns_result *Result = Resolve(FD);
						ResolverFree(Result);
					}
				};

			}
		}
	}
	ResolverDestroy(ResolverFD);
	return 0;
}
