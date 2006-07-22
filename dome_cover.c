#ifndef lint
static const char rcsid[] =
	"$Id: dome_cover.c,v 1.5 2005/06/04 07:53:25 efalk Exp $" ;
#endif

/**********
 *
 *
 *	 @@@    @@@   @   @  @@@@@  @@@@
 *	@      @   @  @   @  @      @   @
 *	@      @   @  @   @  @@@    @@@@
 *	@      @   @   @ @   @      @  @
 *	 @@@    @@@     @    @@@@@  @   @
 *
 *	DOME_COVER - Generate canvas layout and cutting list.
 *
 *
 *	Edward A. Falk
 *	efalk@sourceforge.net
 *
 *	May, 2005
 *
 *
 *
 **********/


static const char usage[] =
"\n"
"dome_cover -- generate cover layout for geodesic dome\n"
"\n"
"usage:	dome_cover [options] [domefile [outfile]]\n"
"	-diam d		  specify pipe diameter (.75/12)\n"
"	-at d		  angle tolerance, in degrees (2)\n"
"	-lt p		  length tolerance, in percent (.1)\n"
"	-scale s	  specify scale factor (1.)\n"
"	-in		  convert feet to inches (scale factor 12)\n"
"	-paper WxH	  page size (8.5 x 11.5)\n"
"	-margins l,r,t.b  margins (1,.5,.5,.5)\n"
"	-textheight h	  set text height, points (10)\n"
#ifdef	TODO
"	-sheets n	  specify number of sheets to use (1)\n"
#endif
"	-explode	  exploded diagram\n"
"	-color		  generate color-coded faces\n"
"	-colors c,c,c..	  specify the exact colors to use\n"
"\n"
"In practice, 8.5x11 paper should be sufficient.\n"
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

static	double	diam = .75/12.;
static	double	at = 2.;
static	double	lt = .001;
static	double	scale = 1.;
static	float	wid = 8.5, hgt = 11;
static	float	lm = 1., rm = .5, tm = .5, bm = .5;
static	float	textheight = 10.;
static	int	sheets = 1;
static	int	explode = 0;
static	int	color = 0;

static	FILE *ofile;


/* Describe one group of identical faces. */
typedef	struct {
	  char *name;
	  StrutInfo *s1, *s2, *s3;
	  int count;
	} FaceInfo;


static	int	assign_faces(const Dome *dome, FaceInfo **info, double lt);
static	void	compute_face(const Dome *dome, const int *face, FaceInfo *info);
static	int	match_face(const FaceInfo *a, const FaceInfo *b);
static	int	facesort(const void *aa, const void *bb);
static	FaceInfo *findFace(FaceInfo *list, int, const Dome *, int face);
static	char	c0;
static	float	xmin, ymin, xmax, ymax;
static	StrutInfo *struts;
static	int	ns;
static	FaceInfo *faces;
static	int	nf, ftotal;

static	float	sx,sy, tx,ty;		/* dome => postscript transform */

static	void	write_prolog(void);
static	void	start_page(int pagenum);
static	void	end_page(int pagenum);
static	void	write_trailer();
static	void	draw_face(Dome *dome, int face, FaceInfo *, int nf);
static	void	draw_single_face(Dome *dome, FaceInfo *,
			double x, double y, double w, double h);
static	void	adjust_face(Point *p0, Point *p1, Point *p2,
			Dome *dome, int face);


#define	streq(a,b)	(strcmp(a,b) == 0)
#define	max(a,b)	((a)>(b)?(a):(b))
#define	min(a,b)	((a)<(b)?(a):(b))


