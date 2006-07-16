#ifndef lint
static const char rcsid[] =
	"$Id: dome.c,v 1.1 2004/11/12 07:13:22 efalk Exp $" ;
#endif

/**********
 *
 *
 *	@@@@    @@@   @   @  @@@@@
 *	@   @  @   @  @@ @@  @
 *	@   @  @   @  @ @ @  @@@
 *	@   @  @   @  @ @ @  @
 *	@@@@    @@@   @ @ @  @@@@@
 *
 *	DOME -- functions for creating and editing Geodesic domes.
 *
 *	Routines provided here:
 *
 *
 *	init_dome(double radius)
 *		Initialize the dome.
 *
 *
 *	tesselate_cmd(int n, double radius)
 *		Initialize a tesselated dome.
 *
 *
 *	normalize_cmd(double radius, double f)
 *		Normalize cdome to the desired radius.
 *
 *
 *	render_dome(Dome *dome)
 *		Render the dome.
 *
 *
 *	draw_faces(Dome *dome)
 *
 *
 *	draw_element(int element)
 *		Draw one single element
 *
 *
 *	delete_element(Dome *dome, int element)
 *		Delete one single element
 *
 *
 *	draw_sphere(double x, double y, double z)
 *		Utility:  Draw a small sphere.  Used for vertices.
 *
 *
 *	Edward A. Falk
 *	falk@efalk.org
 *
 *	November, 2004
 *
 *
 **********/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include <glut.h>

#include "utils.h"
#include "dome.h"


	int	sphereList;
	int	domeList = -1;
	int	cylList;
	int	picking = 0;
	int	show_verts = 1;
	int	show_edges = 1;
	int	show_faces = 1;




static	void	draw_vertices(Point *, int npt);
static	void	draw_faces(Dome *);
static	void	draw_edges(Dome *dome);



	const GLfloat red[4] = {1., 0., 0., 1.0};
	const GLfloat selRed[4] = {1., .7, .7, 1.0};	/* selection red */
	const GLfloat green[4] = {0., 1., 0., 1.0};
	const GLfloat blue[4] = {0., 0., 1., 1.0};
	const GLfloat white[4] = {1., 1., 1., 1.0};
	const GLfloat lgrey[4] = {.8, .8, .8, 1.0};
	const GLfloat lgreya[4] = {.8, .8, .8, .7};

	Dome	tdome, cdome;


/**
 * Initialize the dome.  Initialize cdome with values from f0dome, an
 * icosahedron with the bottom vertex missing.  Render cdome into
 * OpenGL list domeList.
 */
void
init_dome(double radius)
{
	clear_dome(&cdome);
	copy_dome(&f0dome, &cdome);
	scale_dome(&cdome, radius);

	domeList = glGenLists(3);

	render_dome(&cdome);
}



/**
 * Initialize a tesselated dome.  Tesselate f0dome by the specified
 * frequency and copy to cdome.
 */
void
tesselate_cmd(int n, double radius)
{
	tesselate(&f0dome, &tdome, n);
	clear_dome(&cdome);
	copy_dome(&tdome, &cdome);
	scale_dome(&cdome, radius);

	render_dome(&cdome);
}



/**
 * Normalize cdome to the desired radius.  Partial normalization is ok.
 */
int
normalize_cmd(double radius, double f)
{
	normalize_dome(&cdome, radius, f);

	render_dome(&cdome);

	return f < 1.;
}


/**
 * Render the dome.  Vertices, edges, and faces are rendered into
 * domeList, domeList+1, and domeList+2.
 */
void
render_dome(Dome *dome)
{
	glNewList(domeList, GL_COMPILE);
	  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
	  draw_vertices(dome->vertices, dome->nvert);
	glEndList();

	glNewList(domeList+1, GL_COMPILE);
	  draw_edges(dome);
	glEndList();

	glNewList(domeList+2, GL_COMPILE);
	  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, lgrey);
	  draw_faces(dome);
	glEndList();
}



/**
 * Draw one vertex.  Caller must set attributes first.
 */
static	void
draw_vertex(Point *pt)
{
	draw_sphere(pt->x, pt->y, pt->z);
}



/**
 * Draw all vertices.
 */
static	void
draw_vertices(Point *pts, int npts)
{
	int	i;

	for(i = 0; i < npts; ++i)
	{
	  glLoadName(VTX_BASE+i);
	  draw_vertex(pts);
	  ++pts;
	}
}


/**
 * Draw one single edge.  Caller must set attributes first.
 */
