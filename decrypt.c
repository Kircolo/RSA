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
    FILE *priv;
    char *priv_name = "ss.priv";
    int opt = 0;
    int verb = 0;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i': {
            infile = fopen(optarg, "r"); 
            if (!infile) {
                fprintf(stderr, "decrypt - Could not open infile: %s\n", optarg);
                return EXIT_FAILURE;
            }
            break;}
        case 'o': {
            outfile = fopen(optarg, "w");
            if (!outfile) {
                fprintf(stderr, "decrypt - Could not open outfile: %s\n", optarg);
                fclose(infile);
                return EXIT_FAILURE;
            }
            break;}
        case 'n': priv_name = optarg; break;
        case 'v': verb = 1; break;
        case 'h':
            printf(
                "SYNOPSIS\n"
                "  Decrypts a file using Schmidt-Samoa (SS) private key.\n\n"
                "USAGE\n"
                "  decrypt [-hv] [-i infile] [-o outfile] [-n privkey]\n\n"
                "OPTIONS\n"
                "  -i infile     Input file (default: stdin).\n"
                "  -o outfile    Output file (default: stdout).\n"
                "  -n privkey    Private key file (default: ss.priv).\n"
                "  -v            Verbose output.\n"
                "  -h            Display program usage.\n");
            return 0;
        }
    } // end of switch cases

    // open private key file
    priv = fopen(priv_name, "r");
    if (!priv) {
        fprintf(stderr, "decrypt - Could not open private key file: %s\n", priv_name);
        fclose(infile);
        fclose(outfile);
        return EXIT_FAILURE;
    }

    mpz_t d, pq;
    mpz_inits(d, pq, NULL);

    // read private key from private key file
    ss_read_priv(pq, d, priv);

    // verbose output
    if (verb) {
        gmp_fprintf(stderr, "Private modulus pq (%zu bits): %Zd\n", mpz_sizeinbase(pq, 2), pq);
        gmp_fprintf(stderr, "Private key d (%zu bits): %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    // decrypt the input file
    ss_decrypt_file(infile, outfile, d, pq);

    // close files and clear state
    if (infile  && infile  != stdin)  fclose(infile);
    if (outfile && outfile != stdout) fclose(outfile);
    fclose(priv);
    mpz_clears(d, pq, NULL);
    return EXIT_SUCCESS;
}
