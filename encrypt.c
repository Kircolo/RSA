#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <gmp.h>

#include "numtheory.h"
#include "randstate.h"
#include "ss.h"

#define OPTIONS "i:o:n:vh"

int main(int argc, char** argv) {
    FILE *infile = stdin;
    FILE *outfile = stdout;
    FILE *pub;
    char username[100];
    char *pub_name = "ss.pub";
    int opt = 0;
    int verb = 0;
    

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i': {
            infile = fopen(optarg, "r");
            if (!infile) {
                fprintf(stderr, "encrypt - Could not open infile: %s\n", optarg);
                return EXIT_FAILURE;
            }
            break;
        }
        case 'o': {
            outfile = fopen(optarg, "w");
            if (!outfile) {
                fprintf(stderr, "encrypt - Could not open outfile: %s\n", optarg);
                fclose(infile);
                return EXIT_FAILURE;
            }
            break;
        }
        case 'n': pub_name = optarg; break;
        case 'v': verb = 1; break;
        case 'h':
            printf(
                "SYNOPSIS\n"
                "  Encrypts a file using Schmidt-Samoa (SS) public key.\n\n"
                "USAGE\n"
                "  encrypt [-hv] [-i infile] [-o outfile] [-n pubkey]\n\n"
                "OPTIONS\n"
                "  -i infile     Input file (default: stdin).\n"
                "  -o outfile     Output file (default: stdout).\n"
                "  -n pubkey     Public key file (default: ss.pub).\n"
                "  -v            Verbose output.\n"
                "  -h            Display program usage.\n");
            return 0;
        }
    } // end of switch cases

    // open public key file
    pub = fopen(pub_name, "r");
    if (!pub) {
        fprintf(stderr, "encrypt - Could not open public key file: %s\n", pub_name);
        return EXIT_FAILURE;
    }

    mpz_t n;
    mpz_init(n);

    // read public key from public key file
    ss_read_pub(n, username, pub);

    // verbose output
    if (verb) {
        fprintf(stderr, "Username: %s\n", username);
        gmp_fprintf(stderr, "Public key n  (%zu bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
    }

    // encrypt file & clean up
    ss_encrypt_file(infile, outfile, n);
    if (infile  && infile  != stdin)  fclose(infile);
    if (outfile && outfile != stdout) fclose(outfile);
    fclose(pub);
    mpz_clear(n);
    return EXIT_SUCCESS;
}
