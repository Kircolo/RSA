#pragma once

#include <gmp.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Computes the greatest common divisor of two integers using the Euclidean algorithm
 *
 * @param g: Output parameter to store the computed GCD
 * @param a: First input integer (mpz_t type)
 * @param b: Second input integer (mpz_t type)
 *
 * This function implements the Euclidean algorithm to find the greatest common divisor
 * of two arbitrary precision integers. The result is stored in the output parameter g.
 * The function uses temporary variables to avoid modifying the input parameters.
 *
 * Note: The caller is responsible for initializing the output parameter g before
 * calling this function.
 */
void gcd(mpz_t g, const mpz_t a, const mpz_t b);

/**
 * Computes the modular multiplicative inverse of a modulo n using the Extended Euclidean Algorithm.
 * 
 * @param o Output parameter - stores the modular inverse of a mod n, or 0 if no inverse exists
 * @param a The number to find the inverse of
 * @param n The modulus
 * 
 * @note If gcd(a, n) != 1, then no inverse exists and o is set to 0
 * @note Uses the Extended Euclidean Algorithm to find the inverse
 */
void mod_inverse(mpz_t o, const mpz_t a, const mpz_t n);

/**
 * Computes modular exponentiation: (a^d) mod n using the square-and-multiply algorithm.
 * 
 * @param o Output parameter - stores the result of (a^d) mod n
 * @param a The base
 * @param d The exponent
 * @param n The modulus
 * 
 * @note Uses binary exponentiation for efficient computation
 */
void pow_mod(mpz_t o, const mpz_t a, const mpz_t d, const mpz_t n);

/**
 * Tests if a number is prime using the Miller-Rabin primality test.
 * 
 * @param n The number to test for primality
 * @param iters The number of iterations (witnesses) to test - higher values increase accuracy
 * 
 * @return true if n is probably prime, false if n is definitely composite
 * 
 * @note This is a probabilistic test - more iterations reduce the chance of false positives
 * @note Uses the Miller-Rabin algorithm with random witnesses
 */
bool is_prime(const mpz_t n, uint64_t iters);

/**
 * Generates a random prime number with the specified number of bits.
 * 
 * @param p Output parameter - stores the generated prime number
 * @param bits The number of bits for the generated prime
 * @param iters The number of Miller-Rabin iterations to use for primality testing
 * 
 * @note Uses recursive approach to keep generating random numbers until a prime is found
 * @note Depends on the global 'state' variable for random number generation
 */
void make_prime(mpz_t p, uint64_t bits, uint64_t iters);
