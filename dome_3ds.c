#ifndef lint
static const char rcsid[] =
	"$Id: dome_3ds.c,v 1.2 2004/11/12 08:05:47 efalk Exp $" ;
#endif

/**********
 *
 *
 *	@@@@	@@@   @	  @  @@@@@	    @@@	  @@@@	  @@@@	
 *	@   @  @   @  @@ @@  @		       @  @   @	 @	
 *	@   @  @   @  @ @ @  @@@	     @@	  @   @	  @@@	
 *	@   @  @   @  @ @ @  @		       @  @   @	     @	
 *	@@@@	@@@   @ @ @  @@@@@  @@@@@   @@@	  @@@@	 @@@@	
 *
 *	DOME_3DS - Convert dome file to .3ds file.
 *
 *	Tubes and the panels are written out as a single flat triangle mesh.
 *
 *	Routines provided here:
 *
 *
 *	Edward A. Falk
 *	efalk@sourceforge.net
 *
 *	October, 2004
 *
 *
 *
 **********/


static const char usage[] =
"\n"
"dome_3ds -- convert a geodesic dome file to .3ds\n"
"\n"
"usage:	dome_3ds [options] <domefile> <3dsfile>\n"
"	-cameras	add some cameras\n"
"	-radius R	dome radius (needed for camera positions) (12)\n"
"	-floor		add floor\n"
"	-ground		add ground\n"
"	-noframe	do not include frame\n"
"	-noshell	do not include shell\n"
;



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include <lib3ds/file.h>
#include <lib3ds/mesh.h>
#include <lib3ds/material.h>
#include <lib3ds/light.h>
#include <lib3ds/camera.h>
#include <lib3ds/node.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>

#include "3ds_utils.h"

#include "utils.h"
#include "dome.h"


#define	PIPE_RAD	.75/12./2.	/* 3/4" pipe */
#define	PIPE_SIDES	6

#define	EYE_HGT		5.5		/* assuming feet here */

	float	radius = 12.;

static	int	cameras = 0;
static	int	ground = 0;
static	int	add_floor = 0;
static	int	no_frame = 0;
static	int	no_shell = 0;


static	void	write_dome_3ds(Dome *dome, const char *filename);
static	void	add_faces(Dome *, Lib3dsFile *);
static	void	add_edges(Dome *dome, Lib3dsFile *);
static	void	add_cameras(Dome *dome, Lib3dsFile *);
static	void	gen_pipe(Lib3dsFile *file, double x, double y, double z,
			const char *name);
static	void	transform_pipe(Lib3dsNode *, double len, Point *, Point *);


#define	streq(a,b)	(strcmp(a,b) == 0)


int
main(int argc, char **argv)
{
	char *ifilename = NULL;
	char *ofilename = NULL;
	FILE *ifile;
	Dome	cdome;

	for(++argv; --argc > 0; ++argv)
	{
	  if( streq(*argv, "-help") || streq(*argv, "--help") ) {
	    printf(usage);
	    exit(0);
	  }
	  else if( streq(*argv, "-cameras") )
	    cameras = 1;
	  else if( streq(*argv, "-radius") && --argc > 0)
	    radius = atof(*++argv);
	  else if( streq(*argv, "-floor") )
	    add_floor = 1;
	  else if( streq(*argv, "-ground") )
	    ground = 1;
	  else if( streq(*argv, "-noframe") )
	    no_frame = 1;
	  else if( streq(*argv, "-noshell") )
	    no_shell = 1;
	  else if( ifilename == NULL )
	    ifilename = *argv;
	  else if( ofilename == NULL )
	    ofilename = *argv;
	}

	if( ifilename == NULL || ofilename == NULL ) {
	  fprintf(stderr, usage);
	  exit(2);
	}

	if( (ifile = fopen(ifilename, "r")) == NULL )
	{
	  perror(ifilename);
	  exit(3);
	}

	read_dome(&cdome, ifile);
	write_dome_3ds(&cdome, ofilename);

	exit(0);
}



