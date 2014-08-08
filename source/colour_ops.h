/*
 * File: colour_ops.h
 *
 * Description: Header file for colour operations.
 */

#ifndef _COLOUR_OPS_H_
#define _COLOUR_OPS_H_

/* ----- TYPE DECLARATIONS ------------------------------------------------- */

/* this a record structure for storing colour info. */
typedef struct RGBColour {
  double red, green, blue;
} RGBColour;

/* ----- CONSTANTS --------------------------------------------------------- */

/* a constant black colour */
extern const RGBColour colour_black;


/* ---- FUNCTION HEADERS --------------------------------------------------- */

void colour_display(RGBColour colour);
RGBColour colour_scale(double s, RGBColour b);
void colour_add_to(RGBColour *a, RGBColour b);
RGBColour colour_add(RGBColour a, RGBColour b);
RGBColour colour_multiply(RGBColour a, RGBColour b);

#endif  /* _COLOUR_OPS_H_ */
