#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <gmp.h>
#include <string.h>

#include "numtheory.h"
#include "randstate.h"

// Simple ASSERT macro
#define ASSERT_MSG(cond, msg) do { if (!(cond)) { \
    fprintf(stderr, "FAIL: %s\n", msg); return false; } } while (0)

static bool test_gcd(void) {
    printf("[gcd] basic and edge cases...\n");
    mpz_t a,b,g; mpz_inits(a,b,g,NULL);

    // gcd(30,12)=6
    mpz_set_ui(a,30); mpz_set_ui(b,12);
    gcd(g,a,b);
    ASSERT_MSG(mpz_cmp_ui(g,6)==0, "gcd(30,12) != 6");

    // gcd(0,0)=0 (convention). Expect 0.
    mpz_set_ui(a,0); mpz_set_ui(b,0);
    gcd(g,a,b);
    ASSERT_MSG(mpz_cmp_ui(g,0)==0, "gcd(0,0) != 0 (should be 0)");

    // gcd(41,0)=41  ← fails in your current version (b==0 early exit leaves g unset)
    mpz_set_ui(a,41); mpz_set_ui(b,0);
    gcd(g,a,b);
    if (mpz_cmp_ui(g,41)!=0) {
        gmp_fprintf(stderr, "NOTE: Your gcd fails when b==0; got %Zd (expected 41)\n", g);
        mpz_clears(a,b,g,NULL);
        return false;
    }

    // gcd(-42, 56)=14 (non-negative)
    mpz_set_si(a,-42); mpz_set_ui(b,56);
    gcd(g,a,b);
    if (mpz_cmp_ui(g,14)!=0) {
        gmp_fprintf(stderr, "NOTE: Your gcd may not canonicalize sign; got %Zd (expected 14)\n", g);
        mpz_clears(a,b,g,NULL);
        return false;
    }

    mpz_clears(a,b,g,NULL);
    printf("PASS\n");
    return true;
}

static bool test_pow_mod(void) {
    printf("[pow_mod] exponent/base/n edge cases...\n");
    mpz_t a,d,n,o; mpz_inits(a,d,n,o,NULL);

    // a^0 mod n == 1 mod n for n>1
    mpz_set_ui(a,5); mpz_set_ui(d,0); mpz_set_ui(n,7);
    pow_mod(o,a,d,n);
    if (mpz_cmp_ui(o,1)!=0) {
        gmp_fprintf(stderr, "NOTE: a^0 mod n should be 1; got %Zd\n", o);
        mpz_clears(a,d,n,o,NULL); return false;
    }

    // n==1 => everything mod 1 is 0
    mpz_set_ui(a,123456); mpz_set_ui(d,789); mpz_set_ui(n,1);
    pow_mod(o,a,d,n);
    if (mpz_cmp_ui(o,0)!=0) {
        gmp_fprintf(stderr, "NOTE: mod 1 should be 0; got %Zd\n", o);
        mpz_clears(a,d,n,o,NULL); return false;
    }

    mpz_clears(a,d,n,o,NULL);
    printf("PASS\n");
    return true;
}

static bool test_mod_inverse(void) {
    printf("[mod_inverse] invertible & non-invertible, negative a...\n");
    mpz_t a,n,o; mpz_inits(a,n,o,NULL);

    // invertible: inv(3,11)=4
    mpz_set_ui(a,3); mpz_set_ui(n,11);
    mod_inverse(o,a,n);
    if (mpz_cmp_ui(o,4)!=0) {
        gmp_fprintf(stderr, "NOTE: inv(3,11) expected 4; got %Zd\n", o);
        mpz_clears(a,n,o,NULL); return false;
    }

    // non-invertible: inv(2,4)=0
    mpz_set_ui(a,2); mpz_set_ui(n,4);
    mod_inverse(o,a,n);
    if (mpz_cmp_ui(o,0)!=0) {
        gmp_fprintf(stderr, "NOTE: inv(2,4) should be 0; got %Zd\n", o);
        mpz_clears(a,n,o,NULL); return false;
    }

    // negative 'a': inv(-3,11) == inv(8,11) == 7  ← often fails if you don't reduce first/canonicalize
    mpz_set_si(a,-3); mpz_set_ui(n,11);
    mod_inverse(o,a,n);
    if (mpz_cmp_ui(o,7)!=0) {
        gmp_fprintf(stderr, "NOTE: inv(-3,11) should be 7 (canonical in [0,n)); got %Zd\n", o);
        mpz_clears(a,n,o,NULL); return false;
    }

    mpz_clears(a,n,o,NULL);
    printf("PASS\n");
    return true;
}

// Expose Miller–Rabin witness bug where a is sampled in [0, n-3] (missing +2),
// which can pick a=0 or a=1, causing false negative for primes.
// We scan a range of seeds to see if any seed produces a false negative for a small prime.
static bool test_is_prime_flaky(void) {
    printf("[is_prime] scanning seeds to find false negative due to bad witness range...\n");
    mpz_t n; mpz_init_set_ui(n,5); // small prime; bad witnesses 0 or 1 are likely with small modulus
    const uint64_t iters = 1;

    for (uint64_t seed = 1; seed <= 10000; seed++) {
        randstate_init(seed);
        bool ok = is_prime(n, iters);
        randstate_clear();
        if (!ok) {
            printf("FOUND: is_prime(5,1) returned false with seed=%" PRIu64 "\n", seed);
            mpz_clear(n);
            printf("FAIL (expected true). This indicates your witness range likely includes 0 or 1.\n");
            return false;
        }
    }
    mpz_clear(n);
    printf("No false negative found in seeds 1..10000 (still likely with other seeds).\n");
    printf("PASS\n");
    return true;
}

// Check that make_prime returns exactly 'bits' bits (top bit set) and an odd number.
// With the current implementation, bit length may be < bits because top bit isn't forced,
// and recursion can hide that. We'll scan seeds until we catch a short prime.
static bool test_make_prime_bitlen(void) {
    printf("[make_prime] scanning seeds to catch wrong bit-length or evenness...\n");
    const uint64_t bits = 16;   // small to keep scan fast
    const uint64_t iters = 25;  // reasonable MR rounds
    mpz_t p; mpz_init(p);

    for (uint64_t seed = 1; seed <= 10000; seed++) {
        randstate_init(seed);
        make_prime(p, bits, iters);
        randstate_clear();

        size_t got_bits = mpz_sizeinbase(p, 2);
        if (got_bits != bits) {
            gmp_printf("FOUND: make_prime produced %Zu-bit prime (expected %llu) with seed=%llu: %Zd\n",
                       got_bits, (unsigned long long)bits, (unsigned long long)seed, p);
            mpz_clear(p);
            printf("FAIL (bit-length not enforced)\n");
            return false;
        }
        if (mpz_even_p(p)) {
            gmp_printf("FOUND: make_prime produced even number with seed=%llu: %Zd\n",
                       (unsigned long long)seed, p);
            mpz_clear(p);
            printf("FAIL (even prime other than 2)\n");
            return false;
        }
    }
    mpz_clear(p);
    printf("No bit-length/oddness issue observed in seeds 1..10000 (but the bug still exists statistically).\n");
    printf("PASS\n");
    return true;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    int failures = 0;
    if (!test_gcd()) failures++;
    if (!test_pow_mod()) failures++;
    if (!test_mod_inverse()) failures++;
    if (!test_is_prime_flaky()) failures++;
    if (!test_make_prime_bitlen()) failures++;
    if (failures == 0) {
        printf("\nALL TESTS PASSED\n");
        return 0;
    }
    printf("\nTESTS FAILED: %d\n", failures);
    return 1;
}
