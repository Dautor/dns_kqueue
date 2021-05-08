#pragma once

#include "common.h"

struct dns;

struct dns_resolver
{
	s32 fd;
	struct dns *data;
};

struct dns_resolver dns_create (void);
s32                 dns_destroy(struct dns *);
s32                 dns_lookup (struct dns *, char const *name);
struct addrinfo    *dns_result (struct dns *);
s32                 dns_free   (struct dns *, struct addrinfo *result);
