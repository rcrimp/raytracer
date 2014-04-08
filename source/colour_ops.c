

/* ----- INCLUDES ---------------------------------------------------------- */

#include "colour_ops.h"
#include <stdio.h>


/* ----- TYPE DECLARATIONS ------------------------------------------------- */

/* this a record structure for storing colour info. */
/*
typedef struct RGBColour {
  double red, green, blue;
} RGBColour;
*/


/* ----- CONSTANTS --------------------------------------------------------- */

/* a constant black colour */
const RGBColour colour_black = {0.0f, 0.0f, 0.0f};


/* ---- FUNCTIONS ---------------------------------------------------------- */

void colour_display(RGBColour colour) {

  printf("Colour (%1.3lf, %1.3lf, %1.3lf)",
	 colour.red, colour.green, colour.blue);

}

RGBColour colour_scale(double s, RGBColour b){
  RGBColour result;
  result.red   = s * b.red;
  result.green = s * b.green;
  result.blue  = s * b.blue;
  return result;
}
