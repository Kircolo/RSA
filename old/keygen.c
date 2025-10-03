#include "ss.h"
#include "numtheory.h"
#include "randstate.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <gmp.h>
#include <time.h>
#include <sys/stat.h>

#define OPTIONS "b:i:n:d:s:vh"

int main(int argc, char **argv) {
    uint64_t bits;
    uint64_t iters = 50;
    uint64_t seed = time(NULL);
    FILE *pub;
    FILE *priv;
    char *pub_name = "ss.pub";
    char *priv_name = "ss.priv";
    int opt = 0;
    int verb = 0;

    //mpz_t inits
    mpz_t p;
    mpz_t q;
    mpz_t n;
    mpz_t d;
    mpz_t pq;
    mpz_inits(p, q, n, d, pq, NULL);

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'b': bits = atoll(optarg); break;
        case 'i': iters = atoll(optarg); break;
        case 'n': pub_name = optarg; break;
        case 'd': priv_name = optarg; break;
        case 's': seed = atoll(optarg); break;
        case 'v': verb = 1; break;
        case 'h':
            printf("SYNOPSIS\n   Generate keys.\n\n");
            printf(
                "OPTIONS\n   -H\t\tDisplay program usage.\n   -b\t\tSpecifies the minimum bits "
                "needed for the public modulus n.\n   -i\t\tSpecifies the number of Miller-Rabin "
                "iterations for testing primes (DEFAULT: 50).\n   -n pbfile\tSpecifies the public "
                "key file (DEFAULT: ss.pub).\n   -d pvfile\tSpecifies the private key file "
                "(DEFAULT: ss.priv).\n   -s\t\tSpecifies the random seed for the random state "
                "initialization (DEFAULT: the seconds since the UNIX epoch, given by "
                "time(NULL)).\n   -v\t\tEnables verbose output.\n");
        }
    } //end of switch cases

    //read files with error checks
    pub = fopen(pub_name, "w");
    if (pub == NULL) {
        fprintf(stderr, "Could not open public key file.");
        return 8;
    }
    priv = fopen(priv_name, "w");
    if (priv == NULL) {
        fprintf(stderr, "Could not open private key file.");
        return 8;
    }

    //set permissions to priv file as 0600 for r/w permissions for just the user
    fchmod(fileno(priv), 0600);

    //init random state with set seed
    randstate_init(seed);

    //make public key
    ss_make_pub(p, q, n, bits, iters);
    //make private key
    ss_make_priv(d, pq, p, q);

    //get username
    char *username = getenv("USERNAME");

    //write public key to public file
    ss_write_pub(n, username, pub);

    //write public key to public file
    ss_write_priv(pq, d, priv);

    //verbose output
    if (verb == 1) {
        fprintf(stderr, "username: %s\n", username);
        gmp_fprintf(stderr, "p (%lu bits): %Z\n", mpz_sizeinbase(p, 2), p);
        gmp_fprintf(stderr, "q (%lu bits): %Z\n", mpz_sizeinbase(q, 2), q);
        gmp_fprintf(stderr, "n (%lu bits): %Z\n", mpz_sizeinbase(n, 2), n);
        gmp_fprintf(stderr, "d (%lu bits): %Z\n", mpz_sizeinbase(d, 2), d);
        gmp_fprintf(stderr, "pq (%lu bits): %Z\n", mpz_sizeinbase(pq, 2), pq);
    }

    //close files
    fclose(pub);
    fclose(priv);

    //clear state
    randstate_clear();

    //clear mpz values
    mpz_clears(p, q, n, d, pq, NULL);
}