static void
write_dome_3ds(Dome *dome, const char *filename)
{
	Lib3dsFile	*file;
	Lib3dsMaterial	*canvas, *metal;

	edge_lengths(dome);

	file = lib3ds_file_new();

	strncpy(file->name, "OasisDome", sizeof(file->name));
	file->master_scale = 1.;
	file->ambient[0] = file->ambient[1] = file->ambient[2] = .5;
	file->background.solid.use = 1;
	file->background.solid.col[0] = .3;
	file->background.solid.col[1] = .3;
	file->background.solid.col[2] = .5;

	if( cameras )
	  add_cameras(dome, file);


	if( !no_shell ) {
	  canvas = lib3ds_material_new();
	  strncpy(canvas->name, "canvas", sizeof(canvas->name));
	  canvas->ambient[0] = canvas->ambient[1] = canvas->ambient[2] = .8;
	  canvas->diffuse[0] = canvas->diffuse[1] = canvas->diffuse[2] = .8;
	  canvas->specular[0] = canvas->specular[1] = canvas->specular[2] = 0.;
	  canvas->two_sided = 1;
	  /* TODO: change material properties */
	  lib3ds_file_insert_material(file, canvas);

	  add_faces(dome, file);
	}


	if( !no_frame ) {
	  metal = lib3ds_material_new();
	  strncpy(metal->name, "metal", sizeof(metal->name));
	  /* TODO: change material properties */
	  lib3ds_file_insert_material(file, metal);

	  add_edges(dome, file);
	}

	lib3ds_file_save(file, filename);
}






static void
add_faces(Dome *dome, Lib3dsFile *file)
{
	Lib3dsMesh	*mesh;
	Lib3dsPoint	*points;
	Lib3dsFace	*faces;
	int		i;
	static Point	p0 = {0.,0.,0.};
	Point		p1;

	mesh = lib3ds_mesh_new("cover");

	lib3ds_mesh_new_point_list(mesh, dome->nvert);
	points = mesh->pointL;
	for(i=0; i < dome->nvert; ++i) {
	  /* Extend the canvas out slightly, so it encompases the pipes */
	  projectPointOnLine(&p0, &dome->vertices[i], &dome->vertices[i],
	    PIPE_RAD*5, &p1);
	  points[i].pos[0] = p1.x;
	  points[i].pos[1] = p1.y;
	  points[i].pos[2] = p1.z;
	}

	lib3ds_mesh_new_face_list(mesh, dome->nface);
	faces = mesh->faceL;

	for(i=0; i < dome->nface; ++i) {
	  strncpy(faces[i].material, "canvas", sizeof(faces[i].material));
	  faces[i].points[0] = dome->faces[i][0];
	  faces[i].points[1] = dome->faces[i][1];
	  faces[i].points[2] = dome->faces[i][2];
	}

	lib3ds_file_insert_mesh(file, mesh);

	lib3ds_file_insert_node(file, mesh_node_i("cover"));
}




static	void
add_edges(Dome *dome, Lib3dsFile *file)
{
	Lib3dsNode	*node;
	Edge		*edge = dome->edges;
	Point		*verts = dome->vertices, *v;
	int		i;


	/* TODO: We define just one edge, and then use node structures
	 * to clone it.  Is this legitimate?
	 */

	gen_pipe(file, 0., 0., 0., "frame");


	for(i=0; i < dome->nedge; ++i, ++edge)
	{
	  node = mesh_node("frame");
	  v = &verts[edge->v0];
	  transform_pipe(node, edge->len, &verts[edge->v0], &verts[edge->v1]);
	  lib3ds_file_insert_node(file, node);
	}
}





/**
 * Define one pipe mesh.
 */
