#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "maxminddb.h"
#include "maxminddb_app_helper.h"
#include "getopt.h"
#include <assert.h>
#include <netdb.h>

int main(int argc, char *const argv[])
{
    int verbose = 0;
    int character;
    char *fname = NULL;
    MMDB_s *mmdb;
    uint16_t status;

    while ((character = getopt(argc, argv, "vf:")) != -1) {
        switch (character) {
        case 'v':
            verbose = 1;
            break;
        case 'f':
            fname = strdup(optarg);
            break;
        default:
        case '?':
            usage(argv[0]);
        }
    }
    argc -= optind;
    argv += optind;

    if (!fname) {
        fname = strdup(MMDB_DEFAULT_DATABASE);
    }

    assert(fname != NULL);

    status = MMDB_open(fname, MMDB_MODE_STANDARD, mmdb);

    if (!mmdb) {
        fprintf(stderr, "Can't open %s\n", fname);
        exit(1);
    }

    free(fname);

    char *ipstr = argv[0];
    union {
        struct in_addr v4;
        struct in6_addr v6;
    } ip;

    int ai_family = is_ipv4(mmdb) ? AF_INET : AF_INET6;
    int ai_flags = AI_V4MAPPED;

    if (ipstr == NULL || 0 != MMDB_resolve_address(ipstr, ai_family, ai_flags,
                                                  &ip)) {
        fprintf(stderr, "Invalid IP\n");
        exit(1);
    }

    if (verbose) {
        dump_meta(mmdb);
    }

    MMDB_lookup_result_s *result = lookup_or_die(mmdb, ipstr);

    if (result->entry.offset > 0) {
        MMDB_decode_all_s *decode_all;
        MMDB_get_tree(&result->entry, &decode_all);
    } else {
        puts("Sorry, nothing found");
    }

    return (0);
}
