#ifndef lint
static const char rcsid[] =
	"$Id: dome_file.c,v 1.1 2004/11/12 07:13:22 efalk Exp $" ;
#endif

/**********
 *
 *
 *	@@@@    @@@   @   @  @@@@@         @@@@@   @@@   @      @@@@@  
 *	@   @  @   @  @@ @@  @             @        @    @      @      
 *	@   @  @   @  @ @ @  @@@           @@@      @    @      @@@    
 *	@   @  @   @  @ @ @  @             @        @    @      @      
 *	@@@@    @@@   @ @ @  @@@@@  @@@@@  @       @@@   @@@@@  @@@@@  
 *
 *	DOME_FILE - Read/write functions for geodesic domes.
 *
 *
 *
 *	Edward A. Falk
 *	efalk@sourceforge.net
 *
 *	October, 2004
 *
 *
 **********/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include "utils.h"
#include "dome.h"



/**
 * Write out a dome file.
 */
void
write_dome(Dome *dome, FILE *ofile)
{
	int	i;
	Edge	*edge;
	Face	*face;

	if( ofile == NULL )
	  return;

	fprintf(ofile, "radius: %g\n", radius);
	fprintf(ofile, "%d vertices:\n", dome->nvert);
	for(i=0; i < dome->nvert; ++i)
	  fprintf(ofile, "%3d: %.4g,%.4g,%.4g\n",
	    i, dome->vertices[i].x, dome->vertices[i].y, dome->vertices[i].z);

	fprintf(ofile, "%d edges:\n", dome->nedge);
	for(i=0, edge = dome->edges; i < dome->nedge; ++i, ++edge)
	  fprintf(ofile, "%3d: %d,%d, %.4g  \"%s\"\n",
	    i, edge->v0, edge->v1, edge->len,
	    edge->name != NULL ? edge->name : "-");


	fprintf(ofile, "%d faces:\n", dome->nface);
	for(i=0, face = dome->faces; i < dome->nface; ++i, ++face)
	  fprintf(ofile, "%3d: %d,%d,%d\n",
	    i, face[0][0], face[0][1], face[0][2]);
}



/**
 * Read a dome file.
 */
void
read_dome(Dome *dome, FILE *ifile)
{
	int	i, j;
	Edge	*edge;
	Face	*face;
	char	name[80];

	clear_dome(dome);
	fscanf(ifile, "radius: %g\n", &radius);
	fscanf(ifile, "%d vertices:\n", &dome->nvert);
	dome->vertices = malloc(dome->nvert * sizeof(*dome->vertices));
	for(i=0; i < dome->nvert; ++i)
	  fscanf(ifile, "%3d: %g,%g,%g\n", &j,
	    &dome->vertices[i].x, &dome->vertices[i].y, &dome->vertices[i].z);

	fscanf(ifile, "%d edges:\n", &dome->nedge);
	dome->edges = malloc(dome->nedge * sizeof(*dome->edges));
	for(i=0, edge = dome->edges; i < dome->nedge; ++i, ++edge)
	{
	  fscanf(ifile, "%3d: %d,%d, %lg \"%78[^\"]\"\n",
	    &j, &edge->v0, &edge->v1, &edge->len, name);
	  if( strcmp(name, "-") != 0 )
	    edge->name = strdup(name);
	}

	fscanf(ifile, "%d faces:\n", &dome->nface);
	dome->faces = malloc(dome->nface * sizeof(*dome->faces));
	for(i=0, face = dome->faces; i < dome->nface; ++i, ++face)
	  fscanf(ifile, "%3d: %d,%d,%d\n",
	    &j, &face[0][0], &face[0][1], &face[0][2]);
}