static	void
draw_edge(Dome *dome, Edge *edge)
{
	Point	*pts = dome->vertices;
	int	v0, v1;

	v0 = edge->v0;
	v1 = edge->v1;
	glBegin(GL_LINES);
	  glVertex3f(pts[v0].x, pts[v0].y, pts[v0].z);
	  glVertex3f(pts[v1].x, pts[v1].y, pts[v1].z);
	glEnd();
}


/**
 * Draw all edges in blue color.
 */
static	void
draw_edges(Dome *dome)
{
	int	i;

	glDisable(GL_LIGHTING);
	glColor3fv(blue);
	for (i = 0; i < dome->nedge; i++) {
	  glLoadName(EDGE_BASE+i);
	  draw_edge(dome, &dome->edges[i]);
	}
	glEnable(GL_LIGHTING);
}



/**
 * Draw one face.  Caller must set attributes first.
 */
static	void
draw_face(Dome *dome, Face *face)
{
	Point	*pts = dome->vertices;

	Point d1, d2, norm;
	Point	*p0 = pts + (*face)[0];
	Point	*p1 = pts + (*face)[1];
	Point	*p2 = pts + (*face)[2];
	ptSub(p1, p0, &d1);
	ptSub(p2, p0, &d2);
	normcrossprod(&d1, &d2, &norm);
	glNormal3f(norm.x, norm.y, norm.z);
	glBegin(GL_TRIANGLES);
	  glVertex3f(p0->x, p0->y, p0->z);
	  glVertex3f(p1->x, p1->y, p1->z);
	  glVertex3f(p2->x, p2->y, p2->z);
	glEnd();
}


static	void
draw_faces(Dome *dome)
{
	int	i;
	Face	*face = dome->faces;

	for (i = 0; i < dome->nface; i++)
	{
	  glLoadName(i);
	  draw_face(dome, face);
	  ++face;
	}

#ifdef	COMMENT
	/* Show face centers */
	face = dome->faces;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
	for (i = 0; i < dome->nface; i++) {
	  Point ctr;
	  Point	*p0 = pts + (*face)[0];
	  Point	*p1 = pts + (*face)[1];
	  Point	*p2 = pts + (*face)[2];
	  ptAvg3(p0, p1, p2, &ctr);
	  draw_sphere(ctr.x, ctr.y, ctr.z);
	  ++face;
	}
	glEnd();
#endif	/* COMMENT */

#ifdef	COMMENT
	/* Draw normal vectors for debugging */
	glDisable(GL_LIGHTING);
	glColor3fv(blue);
	glBegin(GL_LINES);
	face = dome->faces;
	for (i = 0; i < dome->nface; i++) {
	  Point d1, d2, norm, ctr;
	  Point	*p0 = pts + (*face)[0];
	  Point	*p1 = pts + (*face)[1];
	  Point	*p2 = pts + (*face)[2];
	  ptAvg3(p0, p1, p2, &ctr);
	  ptSub(p1, p0, &d1);
	  ptSub(p2, p0, &d2);
	  normcrossprod(&d1, &d2, &norm);
	  ptMul(&norm, .25, &norm);
	  ptAdd(&ctr, &norm, &norm);
	  glVertex3f(ctr.x, ctr.y, ctr.z);
	  glVertex3f(norm.x, norm.y, norm.z);
	  ++face;
	}
	glEnd();
	glEnable(GL_LIGHTING);
#endif	/* COMMENT */

}


/**
 * Draw one single element
 */
void
draw_element(int element)
{
	if( element >= VTX_BASE )		/* Vertex */
	{
	  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
	  draw_vertex(&cdome.vertices[element-VTX_BASE]);
	}

	else if( element >= EDGE_BASE )	/* edge */
	{
	  glDisable(GL_LIGHTING);
	  glColor3fv(red);
	  draw_edge(&cdome, &cdome.edges[element-EDGE_BASE]);
	  glEnable(GL_LIGHTING);
	}

	else if( element >= 0 )		/* face */
	{
	  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, selRed);
	  draw_face(&cdome, &cdome.faces[element]);
	}
}



/**
 * Delete one single element
 */
void
delete_element(Dome *dome, int element)
{
	if( element >= VTX_BASE )		/* Vertex */
	{
	  delete_vertex(dome, element-VTX_BASE);
	}

	else if( element >= EDGE_BASE )	/* edge */
	{
	  delete_edge(dome, element-EDGE_BASE);
	}

	else if( element >= 0 )		/* face */
	{
	  delete_face(dome, element);
	}

	render_dome(dome);
}


/**
 * Utility:  Draw a small sphere.  Used for vertices.
 */
void
draw_sphere(double x, double y, double z)
{
	glPushMatrix();
	  glTranslatef(x, y, z);
	  glCallList(sphereList);
	glPopMatrix();
}