int
main(int argc, char **argv)
{
	char *ifilename = NULL;
	FILE *ifile = stdin;
	char *ofilename = NULL;
	Dome	dome;
	FaceInfo *fp;
	int	i;
	int	pagenum = 0;
	int	strut_page;
	static const Point	p0 = {0.,0.,0.};

	ofile = stdout;

	for(++argv; --argc > 0; ++argv)
	{
	  if( streq(*argv, "-help") || streq(*argv, "--help") ) {
	    printf(usage);
	    exit(0);
	  }
	  else if( streq(*argv, "-diam") && --argc > 0)
	    diam = atof(*++argv);
	  else if( streq(*argv, "-at") && --argc > 0)
	    at = atof(*++argv);
	  else if( streq(*argv, "-lt") && --argc > 0)
	    lt = atof(*++argv) * .01;
	  else if( streq(*argv, "-in") )
	    scale = 12.;
	  else if( streq(*argv, "-scale") && --argc > 0)
	    scale = atof(*++argv);
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

	/* Convert dimensions from inches to postscript points */
	wid *= 72;
	hgt *= 72;
	lm *= 72;
	rm *= 72;
	tm *= 72;
	bm *= 72;

	read_dome(&dome, ifile);

	/* Increase the dome radius slightly, to account for the
	 * pipe radius
	 */
	for(i=0; i < dome.nvert; ++i) {
	  projectPointOnLine(&p0, &dome.vertices[i], &dome.vertices[i],
	    diam*.5, &dome.vertices[i]);
	}

	ns = assign_labels(&dome, &struts, at, lt);
	nf = assign_faces(&dome, &faces, lt);
	c0 = faces[0].name[0];

	ftotal = 0;
	for(i=0; i < nf; ++i)
	  ftotal += faces[i].count;

	fprintf(stderr, "\n\n%d faces total, %d different sizes\n\n",
	  ftotal, nf);

	fprintf(stderr, " face  count       e1          e2          e3\n");

	for(i=0, fp = faces; i < nf; ++i, ++fp)
	{
	  fprintf(stderr, "%5s  %5d  %7.3f (%s) %7.3f (%s) %7.3f (%s)\n",
	    fp->name, fp->count,
	    fp->s1->len * scale, fp->s1->name,
	    fp->s2->len * scale, fp->s2->name,
	    fp->s3->len * scale, fp->s3->name);
	}


	radius = dome.radius;

	/* Find the bounding box of the projected vertices */
	xmin = xmax = ymin = ymax = 0.;
	for(i=0; i < dome.nvert; ++i)
	{
	  Point pt = dome.vertices[i];
	  project_point(&pt);
	  xmin = MIN(xmin, pt.x); xmax = MAX(xmax, pt.x);
	  ymin = MIN(ymin, pt.y); ymax = MAX(ymax, pt.y);
	}

	/* Compute transformation matrix */
	/* TODO: multi-sheet */

	/* If this is a single-sheet drawing, will the strut list fit on the
	 * page below the diagram?
	 */
	strut_page = sheets != 1 || (nf+4)*(textheight*1.2) > (hgt-wid);

	if( xmax > xmin && ymax > ymin )
	{
	  float w, h;
	  w = (wid - lm - rm);
	  h = (hgt - tm - bm);

	  sx = w / (xmax - xmin);
	  sy = h / (ymax - ymin);

	  sx = sy = MIN(sx, sy);

	  tx = lm + w/2;
	  ty = bm + (strut_page ? (h/2) : (h - w/2));
	}

	write_prolog();
	start_page(++pagenum);

	fprintf(ofile, "%g setlinewidth\n\n", color ? .2 : 1.) ;

	for( i=0; i < dome.nface; ++i)
	  draw_face(&dome, i, faces, nf);

	if( strut_page ) {	/* strut list needs its own page */
	  end_page(pagenum);
	  start_page(++pagenum);
	}

	{
	  /* Draw strut list */
	  float x,y;
	  y = strut_page ? (hgt - tm) - textheight :
	  		   (bm + hgt - wid) - textheight;
	  x = lm;
	  fprintf(ofile, "%% strut list\nfixedFont setfont\n");
	  fprintf(ofile,
	    "%g %g moveto (%d faces total, %d different sizes) show\n",
	    x, y, ftotal, nf);
	  y -= textheight*1.2*2;

	  fprintf(ofile,
	    "%g %g moveto ( face  count       e1          e2          e3) show\n",
	    x, y);
	  y -= textheight*1.2;

	  for(i=0, fp = faces; i < nf; ++i, ++fp)
	  {
	    fprintf(ofile,
	      "%g %g moveto (%5s  %5d  %7.3f (%s) %7.3f (%s) %7.3f (%s)) show\n",
	      x,y,
	      fp->name, fp->count,
	      fp->s1->len * scale, fp->s1->name,
	      fp->s2->len * scale, fp->s2->name,
	      fp->s3->len * scale, fp->s3->name);
	    y -= textheight*1.2;
	    if( y < bm ) {
	      end_page(pagenum);
	      start_page(++pagenum);
	      y = (hgt - tm) - textheight;
	    }
	  }
	}

	end_page(pagenum);


	/* diagrams for the individual pieces */
	{
	  float w, h, x, y;
	  w = (wid - lm - rm) / 2;
	  h = (hgt - tm - bm) / 2;
	  for( i = 0; i < nf; ++i ) {
	    switch( i%4 ) {
	      case 0:
		start_page(++pagenum);
		x = lm; y = bm + h;
		break;
	      case 1:
		x = lm + w; y = bm + h;
		break;
	      case 2:
		x = lm; y = bm;
		break;
	      case 3:
		x = lm + w; y = bm;
		break;
	    }
	    draw_single_face(&dome, &faces[i], x,y,w-36,h-36);
	    if( i%4 == 3 || i == nf - 1 )
	      end_page(pagenum);
	  }
	}

	write_trailer();

	exit(0);
}




/**
 * Label all of the faces by edge lengths.  Any faces with
 * lengths within .1% are considered identical.
 * The most common face is 'A', and so on.
 *
 * \param dome	Dome to be processed
 * \param info	Returned face list; free with free()
 * \param lt	length tolerance as a fraction.
 *
 * \returns number of face entries in info list
 */
static int
assign_faces(const Dome *dome, FaceInfo **info, double lt)
{
	FaceInfo *rval = malloc(dome->nface * sizeof(*rval));
	Face	*face;
	int	ninfo = 0;
	int	i, j;
	char	name[10];

	for(i=0; i < dome->nface; ++i) {
	  rval[i].count = 0;
	  rval[i].name = NULL;
	}

	for(i=0, face = dome->faces; i < dome->nface; ++i, ++face)
	{
	  FaceInfo tmp;
	  compute_face(dome, (int *)face, &tmp);
	  tmp.count = 1;

	  for(j=0; j < ninfo && !match_face(&rval[j], &tmp); ++j);
	  if( j >= ninfo )
	    rval[ninfo++] = tmp;
	  else
	    ++rval[j].count;
	}

	qsort(rval, ninfo, sizeof(*rval), facesort);

	for(i=0; i < ninfo; ++i)
	{
	  if( i < 26 )
	    sprintf(name, "%c", 'A'+i);
	  else if( i < 26*26 )
	    sprintf(name, "%c%c", 'A'-1+i/26, 'A'+i%26);
	  else
	    sprintf(name, "S%d", i);
	  rval[i].name = strdup(name);
	}

	if( info != NULL )
	  *info = rval;

	return ninfo;
}


static StrutInfo *
find_strut(const char *name)
{
	int	i;
	StrutInfo *rval;

	for(rval=struts, i=0;
	    strcmp(rval->name, name) != 0 && i < ns;
	    ++i, ++rval);
	return i < ns ? rval : NULL;
}



/**
 * Examine a face, determine what its three edges are.
 *
 * \param dome  dome containing the data
 * \param face  face to be computed
 * \param info  info structure to fill in
 */
static void
compute_face(const Dome *dome, const int *face, FaceInfo *info)
{
	StrutInfo *s0, *s1, *s2, *tmp;
	Point	v1, v2, norm;
	int	i;

	/* Shortcut:  edges have already been labeled, so it's just a matter of
	 * looking up the label in the strut list.
	 */

	if( (i = find_edge(face[0], face[1], dome)) < 0 ||
	    (s0 = find_strut(dome->edges[i].name)) == NULL )
	{
	  fprintf(stderr,
	    "internal error in compute_face, unable to find edge %d,%d\n",
	    face[0], face[1]);
	  exit(4);
	}

	if( (i = find_edge(face[1], face[2], dome)) < 0 ||
	    (s1 = find_strut(dome->edges[i].name)) == NULL )
	{
	  fprintf(stderr,
	    "internal error in compute_face, unable to find edge %d,%d\n",
	    face[1], face[2]);
	  exit(4);
	}

	if( (i = find_edge(face[2], face[0], dome)) < 0 ||
	    (s2 = find_strut(dome->edges[i].name)) == NULL )
	{
	  fprintf(stderr,
	    "internal error in compute_face, unable to find edge %d,%d\n",
	    face[2], face[0]);
	  exit(4);
	}


	/* Rotate to put first edge (alphabetically) to top; if the first edge
	 * is repeated, prefer aab to aba
	 */

	if( strcmp(s1->name, s0->name) < 0 && strcmp(s1->name, s2->name) <= 0 )
	{
	  tmp = s0; s0 = s1; s1 = s2; s2 = tmp;
	}
	else if( strcmp(s2->name, s0->name) <= 0 &&
		 strcmp(s2->name, s1->name) < 0 ) {
	  tmp = s0; s0 = s2; s2 = s1; s1 = tmp;
	}

	/* Make sure this face is counter-clockwise.  Take the cross-product
	 * of 1st two edges and make sure vector faces out.  If not, swap 2nd
	 * two edges.  (This should never happen, as the dome software keeps
	 * faces in counter-clockwise order anyway.)
	 */

	ptSub(&dome->vertices[face[0]], &dome->vertices[face[1]], &v1);
	ptSub(&dome->vertices[face[1]], &dome->vertices[face[2]], &v2);
	crossprod(&v1, &v2, &norm);
	if( dotprod(&norm, &dome->vertices[face[0]]) < 0. ) {
	  tmp = s1; s1 = s2; s2 = tmp;
	}

	info->s1 = s0;
	info->s2 = s1;
	info->s3 = s2;
}


/**
 * Return true if these two faces match
 */
static int
match_face(const FaceInfo *a, const FaceInfo *b)
{
	return a->s1 == b->s1 && a->s2 == b->s2 && a->s3 == b->s3;
}


/**
 * Find the Face group to which this face belongs.
 *
 * \param list  FaceInfo list
 * \param n     length of list
 * \param dome  dome contianing vertex and face data
 * \param face  index into dome face list
 *
 * \returns element from FaceInfo list or NULL
 */
static FaceInfo *
findFace(FaceInfo *list, int n, const Dome *dome, int face)
{
	int	i;
	FaceInfo tmp;

	compute_face(dome, dome->faces[face], &tmp);

	for(i=0; i < n && !match_face(&list[i], &tmp); ++i);
	if( i < n )
	  return list+i;
	else
	  return NULL;
}


/* Sort by frequency and then alphabetically by strut names */
static	int
facesort(const void *aa, const void *bb)
{
	const FaceInfo *a = aa;
	const FaceInfo *b = bb;
	int i;

	if( (i = b->count - a->count) == 0 &&
	    (i = strcmp(a->s1->name, b->s1->name)) == 0 &&
	    (i = strcmp(a->s2->name, b->s2->name)) == 0)
	     i = strcmp(a->s3->name, b->s3->name);
	return i;
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
	  0.,0., wid, hgt, sheets) ;

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
	  "  dup stringbounds 2 div neg exch 2 div neg exch rmoveto\n"
	  "  show\n"
	  "} bind def\n\n"
	  "/ctrTxt2		%% (string) =>\n"
	  "{\n"
	  "  dup stringbounds 2 copy 2 div neg exch 2 div neg exch rmoveto\n"
	  "  1 setgray gsave rect grestore 0 setgray\n"
	  "  show\n"
	  "} bind def\n\n");
	  fprintf(ofile, "/labelFont /Times-Roman findfont %g scalefont def\n", textheight);
	  fprintf(ofile, "/edgeFont /Times-Roman findfont %g scalefont def\n", textheight-4);
	  fprintf(ofile, "/fixedFont /Courier findfont %g scalefont def\n", textheight);

	fprintf(ofile, "%%%%EndProlog\n\n") ;
}


