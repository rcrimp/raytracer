/*
 * File: ppm.c
 * Author:  Michael Beattie <mike@ethernal.org>
 *
 * $Id: ppm.c,v 1.2 2001/05/22 13:15:43 omnic Exp $
 *
 * Description: PPM output functions.
 *
 */

#include "ppm.h"

void initPPM(int Xsize, int Ysize, FILE *fd) {
  fprintf(fd, "P6\n# %dx%d Raytracer output\n%d %d\n255\n",
          Xsize, Ysize, Xsize, Ysize);
}

void writePPM(RGBColour c, FILE *fd) {

  static int r, g, b;

  r = (int)(c.red   * 255);
  g = (int)(c.green * 255);
  b = (int)(c.blue  * 255);

  if (r < 0) r = 0; if (r > 255) r = 255;
  if (g < 0) g = 0; if (g > 255) g = 255;
  if (b < 0) b = 0; if (b > 255) b = 255;

  fprintf(fd, "%c%c%c", r, g, b);
}

