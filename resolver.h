#pragma once

#include "common.h"

struct dns_result
{

};

s32 ResolverCreate(void);
s32 ResolverDestroy(s32 FD);
struct dns_result *Resolve(s32 FD);
void ResolverFree(struct dns_result *Result);
void ResolverLookup(s32 FD, char const *Name);
