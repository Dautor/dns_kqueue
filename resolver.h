#pragma once

#include "common.h"

#define RECORD_ENUM \
    X(DNS_RECORD_A)

#define X(field) field,

enum Record
{
    RECORD_ENUM
};

#undef X

#define DNS_FLAG_NONBLOCK 1

struct dns_ctx;
struct dns_resolver
{
    s32             fd;
    struct dns_ctx *pCtx;
    void *          custom_resolver_result;
};

struct sockaddr_in;

struct dns_resolver dns_create(struct sockaddr_in *ns);
s32                 dns_destroy(struct dns_ctx *);
s32                 dns_lookup(struct dns_ctx *, char const *name, enum Record type, unsigned int flags);
struct in_addr *    dns_result(struct dns_ctx *);
s32                 dns_free(struct dns_ctx *, struct in_addr *result);
