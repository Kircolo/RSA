#include "ss.h"
#include "numtheory.h"
#include "randstate.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <gmp.h>

#define OPTIONS "i:o:n:vh"

int main(int argc, char **argv) {
    FILE *infile = stdin;
    FILE *outfile = stdout;
    FILE *priv;
    char *priv_name = "ss.priv";
    int opt = 0;
    int verb = 0;

    //mpz inits
    mpz_t pq;
    mpz_t d;
    mpz_inits(pq, d, NULL);

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w"); break;
        case 'n': priv_name = optarg; break;
        case 'v': verb = 1; break;
        case 'h':
            printf("SYNOPSIS\n   Decrypt.\n\n");
            printf("OPTIONS\n   -H\t\tDisplay program usage.\n   -i\t\tSpecifies the input file to "
                   "decrypt (DEFAULT: stdin).\n   -o\t\tSpecifies the input file to decrypt "
                   "(DEFAULT: stdout).\n   -n\t\tSpecifies the file containing the private key "
                   "(DEFAULT: ss.pub).\n   -v\t\tEnables verbose output.\n");
            break;
        }
    } //end of switch cases

    //open private key file
    priv = fopen(priv_name, "w");
    if (priv == NULL) {
        fprintf(stderr, "Could not open private key file.");
        return 8;
    }

    //read private key from private key file
    ss_read_priv(pq, d, priv);

    //verbose output
    if (verb == 1) {
        gmp_fprintf(stderr, "pq (%lu bits): %Z\n", mpz_sizeinbase(pq, 2), pq);
        gmp_fprintf(stderr, "d (%lu bits): %Z\n", mpz_sizeinbase(d, 2), d);
    }

    //decrypt file
    ss_decrypt_file(infile, outfile, d, pq);

    //close files
    fclose(priv);
    fclose(infile);
    fclose(outfile);

    //clear mpz values
    mpz_clears(pq, d, NULL);
}
