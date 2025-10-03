# ASGN5: RSA Cryptography Implementation

## Problem Statement

This assignment implements a **complete RSA cryptography system** with key generation, encryption, and decryption capabilities. The program uses the GMP (GNU Multiple Precision Arithmetic) library for handling large integers and implements the RSA algorithm with proper mathematical foundations. This demonstrates concepts in cryptography, number theory, and secure communication protocols.

## Key Data Structures & Algorithms

### RSA Cryptography Implementation
- **Key Generation**: Prime number generation and key pair creation
- **Encryption/Decryption**: Modular exponentiation for message transformation
- **Number Theory**: Prime factorization, modular arithmetic, and primality testing
- **Large Integer Arithmetic**: GMP library for arbitrary-precision arithmetic

### Core Cryptographic Functions
- **Prime Generation**: Miller-Rabin primality testing for secure prime generation
- **Key Pair Creation**: Public and private key generation with proper mathematical properties
- **Modular Exponentiation**: Efficient computation of large modular powers
- **File Encryption/Decryption**: Secure transformation of file contents

## Architecture & Design

### Program Structure
```c
// Core cryptographic functions
void ss_make_pub(mpz_t p, mpz_t q, mpz_t n, uint64_t nbits, uint64_t iters);
void ss_make_priv(mpz_t d, mpz_t pq, const mpz_t p, const mpz_t q);
void ss_encrypt(mpz_t c, const mpz_t m, const mpz_t n);
void ss_decrypt(mpz_t m, const mpz_t c, const mpz_t d, const mpz_t pq);

// File operations
void ss_encrypt_file(FILE *infile, FILE *outfile, const mpz_t n);
void ss_decrypt_file(FILE *infile, FILE *outfile, const mpz_t d, const mpz_t pq);

// Key management
void ss_write_pub(const mpz_t n, const char username[], FILE *pbfile);
void ss_write_priv(const mpz_t pq, const mpz_t d, FILE *pvfile);
void ss_read_pub(mpz_t n, char username[], FILE *pbfile);
void ss_read_priv(mpz_t pq, mpz_t d, FILE *pvfile);
```

### Key Design Decisions
- **GMP Integration**: Arbitrary-precision arithmetic for cryptographic security
- **Modular Design**: Separate components for key generation, encryption, and decryption
- **File Format Standards**: Compatible key and message file formats
- **Security Considerations**: Proper key size and primality testing parameters

## Implementation Details

### ss.c - RSA Implementation
**Key Functions:**
- **`ss_make_pub()`**: Generates public key components (p, q, n)
- **`ss_make_priv()`**: Generates private key components (d, pq)
- **`ss_encrypt()`**: Encrypts message using public key
- **`ss_decrypt()`**: Decrypts message using private key
- **`ss_encrypt_file()`**: Encrypts entire file contents
- **`ss_decrypt_file()`**: Decrypts entire file contents

**Mathematical Foundation:**
- **RSA Algorithm**: Based on difficulty of factoring large numbers
- **Modular Arithmetic**: All operations performed modulo n
- **Prime Generation**: Secure prime generation with Miller-Rabin testing
- **Key Derivation**: Mathematical relationship between public and private keys

### numtheory.c - Number Theory Functions
**Key Functions:**
- **`make_prime()`**: Generates large prime numbers
- **`gcd()`**: Computes greatest common divisor
- **`mod_inverse()`**: Computes modular multiplicative inverse
- **`pow_mod()`**: Efficient modular exponentiation

### keygen.c - Key Generation Program
**Key Features:**
- **Command Line Interface**: Flexible key generation parameters
- **Security Parameters**: Configurable key size and primality testing
- **Key Storage**: Secure key file generation and storage
- **User Integration**: Username association with public keys

### encrypt.c/decrypt.c - File Encryption Programs
**Key Features:**
- **File Processing**: Secure encryption/decryption of file contents
- **Format Handling**: Proper handling of binary and text files
- **Error Handling**: Robust error checking and validation
- **Performance**: Efficient processing of large files

## How to Build & Run

### Compilation
```bash
# Compile all RSA programs
make

# Or compile manually (requires GMP library)
gcc -o keygen keygen.c ss.c numtheory.c randstate.c -lgmp
gcc -o encrypt encrypt.c ss.c numtheory.c randstate.c -lgmp
gcc -o decrypt decrypt.c ss.c numtheory.c randstate.c -lgmp
```

### Running the Programs
```bash
# Generate key pair
./keygen -b 1024 -i 50 -n pubkey.txt -d privkey.txt -s username

# Encrypt a file
./encrypt -i plaintext.txt -o ciphertext.txt -n pubkey.txt

# Decrypt a file
./decrypt -i ciphertext.txt -o decrypted.txt -d privkey.txt

# Display help
./keygen -h
./encrypt -h
./decrypt -h
```

### Expected Output
```
# Key generation
Generating RSA keys...
Public key saved to: pubkey.txt
Private key saved to: privkey.txt

# Encryption
Encrypting file: plaintext.txt
Encrypted file saved to: ciphertext.txt

# Decryption
Decrypting file: ciphertext.txt
Decrypted file saved to: decrypted.txt
```

## File Descriptions

