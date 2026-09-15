#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "ini.h"

size_t PARTICLE_COUNT = 1000;
size_t MAX_PARTICLE_STEPS = 1000;
int WIDTH = 600;
int HEIGHT = 400;
double DEFAULT_RADIUS = 10.00;
double DLA_STICKINESS_FACTOR = 0.1;

#define MATCH(n) (strcmp(name, n) == 0)

static int ConfigHandler(void *user, const char *section, const char *name, const char *value) {
  (void)user;
  (void)section;

  if (MATCH("particle_count")) {
    PARTICLE_COUNT = (size_t)atoi(value);
  } else if (MATCH("max_particle_steps")) {
    MAX_PARTICLE_STEPS = (size_t)atoi(value);
  } else if (MATCH("width")) {
    WIDTH = atoi(value);
  } else if (MATCH("height")) {
    HEIGHT = atoi(value);
  } else if (MATCH("default_radius")) {
    DEFAULT_RADIUS = atof(value);
  } else if (MATCH("stickiness_factor")) {
    DLA_STICKINESS_FACTOR = atof(value);
  } else {
    return 0; // unknown key
  }

  return 1;
}

void ConfigLoad(const char *path) {
  ini_parse(path, ConfigHandler, NULL);
}
