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
    FILE *pub;
    char username[100];
    char *pub_name = "ss.pub";
    int opt = 0;
    int verb = 0;

    //mpz inits
    mpz_t n;
    mpz_init(n);

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w"); break;
        case 'n': pub_name = optarg; break;
        case 'v': verb = 1; break;
        case 'h':
            printf("SYNOPSIS\n   Encrypt.\n\n");
            printf("OPTIONS\n   -H\t\tDisplay program usage.\n   -i\t\tSpecifies the input file to "
                   "encrypt (DEFAULT: stdin).\n   -o\t\tSpecifies the input file to encrypt "
                   "(DEFAULT: stdout).\n   -n\t\tSpecifies the file containing the public key "
                   "(DEFAULT: ss.pub).\n   -v\t\tEnables verbose output.\n");
            break;
        }
    } //end of switch cases

    //open public key file
    pub = fopen(pub_name, "r");
    if (pub == NULL) {
        fprintf(stderr, "Could not open public key file.");
        return 8;
    }

    //read public key from public key file
    ss_read_pub(n, username, pub);

    //verbose output
    if (verb == 1) {
        fprintf(stderr, "username: %s\n", username);
        gmp_fprintf(stderr, "n (%lu bits): %Z\n", mpz_sizeinbase(n, 2), n);
    }

    //encrypt file
    ss_encrypt_file(infile, outfile, n);

    //close files
    fclose(pub);
    fclose(infile);
    fclose(outfile);

    //clear mpz values
    mpz_clear(n);
}
