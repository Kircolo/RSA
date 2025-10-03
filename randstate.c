#include <stdlib.h>
#include "randstate.h"

gmp_randstate_t state;              //global random state variable

void randstate_init(uint64_t seed) {
    srandom(seed);                  //make random seed
    gmp_randinit_mt(state);         //initialize the state
    gmp_randseed_ui(state, seed);   //set seed value to state
}

void randstate_clear(void) {
    gmp_randclear(state);
}
