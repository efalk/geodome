#ifndef lint
static const char rcsid[] =
	"$Id: dome_file.c,v 1.3 2005/04/19 18:45:47 efalk Exp $" ;
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
	  fprintf(ofile, "%3d: %g,%g,%g\n",
	    i, dome->vertices[i].x, dome->vertices[i].y, dome->vertices[i].z);

	fprintf(ofile, "%d edges:\n", dome->nedge);
	for(i=0, edge = dome->edges; i < dome->nedge; ++i, ++edge)
	  fprintf(ofile, "%3d: %d,%d, %g  \"%s\"\n",
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
	if( fscanf(ifile, "radius: %g\n", &radius) < 1 )
	  goto out;
	if( fscanf(ifile, "%d vertices:\n", &dome->nvert) < 1 )
	  goto out;
	dome->vertices = malloc(dome->nvert * sizeof(*dome->vertices));
	for(i=0; i < dome->nvert; ++i)
	  if( fscanf(ifile, "%3d: %g,%g,%g\n", &j,
	    &dome->vertices[i].x, &dome->vertices[i].y, &dome->vertices[i].z)
	    	< 4 )
	    goto out;

	if( fscanf(ifile, "%d edges:\n", &dome->nedge) < 1 )
	  goto out;
	dome->edges = malloc(dome->nedge * sizeof(*dome->edges));
	for(i=0, edge = dome->edges; i < dome->nedge; ++i, ++edge)
	{
	  if( fscanf(ifile, "%3d: %d,%d, %lg \"%78[^\"]\"\n",
	    &j, &edge->v0, &edge->v1, &edge->len, name) < 5 )
	      goto out;
	  if( strcmp(name, "-") != 0 )
	    edge->name = strdup(name);
	  else
	    edge->name = NULL;
	}

	if( fscanf(ifile, "%d faces:\n", &dome->nface) < 1 )
	  goto out;
	dome->faces = malloc(dome->nface * sizeof(*dome->faces));
	for(i=0, face = dome->faces; i < dome->nface; ++i, ++face)
	  if( fscanf(ifile, "%3d: %d,%d,%d\n",
	    &j, &face[0][0], &face[0][1], &face[0][2]) < 4 )
	      goto out;

	return;

out:
	fprintf(stderr, "premature end of file\n");
	exit(3);
}
