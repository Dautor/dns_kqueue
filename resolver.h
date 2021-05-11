#pragma once

#include "common.h"

enum record_type
{
    DNS_RECORD_A
};

struct dns_ctx;
struct dns_resolver
{
    s32             fd;
    struct dns_ctx *context;
    void *          custom_resolver_result;
};

struct dns_resolver dns_create(void *resolver_specific_data);
s32                 dns_destroy(struct dns_ctx *);
s32                 dns_lookup(struct dns_ctx *, u64 flags, char const *name, enum record_type type);
struct in_addr *    dns_result(struct dns_ctx *, u64 flags);
s32                 dns_free(struct dns_ctx *, struct in_addr *result);
