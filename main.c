#include <stdio.h>
#include <stdlib.h>

#define COLOR_CHANNELS 3
#define WIDTH 600
#define HEIGHT 400

inline int PixelIndex(int pixel) {
  return pixel * COLOR_CHANNELS;
}

int main() {
  const int screen_size = WIDTH * HEIGHT * COLOR_CHANNELS;
  const int pixel_count = WIDTH * HEIGHT;

  int *screen = malloc(screen_size * sizeof(int));
  if (screen == NULL) {
    perror("Could not allocate memory for screen");
    return 1;
  }

  for (int pixel = 0; pixel < pixel_count; pixel++) {
    int i = pixel * COLOR_CHANNELS;

    if (pixel < 120000) {
      screen[i] = 255;     // R
      screen[i + 1] = 255; // G
      screen[i + 2] = 255; // B
    } else {
      screen[i] = 0;
      screen[i + 1] = 0;
      screen[i + 2] = 0;
    }
  }

  FILE *fp = fopen("image.ppm", "w");
  if (fp == NULL) {
    perror("Could not open image.ppm");
    return 1;
  }
  
  fprintf(fp, "P3\n%d %d\n255\n", WIDTH, HEIGHT);

  for (int pixel = 0; pixel < pixel_count; pixel++) {
    int i = pixel * COLOR_CHANNELS;

    fprintf(fp, "%d %d %d\n", screen[i], screen[i + 1], screen[i + 2]);
  }

  free(screen);
  fclose(fp);

  return 0;
}
