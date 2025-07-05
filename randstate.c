#include "randstate.h"

#include <stdint.h>
#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>

//global random state variable
gmp_randstate_t state;

//initialize the random state
void randstate_init(uint64_t seed) {
    srandom(seed); //make random seed
    gmp_randinit_mt(state); //initialize the state
    gmp_randseed_ui(state, seed); //set seed value to state
}

//clear memory of randstate
void randstate_clear(void) {
    gmp_randclear(state);
}
