#ifndef lint
static const char rcsid[] =
	"$Id$" ;
#endif

/**********
 *
 *	@   @  @@@@@   @@@   @       @@@@  
 *	@   @    @      @    @      @      
 *	@   @    @      @    @       @@@   
 *	@   @    @      @    @          @  
 *	 @@@     @     @@@   @@@@@  @@@@   
 *
 *	UTILS -- various small utilities.
 *
 *	Brief description of the program, what it does, how it is used.
 *
 *	Routines provided here:
 *
 *
 *	Edward A. Falk
 *	falk@efalk.org
 *
 *	June, 2004
 *
 *
 *
 **********/

#include <stdio.h>
#include <math.h>

#include "utils.h"


/**
 * Add two 3-d points together.
 */
void
ptAdd(const Point *a, const Point *b, Point *out)
{
	out->x = a->x + b->x;
	out->y = a->y + b->y;
	out->z = a->z + b->z;
}


/**
 * Subtract two 3-d points.
 */
void
ptSub(const Point *a, const Point *b, Point *out)
{
	out->x = a->x - b->x;
	out->y = a->y - b->y;
	out->z = a->z - b->z;
}


/**
 * Multiply a point by a scalar.
 */
void
ptMul(const Point *a, double m, Point *out)
{
	out->x = a->x * m;
	out->y = a->y * m;
	out->z = a->z * m;
}


/**
 * Return the distance between two points.
 */
double
ptDist(const Point *a, const Point *b)
{
	double	dx = a->x - b->x;
	double	dy = a->y - b->y;
	double	dz = a->z - b->z;
	return sqrt(dx*dx + dy*dy + dz*dz);
}


/**
 * Return the length of a vector.
 */
double
len(const Point *v)
{
	return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}


/**
 * Return the length of a vector on the XY plane.
 */
double
lenXY(const Point *v)
{
	return sqrt(v->x * v->x + v->y * v->y);
}


/**
 * Normalize a vector to unit length.
 */
void
normalize(Point *v)
{
   float d = sqrt(v->x*v->x + v->y*v->y + v->z*v->z);
   if (d == 0.0) {
      error("zero length vector");
      return;
   }
   v->x /= d; v->y /= d; v->z /= d;
}


/**
 * Return a normalized cross product of two vectors.
 */
void
normcrossprod(const Point *v1, const Point *v2, Point *out)
{
   out->x = v1->y*v2->z - v1->z*v2->y;
   out->y = v1->z*v2->x - v1->x*v2->z;
   out->z = v1->x*v2->y - v1->y*v2->x;
   normalize(out);
}



/**
 * Return the average of three points.
 */
void
ptAvg3(const Point *a, const Point *b, const Point *c, Point *out)
{
	out->x = (a->x + b->x + c->x) / 3;
	out->y = (a->y + b->y + c->y) / 3;
	out->z = (a->z + b->z + c->z) / 3;
}


/**
 * Create a new point at some proportional distance between two given points.
 */
void
ptInterp(const Point *a, const Point *b, double f, Point *out)
{
	double	f1 = 1. - f;
	out->x = a->x * f1 + b->x * f;
	out->y = a->y * f1 + b->y * f;
	out->z = a->z * f1 + b->z * f;
}



void
assfail(const char *e, const char *file, int line)
{
	fprintf(stderr, "yak!  assertion \"%s\" failed, %s:%d\n",
	  e, file, line);
}


void
error(char *msg)
{
	fprintf(stderr, "%s\n", msg);
}
