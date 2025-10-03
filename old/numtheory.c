#include "numtheory.h"
#include "randstate.h"

void gcd(mpz_t g, const mpz_t a, const mpz_t b) {
    //mpz inits
    mpz_t a1, b1;
    mpz_inits(a1, b1, NULL);
    mpz_set(a1, a);
    mpz_set(b1, b);

    while (mpz_cmp_ui(b1, 0) != 0) { //while b is not 0
        mpz_set(g, b1); //set g to b
        mpz_mod(b1, a1, b1); //b = a % b
        mpz_set(a1, g); //set a to g
    }
    //clean up
    mpz_clears(a1, b1, NULL);
}

void mod_inverse(mpz_t o, const mpz_t a, const mpz_t n) {
    //mpz inits
    mpz_t r, r1, t, t1, q, temp;
    mpz_inits(r, r1, t, t1, q, temp, NULL);

    //set mpz vals
    mpz_set(r, n);
    mpz_set(r1, a);
    mpz_set_si(t, 0);
    mpz_set_si(t1, 1);

    //while r1 != 0 (also omg this code bit is awful)
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
    //if r > 1 return no inverse
    if (mpz_cmp_si(r, 1) > 0) {
        mpz_set_ui(o, 0);
        mpz_clears(r, r1, t, t1, q, temp, NULL);
        return;
    }

    //if t < 0
    if (mpz_cmp_si(t, 0) < 0) {
        mpz_add(t, t, n);
    }
    //clean up and return t
    mpz_set(o, t);
    mpz_clears(r, r1, t, t1, q, temp, NULL);
}

void pow_mod(mpz_t o, const mpz_t a, const mpz_t d, const mpz_t n) {
    //mpz inits
    mpz_t v;
    mpz_t p;
    mpz_t temp;
    mpz_inits(v, p, temp, NULL);
    mpz_set_ui(v, 1); //set v to 1
    mpz_set(p, a); //set p to a
    mpz_set(temp, d); //set temp to d

    //while d > 0
    while (mpz_cmp_ui(temp, 0) > 0) {
        if (mpz_fdiv_ui(temp, 2) == 1) { //if temp is odd
            mpz_mul(v, v, p); //v * p
            mpz_mod(v, v, n); //v % n
        }
        mpz_mul(p, p, p); //p * p
        mpz_mod(p, p, n); //p % n
        mpz_fdiv_q_ui(temp, temp, 2); //d / 2
    }
    //return v
    mpz_set(o, v); //return v
    mpz_clears(v, p, temp, NULL);
}

bool is_prime(const mpz_t n, uint64_t iters) {
    //manual checks from 0-3
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

    //EVENS ARE NOT PRIME OKAY SO PLEASE PLEASE PLEASWS E RETURN FALSE
    if (mpz_even_p(n) != 0) {
        return false;
    }

    //mpz inits
    uint64_t s = 0;
    mpz_t n1, n2, a, r, two, y;
    mpz_inits(n1, n2, a, r, two, y, NULL);

    mpz_sub_ui(n1, n, 1); //make n-1 variable for convenience
    mpz_set(r, n1); //r = n-1 at first
    mpz_set_ui(two, 2); //convenience

    //if not even, divide til even
    while (mpz_fdiv_ui(r, 2) == 0) {
        mpz_fdiv_q_ui(r, r, 2); //keep dividing til even???
        s++; //counts how many iters happened
    }

    //for i 1 to k
    for (uint64_t i = 0; i < iters; i++) {
        mpz_sub_ui(n2, n, 2); //so we make it start at 2
        mpz_urandomm(a, state, n2); //generate the random num from 0 to n-2
        pow_mod(y, a, r, n);

        if (mpz_cmp_ui(y, 1) && mpz_cmp(y, n1)) {
            uint64_t j = 1;

            while ((j <= s - 1) && mpz_cmp(y, n1)) {
                pow_mod(y, y, two, n);
                if (!mpz_cmp_ui(y, 1)) {
                    mpz_clears(n1, n2, a, r, two, y, NULL);
                    return false;
                }
                j++;
            }

            if (mpz_cmp(y, n1)) {
                mpz_clears(n1, n2, a, r, two, y, NULL);
                return false;
            }
        }
    }
    //clean up
    mpz_clears(n1, n2, a, r, two, y, NULL);
    return true;
}

void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    mpz_urandomb(p, state, bits); //make random number in range 0 - 2^n-1
    if (is_prime(p, iters)
        == false) { //if not prime, call make_prime. stops running when prime is generated
        make_prime(p, bits, iters);
    }
}
