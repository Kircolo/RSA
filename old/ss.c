#include "ss.h"
#include "numtheory.h"
#include "randstate.h"
#include <math.h>
#include <stdlib.h>

//learned about generating random numbers in a range from here
//https://www.geeksforgeeks.org/generating-random-number-range-c/
void ss_make_pub(mpz_t p, mpz_t q, mpz_t n, uint64_t nbits, uint64_t iters) {
    //mpz inits
    mpz_t p_check, q_check;
    mpz_inits(p_check, q_check, NULL);

    //make pbits and qbits
    uint64_t pbits = (random() % (((2 * nbits) / 5) - (nbits / 5)))
                     + (nbits / 5); //make random number with range
    uint64_t qbits = nbits - (2 * pbits); //remaining bits go to qbits

    //make primes
    make_prime(p, pbits, iters);
    make_prime(q, qbits, iters);

    //make minus one version
    mpz_sub_ui(p_check, p, 1);
    mpz_sub_ui(q_check, q, 1);
    while (mpz_divisible_p(p, q_check) || mpz_divisible_p(q, p_check)) {
        uint64_t pbits = (random() % (((2 * nbits) / 5) - (nbits / 5)))
                         + (nbits / 5); //make random number with range
        uint64_t qbits = nbits - (2 * pbits); //remaining bits go to qbits

        //make primes
        make_prime(p, pbits, iters);
        make_prime(q, qbits, iters);

        //make minus one version
        mpz_sub_ui(p_check, p, 1);
        mpz_sub_ui(q_check, q, 1);
    }

    mpz_mul(n, p, p);
    mpz_mul(n, n, q);

    mpz_clears(p_check, q_check, NULL);
}

void ss_make_priv(mpz_t d, mpz_t pq, const mpz_t p, const mpz_t q) {

    mpz_t n, p1, q1, numerator, lcm;
    mpz_inits(n, p1, q1, numerator, lcm, NULL);

    //set pq value
    mpz_mul(pq, p, q);

    //set n value
    //n = p*p*q
    mpz_mul(n, p, p);
    mpz_mul(n, n, q);

    //set minus one values
    mpz_sub_ui(p1, p, 1);
    mpz_sub_ui(q1, q, 1);

    //set lcm values
    //lcm = (p-1 * q-1)/ gcd(p-1, q-1)
    mpz_mul(numerator, p1, q1);
    gcd(lcm, p1, q1);
    mpz_fdiv_q(lcm, numerator, lcm);

    //create SS private key d
    //d = 1/(n % lcm)
    mod_inverse(d, n, lcm);

    //clean up
    mpz_clears(n, p1, q1, numerator, lcm, NULL);
}

void ss_write_pub(const mpz_t n, const char username[], FILE *pbfile) {
    //if valid
    if (pbfile) {
        gmp_fprintf(pbfile, "%Zx\n", n); //write n as a hex to pbfile
        gmp_fprintf(pbfile, "%s\n", username); //write username to pbfile
    }
}

void ss_write_priv(const mpz_t pq, const mpz_t d, FILE *pvfile) {
    //if valid
    if (pvfile) {
        gmp_fprintf(pvfile, "%Zx\n", pq); //write pq as a hex to pvfile
        gmp_fprintf(pvfile, "%Zx\n", d); //write d as a hex to pvfile
    }
}

void ss_read_pub(mpz_t n, char username[], FILE *pbfile) {
    //if valid
    if (pbfile) {
        gmp_fscanf(pbfile, "%Zx", n); //read n
        gmp_fscanf(pbfile, "%s", username); //read username
    }
}

void ss_read_priv(mpz_t pq, mpz_t d, FILE *pvfile) {
    //if valid
    if (pvfile) {
        gmp_fscanf(pvfile, "%Zx", pq); //read pq
        gmp_fscanf(pvfile, "%Zx", d); //read d
    }
}

void ss_encrypt(mpz_t c, const mpz_t m, const mpz_t n) {
    pow_mod(c, m, n, n);
}

void ss_encrypt_file(FILE *infile, FILE *outfile, const mpz_t n) {
    uint64_t read; //j
    //mpz inits
    mpz_t size; //k
    mpz_t convert; //m
    mpz_t root;
    mpz_t encrypt;
    mpz_t size2;
    mpz_inits(size, size2, convert, encrypt, root, NULL);

    //size
    mpz_sqrt(root, n); //make sqrt(n)
    mpz_sizeinbase(root, 2);
    mpz_sub_ui(size2, root, 1);
    //mpz_set_ui(size, mpz_sizeinbase(root, 2) - 1); //set numerator
    mpz_fdiv_q_ui(size, size2, 8); //set size
    uint8_t *arr = (uint8_t *) malloc(mpz_get_ui(size)); //make the block
    arr[0] = 0xFF; //set value of 0th byte

    //while there are unprocessed bytes in infile
    while (feof(infile) == 0) {
        read = fread(arr + 1, 1, mpz_get_ui(size) - 1, infile); //read from index 1 to k - 1
        mpz_import(convert, read + 1, 1, 1, 1, 0, arr); //convert read bytes to an mpz_t
        ss_encrypt(encrypt, convert, n); //encrypt message
        gmp_fprintf(outfile, "%Zx\n", encrypt); //write encrypted message to outfile
    }
    //clean up
    mpz_clears(size, convert, encrypt, NULL);
    free(arr);
}

void ss_decrypt(mpz_t m, const mpz_t c, const mpz_t d, const mpz_t pq) {
    pow_mod(m, c, d, pq);
}

void ss_decrypt_file(FILE *infile, FILE *outfile, const mpz_t d, const mpz_t pq) {
    uint64_t converted; //j
    //mpz inits
    mpz_t size; //k
    mpz_t c;
    mpz_t out;
    mpz_t root_pq;
    mpz_inits(size, c, out, root_pq, NULL);

    //size
    mpz_get_ui(pq);
    mpz_sqrt(root_pq, pq);
    mpz_sizeinbase(root_pq, 2);
    mpz_sub_ui(root_pq, root_pq, 1);
    mpz_fdiv_q_ui(size, root_pq, 8);
    // mpz_set_ui(size, mpz_sizeinbase(root_pq, 2) - 1); //set numerator
    uint8_t *arr = (uint8_t *) malloc(mpz_get_ui(size)); //make the block

    //while not end of file
    while (feof(infile) == 0) {
        converted = fread(arr + 1, 1, mpz_get_ui(size) - 1, infile); //read from index 1 to k - 1
        gmp_fscanf(outfile, "%Zx\n", c); //scan hexstring
        ss_decrypt(out, c, d, pq);
        mpz_export(arr, &converted, 1, 1, 1, 0, out); //convert c back to bytes
        fwrite(arr + 1, 1, converted - 1, outfile); //write out j-1 bytes to outfile
    }
    //clean up
    mpz_clears(size, c, out, root_pq, NULL);
    free(arr);
}
