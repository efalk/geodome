#ifndef lint
static const char rcsid[] =
	"$Id: utils.c,v 1.1 2004/11/12 07:13:22 efalk Exp $" ;
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
#include <stdlib.h>
#include <math.h>
#include <string.h>

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
 * Return a cross product of two vectors.
 */
void
crossprod(const Point *v1, const Point *v2, Point *out)
{
   out->x = v1->y*v2->z - v1->z*v2->y;
   out->y = v1->z*v2->x - v1->x*v2->z;
   out->z = v1->x*v2->y - v1->y*v2->x;
}


/**
 * Return a normalized cross product of two vectors.
 */
void
normcrossprod(const Point *v1, const Point *v2, Point *out)
{
   crossprod(v1, v2, out);
   normalize(out);
}


/**
 * Return a dot product of two vectors.
 */
double
dotprod(const Point *v1, const Point *v2)
{
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
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



typedef struct {
  const float vals[3];
  const char *name;
} ColorDef;

static	ColorDef colordefs[] = {	/* color definitions */
  {{1, 0, 0}, "red"},
  {{0, 1, 0}, "green"},
  {{0, 0, 1}, "blue"},
  {{.8, .8, 0}, "yellow"},
  {{1, 0, 1}, "magenta"},
  {{0, 1, 1}, "cyan"},
  {{1, .5, 0}, "orange"},
  {{0, 0, 0}, "black"},
  {{.647, .164, .164}, "brown"},
};

static	ColorDef *defcolors[] = {		/* default color list */
  colordefs + 0,
  colordefs + 1,
  colordefs + 2,
  colordefs + 3,
  colordefs + 4,
  colordefs + 5,
  colordefs + 6,
  colordefs + 7,
  colordefs + 8,
};

static	ColorDef **colors = defcolors;
static	int	ncolors = NA(defcolors);



/**
 * Parse a "color,color,color,..." list, rebuild colors array
 */
void
parse_colors(char *clist)
{
	char	*ptr, *p2;
	int	i,j, len;
	ColorDef *def;

	ptr = clist;
	for(ncolors = 1; (ptr = strchr(ptr,',')) != NULL; ++ptr, ++ncolors);
	colors = (ColorDef **) malloc(ncolors * sizeof(*colors));

	ptr = clist;
	for(i = 0; i < ncolors; ++i) {
	  p2 = strchr(ptr, ',');
	  len = p2 != NULL ? p2 - ptr : strlen(ptr);
	  colors[i] = NULL;
	  for(j=NA(colordefs), def = colordefs; --j >= 0; ++def) {
	    if( strncasecmp(ptr, def->name, len) == 0 ) {
	      colors[i] = def;
	      break;
	    }
	  }
	  if( colors[i] == NULL ) {
	    if( p2 != NULL ) *p2 = '\0';
	    fprintf(stderr, "color %s not found, options are:\n", ptr);
	    for(j=NA(colordefs), def = colordefs; --j >= 0; ++def)
	      fprintf(stderr, " %s,", def->name);
	    fprintf(stderr, "\n");
	    exit(2);
	  }
	  ptr = p2 + 1;
	}
}


const float *
get_color(int c)
{
	return colors[c % ncolors]->vals;
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
