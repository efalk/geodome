#ifndef lint
static const char rcsid[] =
	"$Id: dome_struts.c,v 1.4 2005/05/14 02:11:48 efalk Exp $" ;
#endif

/**********
 *
 *
 *	@@@@    @@@   @   @  @@@@@          @@@@  @@@@@  @@@@   @   @  @@@@@
 *	@   @  @   @  @@ @@  @             @        @    @   @  @   @    @
 *	@   @  @   @  @ @ @  @@@            @@@     @    @@@@   @   @    @
 *	@   @  @   @  @ @ @  @                 @    @    @  @   @   @    @
 *	@@@@    @@@   @ @ @  @@@@@  @@@@@  @@@@     @    @   @   @@@     @
 *
 *	DOME_STRUTS - Generate strut cutting and bending list.
 *
 *
 *
 *	Edward A. Falk
 *	efalk@sourceforge.net
 *
 *	November, 2004
 *
 *
 *
 **********/


static const char usage[] =
"\n"
"dome_struts -- generate strut list for a geodesic dome\n"
"\n"
"usage:	dome_struts [options] [domefile [outfile]]\n"
"	-margin l	specify margin beyond bolt-holes (.75/12)\n"
"	-bl l		specify length of bend (.75/12)\n"
"	-at d		angle tolerance, in degrees (2)\n"
"	-lt p		length tolerance, in percent (.1)\n"
"	-scale s	specify scale factor (1.)\n"
"	-in		convert feet to inches (scale factor 12)\n"
"\n"
"See instructions for more details.\n"
;



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include "utils.h"
#include "dome.h"

	float	radius;

static	double	margin = .75/12.;
static	double	bl = .75/12.;
static	double	at = 2.;
static	double	lt = .001;
static	double	scale = 1.;

static	double	bend_allowances(StrutInfo *strut);


#define	streq(a,b)	(strcmp(a,b) == 0)


int
main(int argc, char **argv)
{
	char *ifilename = NULL;
	FILE *ifile = stdin;
	char *ofilename = NULL;
	FILE *ofile = stdout;
	Dome	dome;
	StrutInfo *struts, *sp;
	int	ns, ntotal;
	int	i;
	double	l2;

	for(++argv; --argc > 0; ++argv)
	{
	  if( streq(*argv, "-help") || streq(*argv, "--help") ) {
	    printf(usage);
	    exit(0);
	  }
	  else if( streq(*argv, "-margin") && --argc > 0)
	    margin = atof(*++argv);
	  else if( streq(*argv, "-at") && --argc > 0)
	    at = atof(*++argv);
	  else if( streq(*argv, "-lt") && --argc > 0)
	    lt = atof(*++argv) * .01;
	  else if( streq(*argv, "-in") )
	    scale = 12.;
	  else if( streq(*argv, "-scale") && --argc > 0)
	    scale = atof(*++argv);
	  else if( ifilename == NULL )
	    ifilename = *argv;
	  else if( ofilename == NULL )
	    ofilename = *argv;
	}

	if( ifilename != NULL && (ifile = fopen(ifilename, "r")) == NULL ) {
	  perror(ifilename);
	  exit(3);
	}

	if( ofilename != NULL && (ofile = fopen(ofilename, "w")) == NULL ) {
	  perror(ofilename);
	  exit(4);
	}

	read_dome(&dome, ifile);
	ns = assign_labels(&dome, &struts, at, lt);
	ntotal = 0;
	for(i=0; i < ns; ++i)
	  ntotal += struts[i].count;

	fprintf(ofile, "\n\n%d struts total, %d different lengths\n\n",
	  ntotal, ns);

	fprintf(ofile, "strut  count   length  a0  a1  length2  cut length\n");

	for(i=0, sp = struts; i < ns; ++i, ++sp)
	{
	  l2 = sp->len + bend_allowances(sp);
	  fprintf(ofile, "%5s  %5d  %7.3f %3.0f %3.0f  %7.3f  %7.3f\n",
	    sp->name, sp->count, sp->len * scale,
	    sp->a0, sp->a1, l2 * scale, (l2 + 2*margin) * scale);
	}


	fprintf(ofile,
	  "\n\n\n"
	  "Notes:\n\n"
	  " length:      distance from vertex to vertex\n"
	  " a0, a1:      bend angles at the two ends\n"
	  " length2:     distance between bolt holes (accounts for bends)\n"
	  " cut length:  total strut length, including margins\n"
	  "\nDon't forget to make a few extras\n"
	  );

	exit(0);
}



/**
 * Bending the ends of the pipe will shorten it slightly.  This function
 * returns the pre-bend length of the strut.
 */
static	double
bend_allowances(StrutInfo *strut)
{
	double d;

	/* Simplifying assumption:  bl << length */

	d = 1. - cos(strut->a0 * M_PI/180.);
	d += 1. - cos(strut->a1 * M_PI/180.);
	return bl * d;
}
