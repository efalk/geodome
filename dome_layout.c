#ifndef lint
static const char rcsid[] =
	"$Id: dome_layout.c,v 1.1 2004/11/12 07:13:22 efalk Exp $" ;
#endif

/**********
 *
 *
 *	@       @@@   @   @   @@@   @   @  @@@@@  
 *	@      @   @   @ @   @   @  @   @    @    
 *	@      @@@@@    @    @   @  @   @    @    
 *	@      @   @    @    @   @  @   @    @    
 *	@@@@@  @   @    @     @@@    @@@     @    
 *
 *	DOME_LAYOUT - Generate postscript layout diagram
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
"dome_layout -- generate layout diagram for geodesic dome\n"
"\n"
"usage:	dome_struts [options] [domefile [postscriptfile]]\n"
"	-at d		  angle tolerance, in degrees (2)\n"
"	-lt p		  length tolerance, in percent (.1)\n"
"	-paper WxH	  page size (8.5 x 11.5)\n"
"	-margins l,r,t.b  margins (1,.5,.5,.5)\n"
"	-textheight h	  set text height, points (10)\n"
#ifdef	TODO
"	-sheets n	  specify number of sheets to use (1)\n"
#endif
"	-explode	  exploded diagram\n"
"	-color		  generate color-coded struts\n"
"	-colors c,c,c..	  specify the exact colors to use\n"
"\n"
"In practice, 8.5x11 paper should be sufficient.\n"
"See instructions for more details.\n"
;



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "utils.h"
#include "dome.h"

	float	radius;

static	float	at = 2.;
static	float	lt = .001;
static	float	wid = 8.5, hgt = 11;
static	float	lm = 1., rm = .5, tm = .5, bm = .5;
static	float	textheight = 10.;
static	int	sheets = 1;
static	int	explode = 0;
static	int	color = 0;

static	char	*ifilename = NULL;
static	FILE	*ifile;
static	char	*ofilename = NULL;
static	FILE	*ofile;


static	char	c0;
static	float	xmin, ymin, xmax, ymax;

static	float	sx,sy, tx,ty;		/* dome => postscript transform */


static	void	write_prolog(void);
static	void	start_page(int pagenum);
static	void	end_page(int pagenum);
static	void	write_trailer(void);
static	void	draw_edge(Dome *, Edge *);
static	void	project_point(Point *);
static	void	adjust_edge(Point *p0, Point *p1, double len);
static	void	parse_colors(char *colors);


#define	streq(a,b)	(strcmp(a,b) == 0)


