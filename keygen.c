#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <getopt.h>
#include <inttypes.h>
#include <errno.h>
#include <ctype.h>
#include <gmp.h>

#include "numtheory.h"
#include "randstate.h"
#include "ss.h"

#define OPTIONS "b:i:n:d:s:vh"

int main(int argc, char** argv) {
    uint64_t bits = 1024;
    uint64_t iters = 50;
    uint64_t seed = time(NULL);
    FILE *pub;
    FILE *priv;
    char *pub_name = "ss.pub";
    char *priv_name = "ss.priv";
    int opt = 0;
    int verb = 0;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'b': { // bits; digits >= 1
            for (const char *t = optarg; *t; t++) {
                if (!isdigit((unsigned char)*t)) {
                    fprintf(stderr, "keygen: invalid -b <bits>: \"%s\"\n", optarg);
                    return EXIT_FAILURE;
                }
            }
            errno = 0;
            char *end = NULL;
            unsigned long long val = strtoul(optarg, &end, 10);
            if (errno || end == optarg || *end != '\0' || val < 1ULL) {
                fprintf(stderr, "keygen: invalid -b <bits>: \"%s\"\n", optarg);
                return EXIT_FAILURE;
            }
            bits = (uint64_t) val;
            break;
        }
        case 'i': { // iters; digits >= 1
            for (const char *t = optarg; *t; t++) {
                if (!isdigit((unsigned char)*t)) {
                    fprintf(stderr, "keygen: invalid -i <iters>: \"%s\"\n", optarg);
                    return EXIT_FAILURE;
                }
            }
            errno = 0;
            char *end = NULL;
            unsigned long long val = strtoull(optarg, &end, 10);
            if (errno || end == optarg || *end != '\0' || val < 1ULL) {
                fprintf(stderr, "keygen: invalid -i <iters>: \"%s\"\n", optarg);
                return EXIT_FAILURE;
            }
            iters = (uint64_t) val;
            break;
        }
        case 'n': pub_name = optarg; break;
        case 'd': priv_name = optarg; break;
        case 's': { // seed
            for (const char *t = optarg; *t; t++) {
                if (!isdigit((unsigned char)*t)) {
                    fprintf(stderr, "keygen: invalid -s <seed>: \"%s\"\n", optarg);
                    return EXIT_FAILURE;
                }
            }
            errno = 0;
            char *end = NULL;
            unsigned long long val = strtoull(optarg, &end, 10);
            if (errno || end == optarg || *end != '\0') {
                fprintf(stderr, "keygen: invalid -s <seed>: \"%s\"\n", optarg);
                return EXIT_FAILURE;
            }
            seed = (uint64_t) val;
            break;
        }
        case 'v': verb = 1; break;
        case 'h':
            printf(
                "SYNOPSIS\n"
                "  Generate Schmidt-Samoa (SS) public and private keys.\n\n"
                "USAGE\n"
                "  keygen [-hv] [-b bits] [-i iters] [-n pbfile] [-d pvfile] [-s seed]\n\n"
                "OPTIONS\n"
                "  -b bits       Min bit-length of modulus n (default: 1024).\n"
                "  -i iters      Miller-Rabin iterations (default: 50).\n"
                "  -n pbfile     Public key output (default: ss.pub).\n"
                "  -d pvfile     Private key output (default: ss.priv).\n"
                "  -s seed       RNG seed (default: time(NULL)).\n"
                "  -v            Verbose output.\n"
                "  -h            Display program usage.\n");
            return 0;
        }
    }

    //read files with error checks
    pub = fopen(pub_name, "w");
    if (!pub) {
        fprintf(stderr, "keygen -  Could not open public key file: %s\n", pub_name);
        return EXIT_FAILURE;
    }
    priv = fopen(priv_name, "w");
    if (!priv) {
        fprintf(stderr, "keygen -  Could not open private key file: %s\n", priv_name);
        fclose(pub);
        return EXIT_FAILURE;
    }

    //set permissions to priv file as 0600 for r/w permissions for just the user
    fchmod(fileno(priv), 0600);

    //init random state with set seed
    randstate_init(seed);

    //mpz_t inits
    mpz_t p, q, n, d, pq;
    mpz_inits(p, q, n, d, pq, NULL);

    // make keys
    ss_make_pub(p, q, n, bits, iters);  // p,q are primes; n = p^2 * q
    ss_make_priv(d, pq, p, q);          // pq = p*q ; d = n^{-1} mod lcm(p-1,q-1)

    // get username
    const char *user = getenv("USER"); 
    if(!user) {
        user = "user";
    }

    // write keys to files
    ss_write_pub(n, user, pub);
    ss_write_priv(pq, d, priv);

    // verbose output
    if (verb) {
        fprintf(stderr, "Username: %s\n", user);
        gmp_fprintf(stderr, "First large prime p  (%zu bits) = %Zd\n", mpz_sizeinbase(p, 2), p);
        gmp_fprintf(stderr, "Second large prime q  (%zu bits) = %Zd\n", mpz_sizeinbase(q, 2), q);
        gmp_fprintf(stderr, "Public key n  (%zu bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_fprintf(stderr, "Private exponent d  (%zu bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
        gmp_fprintf(stderr, "Private modulus pq (%zu bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq);
        fprintf(stderr, "Seed: %" PRIu64 "\n", seed);
    }
    
    // close files, clear state & mpz values
    fclose(pub);
    fclose(priv);
    randstate_clear();
    mpz_clears(p, q, n, d, pq, NULL);
    return EXIT_SUCCESS;
}