static void
start_page(int pagenum)
{
	fprintf(ofile, "%%%%Page: \"%d\" %d\n\n", pagenum, pagenum) ;
	fprintf(ofile, "0 setlinejoin\n0 setlinecap\n[] 0 setdash\n") ;
	fprintf(ofile, "1 setlinewidth\n") ;
	fprintf(ofile, "\n");
}


static void
end_page(int pagenum)
{
	fprintf(ofile, "showpage\n\n");
	fprintf(ofile, "%%%%EndPage: \"%d\" %d\n\n", pagenum, pagenum) ;
	fprintf(ofile, "\n");
}


static void
write_trailer()
{
	fprintf(ofile, "%%%%Trailer\n") ;
	fprintf(ofile, "%%%%EOF\n") ;
}



/**
 * Write out a face in postscript.  Generate a transformation that
 * flattens it out.
 */
static void
draw_face(Dome *dome, int face, FaceInfo *facelist, int nf)
{
	Point	p0, p1, p2;
	float	lx, ly;
	FaceInfo *info, tmp;
	int	i;

	info = findFace(facelist, nf, dome, face);

	if( info == NULL ) {
	  fprintf(stderr, "face not found in draw_face\n");
	  compute_face(dome, dome->faces[face], &tmp);
	  info = &tmp;
	  tmp.name = "x";
	}

	p0 = dome->vertices[dome->faces[face][0]];
	p1 = dome->vertices[dome->faces[face][1]];
	p2 = dome->vertices[dome->faces[face][2]];

	project_point(&p0);
	project_point(&p1);
	project_point(&p2);

	/* We've transformed the end points, but this will cause increasing
	 * distortion as we move away from the center, so now we shorten
	 * the edges back to their original lengths.  This will cause all the
	 * endpoints to break away from each other, but this is a desirable
	 * side-effect, since we're going for an exploded view.
	 */
	if( explode )
	  adjust_face(&p0, &p1, &p2, dome, face);

	/* Convert to postscript coords */
	p0.x = p0.x * sx + tx;  p0.y = p0.y * sy + ty;
	p1.x = p1.x * sx + tx;  p1.y = p1.y * sy + ty;
	p2.x = p2.x * sx + tx;  p2.y = p2.y * sy + ty;

	fprintf(ofile, "%% face %s\n", info->name);
	if( color ) {
	  int c = info->name[0] - c0;
	  const float *clr = get_color(c);
	  fprintf(ofile, "%g %g %g setrgbcolor\n",
	    (clr[0]+3)/4., (clr[1]+3)/4., (clr[2]+3)/4.);
	  fprintf(ofile,
	    "newpath %g %g moveto %g %g lineto %g %g lineto closepath fill\n",
	    p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
	  fprintf(ofile, "0 setgray\n");
	}
	fprintf(ofile,
	  "newpath %g %g moveto %g %g lineto %g %g lineto closepath stroke\n",
	  p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);

	lx = (p0.x + p1.x + p2.x) / 3;
	ly = (p0.y + p1.y + p2.y) / 3;
	fprintf(ofile, "labelFont setfont\n");
	fprintf(ofile, "%g %g moveto (%s) ctrTxt\n", lx, ly, info->name);

	/* Now label the edges.  We need to identify them first. */
	fprintf(ofile, "edgeFont setfont\n");

	i = find_edge(dome->faces[face][0], dome->faces[face][1], dome);
	lx = (p0.x*5 + p1.x*5 + p2.x) / 11;
	ly = (p0.y*5 + p1.y*5 + p2.y) / 11;
	fprintf(ofile, "%g %g moveto (%s) ctrTxt\n",
		lx, ly, dome->edges[i].name);

	i = find_edge(dome->faces[face][1], dome->faces[face][2], dome);
	lx = (p1.x*5 + p2.x*5 + p0.x) / 11;
	ly = (p1.y*5 + p2.y*5 + p0.y) / 11;
	fprintf(ofile, "%g %g moveto (%s) ctrTxt\n",
		lx, ly, dome->edges[i].name);

	i = find_edge(dome->faces[face][2], dome->faces[face][0], dome);
	lx = (p2.x*5 + p0.x*5 + p1.x) / 11;
	ly = (p2.y*5 + p0.y*5 + p1.y) / 11;
	fprintf(ofile, "%g %g moveto (%s) ctrTxt\n",
		lx, ly, dome->edges[i].name);

#ifdef	COMMENT
	i = find_edge(dome->faces[face][0], dome->faces[face][1], dome);
	lx = (p0.x*5 + p1.x*5 + p2.x) / 11;
	ly = (p0.y*5 + p1.y*5 + p2.y) / 11;
	fprintf(ofile, "%g %g moveto (%.2g) ctrTxt\n",
		lx, ly, dome->edges[i].len);

	i = find_edge(dome->faces[face][1], dome->faces[face][2], dome);
	lx = (p1.x*5 + p2.x*5 + p0.x) / 11;
	ly = (p1.y*5 + p2.y*5 + p0.y) / 11;
	fprintf(ofile, "%g %g moveto (%.2g) ctrTxt\n",
		lx, ly, dome->edges[i].len);

	i = find_edge(dome->faces[face][2], dome->faces[face][0], dome);
	lx = (p2.x*5 + p0.x*5 + p1.x) / 11;
	ly = (p2.y*5 + p0.y*5 + p1.y) / 11;
	fprintf(ofile, "%g %g moveto (%.2g) ctrTxt\n",
		lx, ly, dome->edges[i].len);
#endif	/* COMMENT */

	fprintf(ofile, "\n");
}



/**
 * Adjust the vertices so that they're the specified distance
 * apart.  For exploded diagrams.
 */
static void
adjust_face(Point *p0, Point *p1, Point *p2, Dome *dome, int face)
{
#ifdef	TODO
	double d = ptDist(p0, p1);

	d = (d - len) / 2;		/* adjustment factor */

	projectPointOnLine(p0, p1, p0, d, p0);
	projectPointOnLine(p1, p0, p1, d, p1);
#endif	/* TODO */
}



static void
draw_single_face(Dome *dome, FaceInfo *face,
	double x, double y, double w, double h)
{
	double		xs, ys;
	double		maxlen;
	StrutInfo	*s1, *s2, *s3;
	double		A = face->s1->len, B = face->s2->len, C = face->s3->len;
	double		x0,y0, x1,y1, x2,y2;

	/* Pick an orientation.  If triangle is isosolese, put the two
	 * matching sides on top.  Otherwise, put the longest side on
	 * bottom.
	 */

	if( A == B ) {		/* C on bottom */
	  s1 = face->s1; s2 = face->s2; s3 = face->s3;
	}
	else if( A == C ) {	/* B on bottom */
	  s2 = face->s1; s3 = face->s2; s1 = face->s3;
	}
	else if( B == C ) {	/* A on bottom */
	  s3 = face->s1; s1 = face->s2; s2 = face->s3;
	}
	else if( C > A && C > B ) {	/* C on bottom */
	  s1 = face->s1; s2 = face->s2; s3 = face->s3;
	}
	else if( B > A && B > C ) {	/* B on bottom */
	  s2 = face->s1; s3 = face->s2; s1 = face->s3;
	}
	else {				/* A on bottom */
	  s3 = face->s1; s1 = face->s2; s2 = face->s3;
	}

	A = s1->len; B = s2->len; C = s3->len;

	maxlen = max(A, B);
	maxlen = max(maxlen, C);
	if( maxlen <= 0. ) {
	  fprintf(stderr, "invalid dimensions, face %s\n", face->name);
	  return;
	}
	xs = w/maxlen;
	ys = (h-textheight*2)/maxlen;
	xs = ys = min(xs,ys);

	y += textheight * 2;

	/* P0 = lower-left, P1 = lower-right, P2 = top.
	 * C = P0-P1, 
	 */
	x0 = 0; y0 = 0;
	x1 = C; y1 = 0;
	x2 = (B*B - A*A + C*C)/(2*C);
	y2 = sqrt(B*B - x2*x2);

	fprintf(ofile, "%% face %s\n", face->name);

	if( color ) {
	  int c = face->name[0] - c0;
	  const float *clr = get_color(c);
	  fprintf(ofile, "%g %g %g setrgbcolor\n",
	    (clr[0]+3)/4., (clr[1]+3)/4., (clr[2]+3)/4.);
	  fprintf(ofile,
	    "newpath %g %g moveto %g %g lineto %g %g lineto closepath fill\n",
	    x0*xs + x, y0*ys + y, x1*xs + x, y1*ys + y, x2*xs + x, y2*ys + y);
	  fprintf(ofile, "0 setgray\n");
	}

	fprintf(ofile,
	  "newpath %g %g moveto %g %g lineto %g %g lineto closepath stroke\n",
	  x0*xs + x, y0*ys + y, x1*xs + x, y1*ys + y, x2*xs + x, y2*ys + y);

	fprintf(ofile, "labelFont setfont\n");
	fprintf(ofile, "%g %g moveto (%s) ctrTxt\n",
	  (x0+x1+x2)/3*xs + x, (y0+y1+y2)/3*ys + y, face->name);

	fprintf(ofile, "%g %g moveto (%.2f (%s)) ctrTxt2\n",
	  (x0+x2)/2*xs + x, (y0+y2)/2*ys + y, B*scale, s2->name);

	fprintf(ofile, "%g %g moveto (%.2f (%s)) ctrTxt2\n",
	  (x2+x1)/2*xs + x, (y2+y1)/2*ys + y, A*scale, s1->name);

	fprintf(ofile, "%g %g moveto (%.2f (%s)) ctrTxt2\n",
	  (x0+x1)/2*xs + x, (y0+y1)/2*ys + y, C*scale, s3->name);

	fprintf(ofile, "%g %g moveto (Panel %s, make %d) ctrTxt\n",
	  (x0+x1)/2*xs + x, -textheight*1.5 + y, face->name, face->count);

	/* And now the height dimension */

	fprintf(ofile, "newpath %g %g moveto %g %g lineto stroke\n",
	  x1*xs+x+9, y1*ys+y, x1*xs+x+9, y2*ys+y);
	fprintf(ofile, "%g %g moveto (%.2f) ctrTxt2\n",
	  x1*xs+x+9, (y0+y2)/2*ys+y, (y2-y0)*scale);

	fprintf(ofile, "\n");
}