static	void
gen_pipe(Lib3dsFile *file, double x, double y, double z, const char *name)
{
	Lib3dsMesh	*mesh;
	Lib3dsPoint	*points;
	Lib3dsFace	*faces;
	double		a, r = PIPE_RAD;	/* 3/4" pipe */
	int		i;

	/* Each edge is represented by a smooth-shaded hexagonal tube */

	mesh = lib3ds_mesh_new(name);

	lib3ds_mesh_new_point_list(mesh, PIPE_SIDES*2);
	points = mesh->pointL;

	/* Vertical pipe (+Z) direction */
	for(i=0; i<PIPE_SIDES; ++i) {
	  a = i * 2*M_PI / PIPE_SIDES;
	  points[i].pos[0] = points[i+PIPE_SIDES].pos[0] = r * cos(a) + x;
	  points[i].pos[1] = points[i+PIPE_SIDES].pos[1] = r * sin(a) + y;
	  points[i].pos[2] = 0. + z;
	  points[i+PIPE_SIDES].pos[2] = 1. + z;
	}


	/* One tube */

	lib3ds_mesh_new_face_list(mesh, PIPE_SIDES*2);
	faces = mesh->faceL;

	for(i=0; i < PIPE_SIDES; ++i) {
	  strncpy(faces[i*2].material, "metal", sizeof(faces[i].material));
	  faces[i*2].points[0] = i;
	  faces[i*2].points[1] = (i+1) % PIPE_SIDES;
	  faces[i*2].points[2] = i+PIPE_SIDES;
	  faces[i*2].smoothing = 1;

	  strncpy(faces[2*i+1].material, "metal", sizeof(faces[i].material));
	  faces[2*i+1].points[0] = (i+1) % PIPE_SIDES + PIPE_SIDES;
	  faces[2*i+1].points[1] = i+PIPE_SIDES;
	  faces[2*i+1].points[2] = (i+1)%PIPE_SIDES;
	  faces[2*i+1].smoothing = 1;
	}

	lib3ds_file_insert_mesh(file, mesh);
}



/**
 * Compute the translation, scale, and rotation which will stretch a
 * pipe from p0 to p1
 */
void
transform_pipe(Lib3dsNode *node, double len, Point *p0, Point *p1)
{
	Lib3dsVector	v0, v1, va;
	double		angle;

	node_translate(node, p0->x, p0->y, p0->z);
	node_scale(node, 1.,1.,len);

	/* Rotation is the tricky part.  Find the axis and angle that
	 * will swing the far end of the pipe down to p1.
	 */
	v0[0] = 0; v0[1] = 0; v0[2] = 1;
	v1[0] = p1->x - p0->x; v1[1] = p1->y - p0->y; v1[2] = p1->z - p0->z;

	lib3ds_vector_normalize(v1);
	angle = acos(lib3ds_vector_dot(v0, v1));
	lib3ds_vector_cross(va, v0, v1);

	node_rotatev(node, va, -angle);
}



/**
 * Create some cameras
 */
void
add_cameras(Dome *dome, Lib3dsFile *file)
{
	Lib3dsVector	campos, target;
	double		eye = EYE_HGT + dome_min(dome);


	target[0] = 0.; target[1] = 0.; target[2] = eye;
	campos[0] = 0.; campos[1] = -radius*3; campos[2] = eye;
	create_camera(file, campos, target, 0., radius*6, 45., "Front_view");

	campos[0] = -radius*3; campos[1] = 0.; campos[2] = eye;
	create_camera(file, campos, target, 0., radius*6, 45., "Left_side");

	campos[0] = -radius*2; campos[1] = -radius*2; campos[2] = radius*2;
	create_camera(file, campos, target, 0., radius*6, 45., "Aerial");

	campos[0] = 0.; campos[1] = radius*.5; campos[2] = eye;
	create_camera(file, campos, target, 0., radius*6, 60., "Inside");
}



#ifdef	COMMENT
	/* Test to see if I can do this right */
	lib3ds_mesh_new_point_list(mesh, 4);
	points = mesh->pointL;
	points[0].pos[0] = -10.;
	points[0].pos[1] = 0.;
	points[0].pos[2] = 0.;
	points[1].pos[0] = 10.;
	points[1].pos[1] = 10.;
	points[1].pos[2] = 0.;
	points[2].pos[0] = 10.;
	points[2].pos[1] = -10.;
	points[2].pos[2] = 0.;
	points[3].pos[0] = 0.;
	points[3].pos[1] = 0.;
	points[3].pos[2] = 10.;
	lib3ds_mesh_new_flag_list(mesh, 4);

	lib3ds_mesh_new_face_list(mesh, 3);
	faces = mesh->faceL;

	strncpy(faces[0].material, "canvas", sizeof(faces[0].material));
	faces[0].points[0] = 0;
	faces[0].points[1] = 2;
	faces[0].points[2] = 3;

	strncpy(faces[1].material, "canvas", sizeof(faces[0].material));
	faces[1].points[0] = 2;
	faces[1].points[1] = 1;
	faces[1].points[2] = 3;

	strncpy(faces[2].material, "canvas", sizeof(faces[0].material));
	faces[2].points[0] = 1;
	faces[2].points[1] = 0;
	faces[2].points[2] = 3;
#endif	/* COMMENT */
