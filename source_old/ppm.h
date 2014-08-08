/*
 * File: ppm.h
 * Author: Michael Beattie <mike@ethernal.org>
 *
 * $Id: ppm.h,v 1.1 2001/05/22 13:11:57 omnic Exp $
 *
 * Description: Headers for outputting ppm's
 *
 */

#ifndef _PPM_H
#define _PPM_H


#include "colour_ops.h"
#include "stdio.h"


/* main prototypes */
extern void initPPM(int Xsize, int Ysize, FILE *fd);
extern void writePPM(RGBColour col, FILE *fd);


#endif /* _PPM_H */
