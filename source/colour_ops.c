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

RGBColour colour_scale(double s, RGBColour b){
   RGBColour result;
   result.red   = s * b.red;
   result.green = s * b.green;
   result.blue  = s * b.blue;
   return result;
}
RGBColour colour_add(RGBColour a, RGBColour b){
   RGBColour result;
   result.red = a.red + b.red;
   result.green = a.green + b.green;
   result.blue = a.blue + b.blue;
   return result;
}
void colour_add_to(RGBColour *a, RGBColour b){
   a->red += b.red;
   a->green += b.green;
   a->blue += b.blue;
}
RGBColour colour_multiply(RGBColour a, RGBColour b){
   RGBColour result;
   result.red = a.red * b.red;
   result.green = a.green * b.green;
   result.blue = a.blue * b.blue;
   return result;
}

/*RGBColour colour_blend(RGBColour *colours, int colour_count){
   int i;
   RGBColour result;
   result.red = result.green = result.blue = 0;
   for(i = 0; i < colour_count; i++){
      result.red += colours[i].red;
      result.green += colours[i].green;
      result.blue += colours[i].blue;
   }
   result = colour_scale(1.0f / (colour_count), result);
   return result;
   }*/

void colour_display(RGBColour colour) {

  printf("Colour (%1.3lf, %1.3lf, %1.3lf)",
	 colour.red, colour.green, colour.blue);

}
