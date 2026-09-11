#define _POSIX_C_SOURCE 200809L

#include "rng.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <sys/types.h>

void InitRNG(pcg32_random_t *rng) {
  uint64_t seed;
  uint64_t sequence;
  unsigned char random_buffer[8];
  ssize_t seed_n = getrandom(random_buffer, sizeof(random_buffer), 0);
  if (seed_n != sizeof(random_buffer)) {
    perror("CreateRNG() failed, unable to get seed");
    exit(1);
  }
  memcpy(&seed, random_buffer, sizeof(seed));

  ssize_t sequence_n = getrandom(random_buffer, sizeof(random_buffer), 0);
  if (sequence_n != sizeof(random_buffer)) {
    perror("CreateRNG() failed, unable to get sequence");
    exit(1);
  }
  memcpy(&sequence, random_buffer, sizeof(sequence));

  pcg32_srandom_r(rng, seed, sequence);
}
