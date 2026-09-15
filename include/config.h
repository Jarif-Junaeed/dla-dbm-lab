#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

#define PI 3.1415926536
#define COLOR_CHANNELS 3

extern size_t PARTICLE_COUNT;
extern size_t MAX_PARTICLE_STEPS;
extern int WIDTH;
extern int HEIGHT;
extern double DEFAULT_RADIUS;
extern double DLA_STICKINESS_FACTOR;

void ConfigLoad(const char *path);

#endif
