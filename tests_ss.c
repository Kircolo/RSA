#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <gmp.h>

#include "randstate.h"
#include "numtheory.h"
#include "ss.h"

static size_t enc_k_from_n(const mpz_t n) {
    mpz_t root; mpz_init(root);
    mpz_sqrt(root, n);                                  // √n
    size_t k = (mpz_sizeinbase(root, 2) - 1) / 8;       // floor((log2(√n)-1)/8)
    mpz_clear(root);
    return k;
}

static uint8_t *read_all(FILE *f, size_t *len_out) {
    fseek(f, 0, SEEK_END);
    long L = ftell(f);
    if (L < 0) L = 0;
    rewind(f);
    uint8_t *buf = (uint8_t *) malloc((size_t)L);
    size_t got = L ? fread(buf, 1, (size_t)L, f) : 0;
    *len_out = got;
    return buf;
}

static int roundtrip(const uint8_t *data, size_t len, const mpz_t n, const mpz_t d, const mpz_t pq) {
    FILE *fin = tmpfile();      // plaintext in
    FILE *fenc = tmpfile();     // ciphertext out
    FILE *fdec = tmpfile();     // decrypted plaintext out
    if (!fin || !fenc || !fdec) { perror("tmpfile"); return 1; }

    // write input
    if (len) fwrite(data, 1, len, fin);
    rewind(fin);

    // encrypt → fenc
    ss_encrypt_file(fin, fenc, n);
    rewind(fenc);

    // decrypt(fenc) → fdec
    ss_decrypt_file(fenc, fdec, d, pq);
    rewind(fdec);

    // compare
    size_t out_len = 0;
    uint8_t *out = read_all(fdec, &out_len);
    int ok = (out_len == len) && (memcmp(out, data, len) == 0);

    free(out);
    fclose(fin); fclose(fenc); fclose(fdec);
    return ok ? 0 : 2;
}

int main(void) {
    // deterministic RNG so failures are reproducible
    randstate_init(1337);

    // generate a small-but-nontrivial key (fast for tests)
    mpz_t p, q, n, d, pq;
    mpz_inits(p, q, n, d, pq, NULL);
    ss_make_pub(p, q, n, /*nbits=*/256, /*iters=*/25);
    ss_make_priv(d, pq, p, q);

    // figure out a few interesting input lengths around k-1
    size_t k = enc_k_from_n(n);
    size_t cases[] = {0, 1, 5, (k ? k-1 : 1), (k ? k : 2), (k ? (2*(k-1)+3) : 10), 1024};
    uint8_t *rnd = (uint8_t *) malloc(2048);
    for (size_t i = 0; i < 2048; i++) rnd[i] = (uint8_t)(random() & 0xFF);

    int failures = 0;

    // 1) empty file
    failures += roundtrip(NULL, 0, n, d, pq);

    // 2) small ASCII
    const char *msg = "hello world";
    failures += roundtrip((const uint8_t *)msg, strlen(msg), n, d, pq);

    // 3) binary/random at tricky sizes
    for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
        size_t L = cases[i];
        failures += roundtrip(rnd, L > 2048 ? 2048 : L, n, d, pq);
    }

    free(rnd);
    mpz_clears(p, q, n, d, pq, NULL);
    randstate_clear();

    if (failures == 0) {
        printf("ss: ALL ROUNDTRIPS PASSED\n");
        return 0;
    } else {
        printf("ss: ROUNDTRIPS FAILED: %d\n", failures);
        return 1;
    }
}