| File | Purpose | Key Features |
|------|---------|--------------|
| `ss.c/h` | RSA implementation | Core cryptographic functions |
| `numtheory.c/h` | Number theory | Prime generation and modular arithmetic |
| `randstate.c/h` | Random state | Cryptographic random number generation |
| `keygen.c` | Key generation | RSA key pair creation program |
| `encrypt.c` | File encryption | Encrypt files using public key |
| `decrypt.c` | File decryption | Decrypt files using private key |
| `Makefile` | Build configuration | Compilation rules and dependencies |
| `DESIGN.pdf` | Design document | Detailed cryptographic design |
| `WRITEUP.pdf` | Assignment writeup | Analysis and results documentation |

## Learning Objectives

By completing this assignment, students will understand:

1. **Cryptography Fundamentals**
   - Public-key cryptography principles
   - RSA algorithm and mathematical foundations
   - Security considerations and key management

2. **Number Theory Applications**
   - Prime number generation and testing
   - Modular arithmetic and exponentiation
   - Mathematical properties of RSA

3. **Large Integer Arithmetic**
   - GMP library usage and integration
   - Arbitrary-precision arithmetic
   - Performance considerations for large numbers

4. **Security Programming**
   - Cryptographic key management
   - Secure file handling and I/O
   - Error handling in security applications

## Common Pitfalls and Solutions

### Mathematical Implementation
- **Problem**: Incorrect modular arithmetic
- **Solution**: Careful implementation of modular operations

### Key Generation
- **Problem**: Weak prime generation
- **Solution**: Proper Miller-Rabin testing with sufficient iterations

### File Handling
- **Problem**: Insecure file I/O
- **Solution**: Proper error checking and secure file operations

### Performance Issues
- **Problem**: Slow modular exponentiation
- **Solution**: Efficient algorithms for large number operations

## Advanced Concepts

### Cryptographic Security
- **Key Size Requirements**: Appropriate key sizes for security
- **Prime Generation**: Secure methods for generating large primes
- **Attack Resistance**: Understanding potential attacks on RSA

### Number Theory
- **Miller-Rabin Primality Testing**: Probabilistic primality testing
- **Modular Arithmetic**: Properties and efficient computation
- **Chinese Remainder Theorem**: Mathematical optimization techniques

### Performance Optimization
- **Modular Exponentiation**: Efficient algorithms for large powers
- **Memory Management**: Handling large integers efficiently
- **Algorithm Complexity**: Time and space complexity analysis

## Practical Applications

### Cybersecurity
- **Secure Communication**: Encrypted file transfer and messaging
- **Digital Signatures**: Authentication and non-repudiation
- **Key Management**: Secure key generation and storage

### Software Engineering
- **Cryptographic Libraries**: Foundation for security applications
- **Secure Protocols**: Implementation of secure communication
- **System Security**: Integration of cryptography into applications

### Research and Development
- **Cryptographic Research**: Development of new algorithms
- **Security Analysis**: Vulnerability assessment and testing
- **Performance Optimization**: Improving cryptographic efficiency

## References

- **Applied Cryptography** (Schneier) - Cryptographic protocols and algorithms
- **Introduction to Algorithms** (CLRS) - Algorithm design and analysis
- **Handbook of Applied Cryptography** (Menezes) - Comprehensive cryptography reference
- **The Art of Computer Programming, Volume 2** (Knuth) - Mathematical algorithms

TO USE THIS PROGRAM:  
  
keygen has the following getopt() functions:  
  
-b Specifies the minimum bits needed for the public modulus n  
-i Specifies the number of Miller-Rabin iterations for testing primes (DEFAULT: 50)  
-n pbfile Specifies the public key file (DEFAULT: ss.pub)  
-d pvfile Specifies the private key file (DEFAULT: ss.priv)  
-s Specifies the random seed for the random state initialization (DEFAULT: the seconds since the UNIX epoch, given by time(NULL))  
-v Enables verbose output  
-h Display help message detailing program usage  
  
encrypt has the following getopt() functions:  
  
-i Specifies the input file to encrypt (DEFAULT: stdin)  
-o Specifies the input file to encrypt (DEFAULT: stdout)  
-n Specifies the file containing the public key (DEFAULT: ss.pub)  
-v Enables verbose output  
-h Display help message detailing program usage  
  
decrypt has the following getopt() functions:  
  
-i Specifies the input file to decrypt (DEFAULT: stdin)  
-o Specifies the input file to decrypt (DEFAULT: stdout)  
-n Specifies the file containing the private key (DEFAULT: ss.priv)  
-v Enables verbose output  
-h Display help message detailing program usage  
  
CITATIONS:  
  
All of the mpz functions were taken from the gmp documentation  
A method to generate a random number from range was learned from here:  https://www.geeksforgeeks.org/generating-random-number-range-c/  
  
DELIVERABLES:  
  
decrypt.c  
– Contains the implementation and main() for decrypt  
  
encrypt.c  
– Contains the implementation and main() for decrypt  
  
keygen.c  
– Contains the implementation and main() for decrypt  
  
numtheory.c  
– Contains the implementation of the number theory functions  
  
numtheory.h  
– Interface for number theory functions  
  
randstate.c  
– Contains the implementation of random state for SS library and numtheory functions  
  
randstate.h  
– Interface for initializing and clearing the random state  
  
ss.c  
– Contains the implementation for the SS  library  
  
ss.h  
– Interface for the SS library  
  
README.md  
Makefile  
DESIGN.pdf  
WRITEUP.pdf  
  