int
main(int argc, char **argv)
{
	Dome	dome;
	StrutInfo *struts;
	Edge	*edge;
	int	ns;
	int	i;

	ifile = stdin;
	ofile = stdout;

	for(++argv; --argc > 0; ++argv)
	{
	  if( streq(*argv, "-help") || streq(*argv, "--help") ) {
	    printf(usage);
	    exit(0);
	  }
	  else if( streq(*argv, "-at") && --argc > 0)
	    at = atof(*++argv);
	  else if( streq(*argv, "-lt") && --argc > 0)
	    lt = atof(*++argv) * .01;
	  else if( streq(*argv, "-textheight") && --argc > 0)
	    textheight = atof(*++argv);
	  else if( streq(*argv, "-paper") && --argc > 0)
	    sscanf(*++argv, "%gx%g", &wid, &hgt);
	  else if( streq(*argv, "-margin") && --argc > 0)
	    sscanf(*++argv, "%g,%g,%g,%g", &lm, &rm, &tm, &bm);
	  else if( streq(*argv, "-sheets") && --argc > 0)
	    sheets = atoi(*++argv);
	  else if( streq(*argv, "-explode") )
	    explode = 1;
	  else if( streq(*argv, "-color") )
	    color = 1;
	  else if( streq(*argv, "-colors") && --argc > 0)
	    parse_colors( *++argv );
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
	c0 = struts[0].name[0];

	/* Guess radius from the highest point on the dome */
	radius = dome_max(&dome);

	/* Find the bounding box of the projected vertices */
	xmin = xmax = ymin = ymax = 0.;
	for(i=0; i < dome.nvert; ++i)
	{
	  Point pt = dome.vertices[i];
#ifdef	COMMENT
	  printf("(%g,%g,%g) => ", pt.x, pt.y, pt.z);
#endif	/* COMMENT */
	  project_point(&pt);
#ifdef	COMMENT
	  printf("(%g,%g,%g)\n", pt.x, pt.y, pt.z);
#endif	/* COMMENT */
	  xmin = MIN(xmin, pt.x); xmax = MAX(xmax, pt.x);
	  ymin = MIN(ymin, pt.y); ymax = MAX(ymax, pt.y);
	}

	/* Compute transformation matrix */
	/* TODO: multi-sheet */

	if( xmax > xmin && ymax > ymin )
	{
	  float w, h;
	  w = (wid - lm - rm) * 72;	/* working space in ps points */
	  h = (hgt - tm - bm) * 72;

	  sx = w / (xmax - xmin);
	  sy = h / (ymax - ymin);

	  sx = sy = MIN(sx, sy);

	  tx = lm*72 + w/2;
	  ty = bm*72 + h/2;
	}

	write_prolog();
	start_page(1);

	for( i=0, edge = dome.edges; i < dome.nedge; ++i, ++edge )
	  draw_edge(&dome, edge);

	end_page(1);
	write_trailer();

	exit(0);
}



static void
write_prolog(void)
{
	fprintf(ofile,
	  "%%!PS-Adobe-3.0\n"
	  "%%%%BoundingBox: %g %g %g %g\n"
	  "%%%%Pages: %d\n"
	  "%%%%Creator: geodome\n"
	  "%%%%DocumentFonts: (atend)\n"
	  "%%%%DocumentNeedsFonts: (atend)\n"
	  "%%%%EndComments:\n\n",
	  0.,0., wid*72., hgt*72., sheets) ;

	fprintf(ofile, "%%%%BeginProlog\n\n") ;

	fprintf(ofile,
	  "/stringbounds           %% (string) => wid hgt\n"
	  "{\n"
	  "  gsave 0 0 moveto true charpath flattenpath pathbbox grestore\n"
	  "  3 1 roll sub 3 2 roll exch sub exch\n"
	  "} bind def\n\n"
	  "/rect		%% wid hgt =>\n"
	  "{\n"
	  "  currentpoint newpath moveto\n"
	  "  1 index 0 rlineto 0 exch rlineto neg 0 rlineto closepath fill\n"
	  "} bind def\n\n"
	  "/ctrTxt		%% (string) =>\n"
	  "{\n"
	  "  dup stringbounds 2 copy 2 div neg exch 2 div neg exch rmoveto\n"
	  "  1 setgray gsave rect grestore 0 setgray\n"
	  "  show\n"
	  "} bind def\n\n");

	fprintf(ofile, "%%%%EndProlog\n\n") ;
}


static void
start_page(int pagenum)
{
	fprintf(ofile, "%%%%Page: \"%d\" %d\n\n", pagenum, pagenum) ;
#ifdef	COMMENT
	fprintf(ofile, "%%%%BeginPaperSize: %s\n", "letter") ;
	fprintf(ofile, "%%%%EndPaperSize\n\n") ;
#endif	/* COMMENT */
	fprintf(ofile, "0 setlinejoin\n0 setlinecap\n[] 0 setdash\n") ;
	fprintf(ofile, "1 setlinewidth\n") ;
	fprintf(ofile, "/Times-Roman findfont %g scalefont setfont\n\n",
	  textheight);
}


static void
end_page(int pagenum)
{
	fprintf(ofile, "showpage \n\n");
	fprintf(ofile, "%%%%EndPage: \"%d\" %d\n\n", pagenum, pagenum) ;
}


static void
write_trailer()
{
	fprintf(ofile, "%%%%Trailer\n") ;
	fprintf(ofile, "%%%%EOF\n") ;
}


typedef struct {
  const char *vals;
  const char *name;
} ColorDef;

static	ColorDef colordefs[] = {	/* color definitions */
  {"1 0 0", "red"},
  {"0 1 0", "green"},
  {"0 0 1", "blue"},
  {".8 .8 0", "yellow"},
  {"1 .5 0", "orange"},
  {"1 0 1", "magenta"},
  {"0 1 1", "cyan"},
  {"0 0 0", "black"},
  {".647 .164 .164", "brown"},
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
 * Write out an edge in postscript.  Generate a transformation that
 * flattens it out.
 */
static void
draw_edge(Dome *dome, Edge *edge)
{
	Point	p0, p1;
	float	lx, ly;

	p0 = dome->vertices[edge->v0];
	p1 = dome->vertices[edge->v1];

	project_point(&p0);
	project_point(&p1);

	/* We've transformed the end points, but this will cause increasing
	 * distortion as we move away from the center, so now we shorten
	 * the edge back to its original length.  This will cause all the
	 * endpoints to break away from each other, but this is a desirable
	 * side-effect, since we're going for an exploded view.
	 */
	if( explode )
	  adjust_edge(&p0, &p1, edge->len);

	/* Convert to postscript coords */
	p0.x = p0.x * sx + tx;  p0.y = p0.y * sy + ty;
	p1.x = p1.x * sx + tx;  p1.y = p1.y * sy + ty;

	fprintf(ofile, "%% strut %s\n", edge->name);
	if( color ) {
	  int c = edge->name[0] - c0;
	  fprintf(ofile, "%s setrgbcolor\n",
	    colors[c % ncolors]->vals);
	}
	fprintf(ofile, "newpath %g %g moveto %g %g lineto stroke\n",
	  p0.x, p0.y, p1.x, p1.y);
	if( color )
	  fprintf(ofile, "0 setgray\n");

	lx = (p0.x + p1.x) / 2;
	ly = (p0.y + p1.y) / 2;
	fprintf(ofile, "%g %g moveto (%s) ctrTxt\n\n", lx, ly, edge->name);

}


/**
 * Take a point, and project it to the "flattened" x-y coordinate system.
 */
static void
project_point(Point *p)
{
	Point	p0, p1;
	static Point zero = {0.,0.,0.};
	double	a;
	double	dist;

	/* How do we flatten this?  Find the circumfrential distance
	 * from the top of the dome to the vertex, project the
	 * vertex that far from the center.
	 */

	/* Find the angle described by this point.  Since we're taking
	 * the angle relative to a vertical vector (0,0,1), this is
	 * trivially computed from the z component of the normalized
	 * vector.
	 */

	p0 = *p;
	normalize_vertex(&p0, 1., 1.);
	a = acos(p0.z);
	dist = a * radius;

	p1.x = p->x; p1.y = p->y; p1.z = 0.;

	/* See projectPointOnLine() documentation for more info.  In
	 * essense, what this call does is to project 0 onto the 0-p1
	 * line.  Normally, this lands back at 0, but then the 'dist'
	 * parameter offsets it to where we want.
	 */
	if( !projectPointOnLine(&zero, &p1, &zero, dist, p) )
	  p->z = 0.;
}



/**
 * Adjust the two endpoints so that they're the specified distance
 * apart.
 */
static void
adjust_edge(Point *p0, Point *p1, double len)
{
	double d = ptDist(p0, p1);

	d = (d - len) / 2;		/* adjustment factor */

	projectPointOnLine(p0, p1, p0, d, p0);
	projectPointOnLine(p1, p0, p1, d, p1);
}


/**
 * Parse a "color,color,color,..." list, rebuild colors array
 */
static	void
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
