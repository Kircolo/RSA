#include "numtheory.h"
#include "randstate.h"

void gcd(mpz_t g, const mpz_t a, const mpz_t b) {
    // initialize mpz
    mpz_t a1, b1;
    mpz_inits(a1, b1, NULL);
    mpz_abs(a1, a);
    mpz_abs(b1, b);

    // cases for 0
    if (mpz_cmp_ui(a1, 0) == 0 && mpz_cmp_ui(b1, 0) == 0) {
        mpz_set_ui(g, 0);
        mpz_clears(a1, b1, NULL);
        return;
    }

    // if b is 0, gcd is a
    if (mpz_cmp_ui(b1, 0) == 0) {
        mpz_set(g, a1);
        mpz_clears(a1, b1, NULL);
        return;
    }

    // Euclidean algorithm
    while (mpz_cmp_ui(b1, 0) != 0) {    // while b is not 0
        mpz_set(g, b1);                 // set g to b
        mpz_mod(b1, a1, b1);            // b = a % b
        mpz_set(a1, g);                 // set a to g
    }
    // gcd found; clean up
    mpz_set(g, a1);
    mpz_clears(a1, b1, NULL);
}

void mod_inverse(mpz_t o, const mpz_t a, const mpz_t n) {
    // check n == 0
    if (mpz_sgn(n) == 0) {
        mpz_set_ui(o, 0);
        return;
    }

    // mpz inits
    mpz_t r, r1, t, t1, q, temp;
    mpz_inits(r, r1, t, t1, q, temp, NULL);
    mpz_set(r, n);
    mpz_mod(r1, a, n);  // r1 = a mod n; 'a' into [0, n)
    mpz_set_si(t, 0);
    mpz_set_si(t1, 1);

    // while r1 != 0
    while (mpz_cmp_si(r1, 0)) {
        mpz_fdiv_q(q, r, r1);
        mpz_set(temp, r1);
        mpz_mul(r1, r1, q);
        mpz_sub(r1, r, r1);
        mpz_set(r, temp);
        mpz_set(temp, t1);
        mpz_mul(t1, t1, q);
        mpz_sub(t1, t, t1);
        mpz_set(t, temp);
    }
    
    // if r > 1 return no inverse
    if (mpz_cmp_si(r, 1) != 0) {
        mpz_set_ui(o, 0);
        mpz_clears(r, r1, t, t1, q, temp, NULL);
        return;
    }

    // canonicalize: o in [0, n)
    mpz_mod(o, t, n);
    if (mpz_sgn(o) < 0) mpz_add(o, o, n);

    // // if t < 0 
    // if (mpz_cmp_si(t, 0) < 0) {
    //     mpz_add(t, t, n);
    // }

    // clean up and return t
    // mpz_set(o, t);
    mpz_clears(r, r1, t, t1, q, temp, NULL);
}

void pow_mod(mpz_t o, const mpz_t a, const mpz_t d, const mpz_t n) {
    // n == 1 case
    if (mpz_cmp_ui(n, 1) == 0) {
        mpz_set_ui(o, 0);
        return;
    }

    //mpz inits
    mpz_t v;
    mpz_t p;
    mpz_t temp;
    mpz_inits(v, p, temp, NULL);
    mpz_set_ui(v, 1);
    mpz_mod(p, a, n);    // p = a % n
    mpz_set(temp, d);
    
    
    while (mpz_cmp_ui(temp, 0) > 0) {       // while d > 0
        if (mpz_fdiv_ui(temp, 2) == 1) {    //   if temp is odd
            mpz_mul(v, v, p);               //     v * p
            mpz_mod(v, v, n);               //     v % n
        }
        mpz_mul(p, p, p);                   // p * p
        mpz_mod(p, p, n);                   // p % n
        mpz_fdiv_q_ui(temp, temp, 2);       // d / 2
    }

    // return v and clean up
    mpz_set(o, v);
    mpz_clears(v, p, temp, NULL);
}

bool is_prime(const mpz_t n, uint64_t iters) {
    // manual checks from 0-3
    if (!mpz_cmp_ui(n, 0)) {
        return false;
    }
    if (!mpz_cmp_ui(n, 1)) {
        return false;
    }
    if (!mpz_cmp_ui(n, 2)) {
        return true;
    }
    if (!mpz_cmp_ui(n, 3)) {
        return true;
    }

    // evens are not prime!
    if (mpz_even_p(n) != 0) {
        return false;
    }

    //mpz inits
    uint64_t s = 0;
    mpz_t n1, n2, a, r, two, y;
    mpz_inits(n1, n2, a, r, two, y, NULL);

    mpz_sub_ui(n1, n, 1);   // make n-1 variable for convenience
    mpz_set(r, n1);         // r = n-1 at first
    mpz_set_ui(two, 2);     // convenience

    // if r is even, divide til odd; n-1 = 2^s * r
    while (mpz_fdiv_ui(r, 2) == 0) {
        mpz_fdiv_q_ui(r, r, 2);
        s++; // count how many iters happened
    }

    // for i 1 to k
    for (uint64_t i = 0; i < iters; i++) {
        mpz_sub_ui(n2, n, 3);           // n2 = n - 3 --> we'll sample in [0, n-4]
        mpz_urandomm(a, state, n2);     //generate the random num from 0 to n-4
        mpz_add_ui(a, a, 2);            // shift to [2, n-2]
        pow_mod(y, a, r, n);

        // if (y != 1) and (y != n-1)
        if (mpz_cmp_ui(y, 1) && mpz_cmp(y, n1)) {
            uint64_t j = 1;

            while ((j <= s - 1) && mpz_cmp(y, n1)) {
                pow_mod(y, y, two, n);
                // y == 1 return false
                if (!mpz_cmp_ui(y, 1)) {
                    mpz_clears(n1, n2, a, r, two, y, NULL);
                    return false;
                }
                j++;
            }

            // return false because y != n-1
            if (mpz_cmp(y, n1)) {
                mpz_clears(n1, n2, a, r, two, y, NULL);
                return false;
            }
        }
    }
    // prime! & clean up
    mpz_clears(n1, n2, a, r, two, y, NULL);
    return true;
}

void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    // make random number in range 0 - 2^bits - 1, then force size and oddness
    if (bits < 2) {
        mpz_set_ui(p, 2);
        return;
    }

    do {
        mpz_urandomb(p, state, bits);       // random candidate
        mpz_setbit(p, bits - 1);            // force exact bit-length
        mpz_setbit(p, 0);                   // force odd
    } while (is_prime(p, iters) == false);  // loop until prime
}
