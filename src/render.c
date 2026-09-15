#define _POSIX_C_SOURCE 200809L

#include "render.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "config.h"

int Render(bool *cluster) {
  const size_t screen_size = WIDTH * HEIGHT * COLOR_CHANNELS;
  const size_t pixel_count = WIDTH * HEIGHT;

  uint8_t *screen = calloc(screen_size, sizeof(uint8_t));
  if (screen == NULL) {
    perror("Could not allocate memory for screen");
    return 1;
  }

  for (size_t pixel = 0; pixel < pixel_count; pixel++) {
    size_t i = pixel * COLOR_CHANNELS;

    if (cluster[pixel] == 0) {
      screen[i] = 255;     // R
      screen[i + 1] = 255; // G
      screen[i + 2] = 255; // B
    } else {
      screen[i] = 0;     // R
      screen[i + 1] = 0; // G
      screen[i + 2] = 0; // B
    }
  }

  if (mkdir("output", 0755) == -1 && errno != EEXIST) {
    perror("mkdir");
    free(screen);
    return 1;
  }

  FILE *fp = fopen("output/image.ppm", "w");
  if (fp == NULL) {
    perror("Could not open image.ppm");
    free(screen);
    return 1;
  }

  fprintf(fp, "P3\n%d %d\n255\n", WIDTH, HEIGHT);

  for (size_t pixel = 0; pixel < pixel_count; pixel++) {
    int i = pixel * COLOR_CHANNELS;

    fprintf(fp, "%d %d %d\n", screen[i], screen[i + 1], screen[i + 2]);
  }

  free(screen);
  fclose(fp);

  return 0;
}

int AnimateCluster(struct clusterDeltas *cluster_deltas) {
  const size_t screen_size = WIDTH * HEIGHT * COLOR_CHANNELS;

  uint8_t *screen = calloc(screen_size, sizeof(uint8_t));
  if (screen == NULL) {
    perror("Could not allocate memory for screen");
    return 1;
  }

  char command[1024];

  snprintf(command, sizeof(command),
           "ffmpeg -y "
           "-f rawvideo "
           "-pixel_format rgb24 "
           "-video_size %zux%zu "
           "-framerate 120 "
           "-i - "
           "-c:v libx264 "
           "-pix_fmt yuv420p "
           "output/animation.mp4",
           (size_t)WIDTH, (size_t)HEIGHT);

  FILE *ffmpeg = popen(command, "w");

  if (ffmpeg == NULL) {
    perror("Could not start ffmpeg");
    free(screen);
    return 1;
  }

  for (size_t frame = 0; frame < cluster_deltas->m_used_capacity; frame++) {
    size_t src = cluster_deltas->m_cluster_deltas[frame].src * COLOR_CHANNELS;
    size_t dest = (cluster_deltas->m_cluster_deltas[frame].dest == SIZE_MAX)
                      ? SIZE_MAX
                      : cluster_deltas->m_cluster_deltas[frame].dest * COLOR_CHANNELS;

    if (src == dest) {
      screen[src] = 255;
      screen[src + 1] = 255;
      screen[src + 2] = 255;
    } else if (dest == SIZE_MAX) {
      // Remove Particle from src
      screen[src] = 0;
      screen[src + 1] = 0;
      screen[src + 2] = 0;
    } else {
      // Remove Particle from src
      screen[src] = 0;
      screen[src + 1] = 0;
      screen[src + 2] = 0;

      // Add Particle at dest
      screen[dest] = 255;
      screen[dest + 1] = 255;
      screen[dest + 2] = 255;
    }

    fwrite(screen, 1, screen_size, ffmpeg);
  }

  pclose(ffmpeg);
  free(screen);
  return 0;
}
