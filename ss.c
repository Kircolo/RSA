// #include 
#include "ss.h"

void ss_make_pub(mpz_t p, mpz_t q, mpz_t n, uint64_t nbits, uint64_t iters) {

}

void ss_make_priv(mpz_t d, mpz_t pq, const mpz_t p, const mpz_t q) {

}

void ss_write_pub(const mpz_t n, const char username[], FILE *pbfile) {

}

void ss_write_priv(const mpz_t pq, const mpz_t d, FILE *pvfile) {

}

void ss_read_pub(mpz_t n, char username[], FILE *pbfile) {

}

void ss_read_priv(mpz_t pq, mpz_t d, FILE *pvfile) {

}

void ss_encrypt(mpz_t c, const mpz_t m, const mpz_t n){

}

void ss_encrypt_file(FILE *infile, FILE *outfile, const mpz_t n) {

}

void ss_decrypt(mpz_t m, const mpz_t c, const mpz_t d, const mpz_t pq){

}

void ss_decrypt_file(FILE *infile, FILE *outfile, const mpz_t d, const mpz_t pq) {

}
