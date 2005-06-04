#ifndef lint
static const char rcsid[] =
	"$Id: dome_math.c,v 1.4 2005/05/14 02:11:48 efalk Exp $" ;
#endif

/**********
 *
 *
 *	@@@@    @@@   @   @  @@@@@         @   @   @@@   @@@@@  @   @
 *	@   @  @   @  @@ @@  @             @@ @@  @   @    @    @   @
 *	@   @  @   @  @ @ @  @@@           @ @ @  @@@@@    @    @@@@@
 *	@   @  @   @  @ @ @  @             @ @ @  @   @    @    @   @
 *	@@@@    @@@   @ @ @  @@@@@  @@@@@  @ @ @  @   @    @    @   @
 *
 *	DOME_MATH - Various math routines.
 *
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
#include <string.h>
#include <math.h>
#include <sys/types.h>

#include "utils.h"
#include "dome.h"




static	void	tesselate1(const Dome *in, Face face, int f, Dome *out,
			int *fcount, int *vcount, int *ecount);
static	void	add_face(Point *p0, Point *p1, Point *p2, Dome *dome,
			int *nf, int *ne, int *nv);
static	int	match_vtx(Point *vtx, Dome *dome, int *nv);
static	void	find_neighbors(Dome *dome, int v, Vertex *vertices,
			Edge **e0, Edge **e1, int *v0, int *v1);
#ifdef	COMMENT
static	void	find_new_point(Point *p, Point *p0, Point *p1, double, double);
#endif	/* COMMENT */
static	void	find_best_point(Point *p0, Point *p1, Point *p2, double,
			Point *rval);


	/* The 12 vertices */

static Point icos_vert[12] = {
  /* coordinates for icosahedron with edge length 1.05146 */
  /* Radius = 1 */
  /* Z is up */
  {0, 0, 1},				/* a */
  {0.894427, 0, 0.447214},		/* b */
  {0.276393, -0.850651, 0.447214},	/* c */
  {-0.723607, -0.525731, 0.447214},	/* d */
  {-0.723607, 0.525731, 0.447214},	/* e */
  {0.276393, 0.850651, 0.447214},	/* f */
  {-0.894427, 0, -0.447214},		/* g */
  {-0.276393, 0.850651, -0.447214},	/* h */
  {0.723607, 0.525731, -0.447214},	/* i */
  {0.723607, -0.525731, -0.447214},	/* j */
  {-0.276393, -0.850651, -0.447214},	/* k */
  {0, 0, -1},				/* l */
};

	/* The 30 edges */
static Edge icos_edge[30] = {
	{0,1,0.,NULL}, {0,2,0.,NULL}, {0,3,0.,NULL}, {0,4,0.,NULL},
	{0,5,0.,NULL}, {1,2,0.,NULL}, {2,3,0.,NULL}, {3,4,0.,NULL},
	{4,5,0.,NULL}, {5,1,0.,NULL}, {1,9,0.,NULL}, {9,2,0.,NULL},
	{2,10,0.,NULL}, {10,3,0.,NULL}, {3,6,0.,NULL}, {6,4,0.,NULL},
	{4,7,0.,NULL}, {7,5,0.,NULL}, {5,8,0.,NULL}, {8,1,0.,NULL},
	{6,7,0.,NULL}, {7,8,0.,NULL}, {8,9,0.,NULL}, {9,10,0.,NULL},
	{10,6,0.,NULL}, {11,7,0.,NULL}, {11,8,0.,NULL}, {11,9,0.,NULL},
	{11,10,0.,NULL}, {11,6,0.,NULL}
};

	/* The 20 faces */

static Face icos_face[20] = {
	{0,2,1}, {0,3,2}, {0,4,3}, {0,5,4}, {0,1,5}, {1,2,9}, {2,10,9},
	{2,3,10}, {3,6,10}, {3,4,6}, {4,7,6}, {4,5,7}, {5,8,7}, {5,1,8},
	{1,9,8}, {11,6,7}, {11,7,8}, {11,8,9}, {11,9,10}, {11,10,6},
};


	Dome	f0ball = {
	  1.,
	  icos_vert, NA(icos_vert),
	  icos_edge, NA(icos_edge),
	  icos_face, NA(icos_face)
	};

	Dome	f0dome = {
	  1.,
	  icos_vert, NA(icos_vert)-1,
	  icos_edge, NA(icos_edge)-5,
	  icos_face, NA(icos_face)-5
	};



#ifdef	COMMENT
	/* OCTOHEDRON */

	/* Octohedrons don't make very good geodesic domes, so this is
	 * dead code.
	 */

	/* The 6 vertices */

static Point octo_vert[6] = {
  /* coordinates for octohedron */
  /* Radius = 1 */
  {0, 0, 1},		/* a */
  {1, 0, 0},		/* b */
  {0, -1, 0},		/* c */
  {-1, 0, 0},		/* d */
  {0, 1, 0},		/* e */
  {0, 0, -1}		/* f */
};

	/* The 12 edges */
static Edge octo_edge[12] = {
  {0,1, 0,NULL}, {0,2, 0,NULL}, {0,3, 0,NULL}, {0,4, 0,NULL},
  {1,2, 0,NULL}, {2,3, 0,NULL}, {3,4, 0,NULL}, {4,1, 0,NULL},
  {1,5, 0,NULL}, {2,5, 0,NULL}, {3,5, 0,NULL}, {4,5, 0,NULL},
};

	/* The 8 faces */

static Face octo_face[8] = {
  {0,2,1}, {0,3,2}, {0,4,3}, {0,1,4}, {5,1,2}, {5,2,3}, {5,3,4}, {5,4,1}
};


	Dome	octoball = {
	  octo_vert, NA(octo_vert),
	  octo_edge, NA(octo_edge),
	  octo_face, NA(octo_face)
	};

	Dome	octodome = {
	  octo_vert, NA(octo_vert)-1,
	  octo_edge, NA(octo_edge)-4,
	  octo_face, NA(octo_face)-4
	};
#endif	/* COMMENT */




/**
 * Zero a dome structure.  Any resources allocated are leaked.
 * Only call this on dome structures for which you know there are
 * no allocated resources.
 */
void
clear_dome(Dome *dome)
{
	memset(dome, 0, sizeof(*dome));
}



/**
 * Free all of the dome's resources and clear it.
 */
void
delete_dome(Dome *dome)
{
	free(dome->vertices);
	free(dome->edges);
	free(dome->faces);
	clear_dome(dome);
}


/**
 * Copy vertex values from one dome to the other.  Both domes must have
 * the same number of vertices.
 */
void
copy_dome_values(const Dome *in, Dome *out)
{
	if( in->nvert != out->nvert ) {
	  fprintf(stderr, "mismatched domes in copy_dome_values()\n");
	  return;
	}

	memcpy(out->vertices, in->vertices,
		out->nvert * sizeof(*out->vertices));
}



/**
 * Copy one dome to another.  All resources in the output dome are freed
 * first, and then replaced with copies from the input dome.
 */
void
copy_dome(const Dome *in, Dome *out)
{
	free(out->vertices);
	free(out->edges);
	free(out->faces);

	out->nface = in->nface;
	out->nedge = in->nedge;
	out->nvert = in->nvert;

	out->vertices = calloc(out->nvert, sizeof(*out->vertices));
	out->faces = calloc(out->nface, sizeof(*out->faces));
	out->edges = calloc(out->nedge, sizeof(*out->edges));

	memcpy(out->vertices, in->vertices,
		out->nvert * sizeof(*out->vertices));
	memcpy(out->faces, in->faces,
		out->nface * sizeof(*out->faces));
	memcpy(out->edges, in->edges,
		out->nedge * sizeof(*out->edges));
}


/**
 * Subdivide a dome.  Previous values in output dome are freed.
 *
 * \parm in	input dome
 * \parm out	result dome
 * \param f	tesselation factor, 2 or more.
 */
void
tesselate(const Dome *in, Dome *out, int f)
{
	int	nv, ne, nf;
	int	fcount = 0, vcount = 0, ecount = 0;
	int	i;
	Edge	*edge;

	/* New # faces is old # faces * f² */
	nf = in->nface * f * f;

	/* Each face has three edges, with a full sphere, each edge is
	 * shared with two faces, but this may not be a full sphere. */
	ne = nf * 3;

	/* Each face has three vertices, each vertex shared
	 * among three or more faces.
	 */
	nv = nf * 3;

	out->vertices = malloc(nv * sizeof(*out->vertices));
	out->nvert = nv;
	out->faces = malloc(nf * sizeof(*out->faces));
	out->nface = nf;
	out->edges = edge = malloc(ne * sizeof(*out->edges));
	out->nedge = ne;
	for(i = ne; --i >= 0; edge++->name = NULL);

	/* I'm too lazy to find a proper way to populate the data
	 * structures with the vertices in edges in any kind of
	 * optimal manner, so I'll just settle for brute force
	 * matching.  For each vertex I want to add, I'll see if it's
	 * already there.
	 */


	for(i = 0; i < in->nface; ++i)
	  tesselate1(in, in->faces[i], f, out, &fcount, &vcount, &ecount);

	out->nface = fcount;
	out->nedge = ecount;
	out->nvert = vcount;
}



/**
 * Tesselate one triangle
 */
static	void
tesselate1(const Dome *in, Face face, int f, Dome *out,
	int *fcount, int *vcount, int *ecount)
{
	int	i,j;
	Point	*p0, *p1, *p2;
	Point	v01, v02, v12;
	Point	n0, n1, n2;
	double	frac = 1./f;
	Point	v01f, v02f, v12f;

	p0 = &in->vertices[face[0]];
	p1 = &in->vertices[face[1]];
	p2 = &in->vertices[face[2]];

	ptSub(p1, p0, &v01);
	ptSub(p2, p0, &v02);
	ptSub(p2, p1, &v12);

	ptMul(&v01, frac, &v01f);
	ptMul(&v02, frac, &v02f);
	ptMul(&v12, frac, &v12f);

	for(i=0; i < f; ++i)
	  for(j=0; j <= i; ++j)
	  {
	    /* top of new triangle = p0 + v01f*i + v12f*j */
	    if( i == 0 && j == 0 )
	      n0 = *p0;
	    else {
	      ptMul(&v01, i*frac, &n0); ptAdd(&n0, p0, &n0);
	      ptMul(&v12, j*frac, &n2); ptAdd(&n0, &n2, &n0);
	    }

	    /* 2nd point = top + v01f */
	    if( i == f-1 && j == 0 )
	      n1 = *p1;
	    else
	      ptAdd(&n0, &v01f, &n1);

	    /* 3rd point = top + v02f */
	    if( i == f-1 && j == f-1 )
	      n2 = *p2;
	    else
	      ptAdd(&n0, &v02f, &n2);

	    add_face(&n0, &n1, &n2, out, fcount, ecount, vcount);

	    if( j < i ) {
	      ptAdd(&n0, &v12f, &n1);
	      add_face(&n0, &n2, &n1, out, fcount, ecount, vcount);
	    }
	  }
}



/**
 * Append a face to the dome's face list.  This may involve adding
 * vertices and edges too.
 */
static	void
add_face(Point *p0, Point *p1, Point *p2, Dome *dome,
			int *nf, int *ne, int *nv)
{
	int	v0, v1, v2;

	if( *nf >= dome->nface ) {
	  fprintf(stderr, "Internal error: out of faces in tesselate\n");
	  return;
	}

	v0 = match_vtx(p0, dome, nv);
	v1 = match_vtx(p1, dome, nv);
	v2 = match_vtx(p2, dome, nv);
	dome->faces[*nf][0] = v0;
	dome->faces[*nf][1] = v1;
	dome->faces[*nf][2] = v2;
	++(*nf);

	(void) match_edge(v0, v1, dome, ne);
	(void) match_edge(v1, v2, dome, ne);
	(void) match_edge(v2, v0, dome, ne);
}



/**
 * Return the index of the vertex that matches this one.  Insert if
 * needed.
 */
static	int
match_vtx(Point *vtx, Dome *dome, int *nv)
{
	int	i;

	for(i=0; i < *nv; ++i)
	  if( ptDist(vtx, &dome->vertices[i]) < .1 )
	    return i;

	if( *nv >= dome->nvert ) {
	  fprintf(stderr, "Internal error: out of vertices in tesselate\n");
	  return 0;
	}

	dome->vertices[*nv] = *vtx;
	return (*nv)++;
}


/**
 * Return the index of the edge that matches this one.
 *
 * \param v0, v1  vertices to match
 * \param dome    dome containing vertices and edges
 */
int
find_edge(int v0, int v1, const Dome *dome)
{
	int	i;

	for(i=0; i < dome->nedge; ++i)
	  if( (dome->edges[i].v0 == v0 && dome->edges[i].v1 == v1)  ||
	      (dome->edges[i].v1 == v0 && dome->edges[i].v0 == v1) )
	    return i;

	return -1;
}


/**
 * Return the index of the edge that matches this one.  Insert if needed.
 *
 * \param v0, v1  vertices to match
 * \param dome    dome containing vertices and edges
 * \param ne      number of edges in dome.
 */
int
match_edge(int v0, int v1, const Dome *dome, int *ne)
{
	int	i;

	if( (i = find_edge(v0, v1, dome)) >= 0 )
	  return i;

	if( *ne >= dome->nedge ) {
	  fprintf(stderr, "Internal error: out of edges in match_edge\n");
	  return 0;
	}

	dome->edges[*ne].v0 = v0;
	dome->edges[*ne].v1 = v1;
	return (*ne)++;
}



/**
 * Normalize a vertex so it has a specified radius from the center.
 */
void
normalize_vertex(Point *vtx, double r, double frac)
{
	double	l;
	double	frac1 = 1. - frac;

	l = len(vtx);
	if( l > 0. && frac > 0. )
	{
	  l = frac/l + frac1;
	  l *= r;
	  vtx->x *= l; vtx->y *= l; vtx->z *= l;
	}
}



/**
 * Same as normalize_vertex(), but only normalizes in X-Y plane.
 */
void
normalize_vertex_h(Point *vtx, double r)
{
	double	l, r2;

	if( vtx->z >= r || vtx->z <= -r ) {
	  normalize_vertex(vtx, r, 1.);
	  return;
	}

	l = lenXY(vtx);
	if( l > 0. )
	{
	  r2 = sqrt(r*r - vtx->z*vtx->z);
	  l = r2/l;
	  vtx->x *= l; vtx->y *= l;
	}
}



/**
 * Normalize all the vertices in a dome to the specified sphere.  For fun,
 * we allow this to be done only part-way, via the 'frac' argument.
 */
void
normalize_dome(Dome *dome, double r, double frac)
{
	int	i;
	Point	*vtx;

	vtx = dome->vertices;
	for(i=0; i < dome->nvert; ++i, ++vtx)
	  normalize_vertex(vtx, r, frac);

	edge_lengths(dome);
}


/**
 * Scale entire dome in-place by the specified scale factor.
 */
void
scale_dome(Dome *dome, double f)
{
	int	i;

	for(i=0; i < dome->nvert; ++i)
	{
	  dome->vertices[i].x *= f;
	  dome->vertices[i].y *= f;
	  dome->vertices[i].z *= f;
	}
	edge_lengths(dome);
}



/**
 * Compute the lengths of all the edges in the dome.
 */
void
edge_lengths(Dome *dome)
{
	int	i;
	Edge	*edge;
	Point	*p0, *p1;

	for(i=0, edge = dome->edges; i < dome->nedge; ++i, ++edge)
	{
	  p0 = &dome->vertices[edge->v0];
	  p1 = &dome->vertices[edge->v1];
	  edge->len = ptDist(p0, p1);
	}
}



/**
 * Remove this face from the face list.
 */
void
delete_face(Dome *dome, int face)
{
	if( face < 0 || face >= dome->nface )
	  return;
	if( face < dome->nface - 1)
	  memmove(&dome->faces[face], &dome->faces[face+1],
	    sizeof(dome->faces[0]) * (dome->nface - face - 1));
	--dome->nface;
}



/**
 * Remove this edge from the edge list.
 */
void
delete_edge(Dome *dome, int edge)
{
	if( edge < 0 || edge >= dome->nedge )
	  return;
	if( edge < dome->nedge - 1)
	  memmove(&dome->edges[edge], &dome->edges[edge+1],
	    sizeof(dome->edges[0]) * (dome->nedge - edge - 1));
	--dome->nedge;
}



/**
 * Remove this vertex from the vertices list.  This will also remove
 * all faces and edges connected to this vertex.
 */
void
delete_vertex(Dome *dome, int vertex)
{
	int	i;
	Face	*face;
	Edge	*edge;

	if( vertex < 0 || vertex >= dome->nvert )
	  return;

	/* This is a little more involved, since we also have to delete
	 * all the faces and edges that reference this vertex.
	 */

	face = dome->faces + dome->nface - 1;
	for(i=dome->nface - 1; i >= 0; --i, --face)
	{
	  if( (*face)[0] == vertex || (*face)[1] == vertex ||
	      (*face)[2] == vertex )
	    delete_face(dome, i);
	}

	edge = dome->edges + dome->nedge - 1;
	for(i=dome->nedge - 1; i >= 0; --i, --edge)
	{
	  if( edge->v0 == vertex || edge->v1 == vertex )
	    delete_edge(dome, i);
	}


	/* Next pass:  renumber all the vertex references in the
	 * face and edge lists.
	 */

	for(i=0, face = dome->faces; i < dome->nface; ++i, ++face)
	{
	  if( face[0][0] > vertex ) --face[0][0];
	  if( face[0][1] > vertex ) --face[0][1];
	  if( face[0][2] > vertex ) --face[0][2];
	}

	for(i=0, edge = dome->edges; i < dome->nedge; ++i, ++edge)
	{
	  if( edge->v0 > vertex ) --edge->v0;
	  if( edge->v1 > vertex ) --edge->v1;
	}

	if( vertex < dome->nvert - 1)
	  memmove(&dome->vertices[vertex], &dome->vertices[vertex+1],
	    sizeof(dome->vertices[0]) * (dome->nvert - vertex - 1));
	--dome->nvert;
}




/**
 * Build a vertex list from a dome.  Input vertex array must be of same
 * length as dome's vertex list.
 */
void
build_vertex_list(Dome *dome, Vertex vertices[])
{
	int	i;
	Vertex	*vtx;
	Edge	*edge;

	for(i=0, vtx = vertices; i < dome->nvert; ++i, ++vtx)
	  vtx->nedge = 0;

	for(i=0, edge = dome->edges; i < dome->nedge; ++i, ++edge) {
	  vtx = &vertices[edge->v0];
	  vtx->edges[vtx->nedge++] = i;
	  vtx = &vertices[edge->v1];
	  vtx->edges[vtx->nedge++] = i;
	}
}




/**
 * Bottom vertices can be identified by the fact that they
 * have less than 5 attached edges.   TODO: find a better way; this
 * will fail on domes based on octahedrons or tetrahedrons.
 */
static inline int
is_bottom(Vertex *vtx)
{
	return vtx->nedge < 5;
}



/**
 * Find all vertices in a dome which lie along the bottom.
 *
 * Returns list of bottom vertex indices, and number of vertices.
 *
 * \param dome		dome to be examined
 * \param vertices	list of vertices to be examined
 * \param bottom	returned list of indices into vertex list.
 *
 * \return	number of vertex indices returned.
 */
int
find_bottom_vertices(Dome *dome, Vertex vertices[], int bottom[])
{
	int	i, nbottom;
	Vertex	*vtx;

	/* Find the bottom vertices */
	for(i=0, vtx = vertices, nbottom = 0; i < dome->nvert; ++i, ++vtx)
	  if( is_bottom(vtx) )
	    bottom[nbottom++] = i;

	return nbottom;
}



/**
 * Return the maximum Z coordinate in the dome.
 */
double
dome_max(Dome *dome)
{
	double	zmax;
	int	i;

	zmax = dome->vertices[0].z;
	for(i=0; i < dome->nvert; ++i)
	{
	  if( dome->vertices[i].z > zmax )
	    zmax = dome->vertices[i].z;
	}

	return zmax;
}



/**
 * Return the minimum Z coordinate in the dome.
 */
double
dome_min(Dome *dome)
{
	double	zmin;
	int	i;

	zmin = dome->vertices[0].z;
	for(i=0; i < dome->nvert; ++i)
	{
	  if( dome->vertices[i].z < zmin )
	    zmin = dome->vertices[i].z;
	}

	return zmin;
}


static	int
intcompare(const void *aa, const void *bb)
{
	return *(int *)bb - *(int *)aa;
}


/**
 * Delete the bottom row of vertices.
 */
void
delete_row(Dome *dome)
{
	Vertex	vertices[dome->nvert];		/* WARNING: GNU extension */
	int	bottom[dome->nvert];
	int	nbottom;
	int	i;

	build_vertex_list(dome, vertices);

	nbottom = find_bottom_vertices(dome, vertices, bottom);

	/* OK, we want to delete the vertices from high to low so
	 * that their renumbering doesn't mess things up.
	 */

	qsort(bottom, nbottom, sizeof(bottom[0]), intcompare);

	for(i=0; i < nbottom; ++i)
	  delete_vertex(dome, bottom[i]);
}


/**
 * This function attempts to level the bottom of the dome.
 * Certain key quasi-vertical struts along the bottom are lengthened
 * so that all bottom vertices are the same Z value.
 */
void
level_bottom(Dome *dome)
{
	Vertex	vertices[dome->nvert];		/* WARNING: GNU extension */
	int	bottom[dome->nvert];
	int	nbottom;
	int	i, j;
	double	zmin;

	build_vertex_list(dome, vertices);

	nbottom = find_bottom_vertices(dome, vertices, bottom);

#ifdef	COMMENT
	printf("nbottom = %d:\n", nbottom);
	for(i=0; i < nbottom; ++i)
	  printf("%d: %d, (%g,%g,%g)\n",
	    i, bottom[i], dome->vertices[bottom[i]].x,
	    dome->vertices[bottom[i]].y, dome->vertices[bottom[i]].z);
#endif	/* COMMENT */


	/* Find the lowest vertex; that will be our target */
	zmin = dome->vertices[0].z;
	for(i=0; i < nbottom; ++i)
	{
	  j = bottom[i];
	  if( dome->vertices[j].z < zmin )
	    zmin = dome->vertices[j].z;
	}

#ifdef	COMMENT
	printf("zmin = %lg\n", zmin);
#endif	/* COMMENT */

	/* Level the floor */
	for(i=0; i < nbottom; ++i)
	  dome->vertices[bottom[i]].z = zmin;


#ifdef	COMMENT
	/* We could simply set the Z coordinate of this vertex
	 * to zmin and leave it at that, but that would affect the lengths
	 * of the struts to the sides as well as the ones above.  In the
	 * interest of not introducing too many different strut lengths,
	 * we adjust the x,z position accordingly.  This is done by
	 * adjusting the vertex's radius the minimum amount to make one
	 * of its two neighbor struts the right length again.
	 *
	 * This is only an approximate algorithm, as I lack the knowledge
	 * of engineering to compute this properly.
	 */

	for(i=0; i < nbottom; ++i)
	{
	  int v0, v1;
	  static Point ctr = {0,0,0};
	  Point p, p0, p1, *pj;
	  Edge	*e0, *e1;
	  j = bottom[i];
	  pj = &dome->vertices[j];
	  find_neighbors(dome, j, vertices, &e0, &e1, &v0, &v1);
	  p0 = *pj;
	  find_best_point(&ctr, &dome->vertices[j], &dome->vertices[v0], &p0);
	  p1 = *pj;
	  find_best_point(&ctr, &dome->vertices[j], &dome->vertices[v1], &p1);
	  if( ptDist(pj, &p0) < ptDist(pj, &p1) )
	    *pj = p0;
	  else
	    *pj = p1;
	}
#endif	/* COMMENT */

	edge_lengths(dome);
}


/**
 * This function attempts raises the dome so that all vertices are
 * above 0,0,0
 */
void
raise_dome(Dome *dome)
{
	int	i;
	double	zmin;

	zmin = dome_min(dome);

	for(i=0; i < dome->nvert; ++i)
	  dome->vertices[i].z -= zmin;
}


/**
 * This function attempts to level the bottom of the dome.
 * Certain key quasi-vertical struts along the bottom are lengthened
 * so that all bottom vertices are the same Z value.
 */
void
level_bottom2(Dome *dome)
{
	Vertex	vertices[dome->nvert];		/* WARNING: GNU extension */
	int	bottom[dome->nvert];
	int	nbottom;
	int	i, j;

	build_vertex_list(dome, vertices);

	nbottom = find_bottom_vertices(dome, vertices, bottom);

#ifdef	COMMENT
	printf("nbottom = %d:\n", nbottom);
	for(i=0; i < nbottom; ++i)
	  printf("%d: %d, (%g,%g,%g)\n",
	    i, bottom[i], dome->vertices[bottom[i]].x,
	    dome->vertices[bottom[i]].y, dome->vertices[bottom[i]].z);
#endif	/* COMMENT */


	/* We could simply set the Y coordinate of this vertex
	 * to ymin and leave it at that, but that would affect the lengths
	 * of the struts to the sides as well as the ones above.  In the
	 * interest of not introducing too many different strut lengths,
	 * we adjust the x,z position accordingly.  This is done by
	 * adjusting the vertex's radius the minimum amount to make one
	 * of its two neighbor struts the right length again.
	 *
	 * This is only an approximate algorithm, as I lack the knowledge
	 * of engineering to compute this properly.
	 */

	for(i=0; i < nbottom; ++i)
	{
	  int v0, v1;
	  static Point ctr = {0,0,0};
	  Point *p, p0, p1;
	  Edge	*e0, *e1;
	  j = bottom[i];
	  p = &dome->vertices[j];
	  find_neighbors(dome, j, vertices, &e0, &e1, &v0, &v1);
	  p0 = *p;
	  ctr.z = p0.z;
	  find_best_point(&ctr, &dome->vertices[j], &dome->vertices[v0],
		e0->len, &p0);
	  p1 = *p;
	  ctr.z = p1.z;
	  find_best_point(&ctr, &dome->vertices[j], &dome->vertices[v1],
		e1->len, &p1);
	  if( ptDist(p, &p0) < ptDist(p, &p1) )
	    *p = p0;
	  else
	    *p = p1;
	}

	edge_lengths(dome);
}



/**
 * Utility:  given an edge and a vertex, return the other vertex on that edge.
 */
static inline int
other_vertex(int v0, Edge *edge)
{
	if( edge->v0 == v0 )
	  return edge->v1;
	else
	  return edge->v0;
}


/**
 * Utility:  Return the edges and matching vertices of the two edges sharing
 * the dome bottom with this edge.
 */
static	void
find_neighbors(Dome *dome, int v, Vertex *vertices,
	Edge **e0, Edge **e1, int *v0, int *v1)
{
	int	i,j;
	Vertex	*vtx = &vertices[v];

	for(i=0; i < vtx->nedge; ++i)
	{
	  j = other_vertex(v, &dome->edges[vtx->edges[i]]);
	  if( is_bottom(&vertices[j]) ) {
	    *v0 = j;
	    *e0 = &dome->edges[vtx->edges[i]];
	    break;
	  }
	}
	for(++i; i < vtx->nedge; ++i)
	{
	  j = other_vertex(v, &dome->edges[vtx->edges[i]]);
	  if( is_bottom(&vertices[j]) ) {
	    *v1 = j;
	    *e1 = &dome->edges[vtx->edges[i]];
	    break;
	  }
	}
}



/**
 * Find a point on the line l0-l1, len distance from p2.  There are usually
 * two solutions, so return the one closest to the original value in p.
 */
static	void
find_best_point(Point *l0, Point *l1, Point *p2, double len, Point *p)
{
	Point	c, r0, r1;
	double	base, dist;

	/* Step one: find the point on the line closest to p2 */
	projectPointOnLine(l0, l1, p2, 0., &c);

	/* Step two: get the base of this right triangle */
	base = ptDist(&c, p2);
	if( base >= len ) {	/* degenerate case */
	  *p = c;
	  return;
	}

	/* Step three: the height, given hypotenuse = len */
	dist = sqrt(len*len - base*base);

	/* Step four: project this distance from c in both directions */
	(void) projectPointOnLine(l0, l1, p2, dist, &r0);
	(void) projectPointOnLine(l0, l1, p2, -dist, &r1);

	if( ptDist(p, &r0) < ptDist(p, &r1) )
	  *p = r0;
	else
	  *p = r1;
}


#ifdef	COMMENT
static	void
find_new_point(Point *p, Point *p0, Point *p1, double L0, double L1)
{
	/* Given two points, P0, P1, two strut lengths, L0, L1, and
	 * distance between P0,P1 = d:
	 *
	 * P0        P1
	 * * x  |    *
	 *  \   |y  /
	 *   \  |  /
	 *  L0\ | /L1
	 *     \|/
	 *      *
	 *
	 * x = (d² + L0² - L1²)/2d  and  y² = L0² - x²
	 */


	L0 = ptDist(p, p0);
	L1 = ptDist(p, p1);

	/* TODO: think about this. */
}
#endif	/* COMMENT */



/**
 * Find the projection of a point on a line, with optional offset.
 *
 * \param l0	 first endpoint of line
 * \param l1	 second endpoint of line
 * \param pt	 point to project onto line
 * \param offset optional offset; returned point will be this much
 *		 further away from l0.
 * \param rval	 Returned point.
 *
 * \returns 1 on success, 0 on degenerate line
 *
 * TODO: can we get the distance for free?
 */

int
projectPointOnLine(const Point *l0, const Point *l1, const Point *pt, double offset, Point *rval)
{
	double	vx, vy, vz;
	double	vx2, vy2, vz2;
	double	len, dist;

	/* Construct the l0 => l1 vector, and normalize it. */
	vx = l1->x - l0->x;
	vy = l1->y - l0->y;
	vz = l1->z - l0->z;

	if( (len = sqrt(vx*vx + vy*vy + vz*vz)) == 0. )
	  return 0;

	len = 1./len;
	vx *= len; vy *= len; vz *= len;

	/* Construct the l0 => pt vector */
	vx2 = pt->x - l0->x;
	vy2 = pt->y - l0->y;
	vz2 = pt->z - l0->z;

	/* Use dot-product to project this vector on the l0 => l1 vector.
	 * This gives us a distance, to which we add the offset.
	 */
	dist = vx*vx2 + vy*vy2 + vz*vz2 + offset;

	/* And now compute the projected point. */
	rval->x = l0->x + dist * vx;
	rval->y = l0->y + dist * vy;
	rval->z = l0->z + dist * vz;

	return 1;
}




static	void	compute_strut(Dome *dome, Edge *edge, StrutInfo *strut);
static	double	bend_angle(Point *p0, Point *p1);
static	int	match_strut(StrutInfo *a, StrutInfo *b, double, double);
static	int	strutsort(const void *, const void *);

/**
 * Label all of the struts by length and bend angles.  Any struts with
 * lengths within .1% and angles within 2° are considered identical.
 * The most common strut is 'A', and so on.
 *
 * \param dome	Dome to be processed
 * \param info	Returned strut list; free with free()
 *
 * \returns number of struts
 */
int
assign_labels(Dome *dome, StrutInfo **info, double at, double lt)
{
	StrutInfo *rval = malloc(dome->nedge * sizeof(*rval));
	Edge	*edge;
	int	ninfo = 0;
	int	i, j;
	char	name[10];

	for(i=0, edge = dome->edges; i < dome->nedge; ++i, ++edge)
	{
	  StrutInfo tmp;
	  compute_strut(dome, edge, &tmp);
	  tmp.count = 1;

	  for(j=0; j < ninfo && !match_strut(&rval[j], &tmp, at, lt); ++j);
	  if( j >= ninfo )
	    rval[ninfo++] = tmp;
	  else
	    ++rval[j].count;
	}

	qsort(rval, ninfo, sizeof(*rval), strutsort);

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

	/* Copy the strut names to the edges */
	for(i=0, edge = dome->edges; i < dome->nedge; ++i, ++edge)
	{
	  StrutInfo tmp;
	  compute_strut(dome, edge, &tmp);

	  for(j=0; j < ninfo && !match_strut(&rval[j], &tmp, at, lt); ++j);
	  edge->name = strdup(rval[j].name);
	}

	if( info != NULL )
	  *info = rval;
	else
	  free(rval);

	return ninfo;
}



static	void
compute_strut(Dome *dome, Edge *edge, StrutInfo *strut)
{
	Point	*p0 = &dome->vertices[edge->v0];
	Point	*p1 = &dome->vertices[edge->v1];

	strut->len = edge->len;
	strut->a0 = bend_angle(p0, p1);
	strut->a1 = bend_angle(p1, p0);
}


/**
 * Compute the bend angle at the p0 end of this strut.
 */
static	double
bend_angle(Point *p0, Point *p1)
{
	Point	v0, v1;
	double	l;

	/* Form two vectors:  from p0 to 0 and from p0 to p1.  Take
	 * the dot-product of the two vectors and that's the cosine
	 * of the angle.
	 */

	v0.x = -p0->x; v0.y = -p0->y; v0.z = -p0->z;
	l = len(&v0);
	if( l == 0. )
	  return 0.;
	l = 1./l;
	v0.x *= l; v0.y *= l; v0.z *= l;

	v1.x = p1->x - p0->x; v1.y = p1->y - p0->y; v1.z = p1->z - p0->z;
	l = len(&v1);
	if( l == 0. )
	  return 0.;
	l = 1./l;
	v1.x *= l; v1.y *= l; v1.z *= l;

	return 90. - 180./M_PI * acos( v0.x*v1.x + v0.y*v1.y + v0.z*v1.z );
}


/**
 * Return true if these two struts match within a small margin of error.
 */
static	int
match_strut(StrutInfo *a, StrutInfo *b, double at, double lt)
{
	double l0 = 1. - lt;
	double l1 = 1. + lt;

	if( b->len > a->len * l1 || b->len < a->len * l0 )
	  return 0;

	if( b->a0 >= a->a0 - at && b->a0 <= a->a0 + at  &&
	    b->a1 >= a->a1 - at && b->a1 <= a->a1 + at )
	      return 1;

	if( b->a0 >= a->a1 - at && b->a0 <= a->a1 + at  &&
	    b->a1 >= a->a0 - at && b->a1 <= a->a0 + at )
	      return 1;

	return 0;
}


static	int
strutsort(const void *aa, const void *bb)
{
	const StrutInfo *a = aa;
	const StrutInfo *b = bb;
	int i;

	/* Sort by frequency */
	if( (i = b->count - a->count) != 0 )
	  return i;

	/* Then by length */
	return b->len > a->len ? 1 : -1;
}


/**
 * Take a point, and project it to the "flattened" x-y coordinate system.
 */
void
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


#ifdef	DEBUG
void
compare_domes(const char *file, int line, Dome *old, Dome *new)
{
	int	i;
	Point	*ov, *nv;
	Edge	*oe, *ne;

	if( old->nvert != new->nvert ) {
	  printf("old: %d vertices, new: %d vertices\n",
	    old->nvert, new->nvert);
	  return;
	}

	if( old->nedge != new->nedge ) {
	  printf("old: %d edges, new: %d edges\n",
	    old->nedge, new->nedge);
	  return;
	}

	ov = old->vertices;
	nv = new->vertices;
	for(i=0; i < old->nvert; ++i) {
	  if( nv->x != ov->x || nv->y != ov->y || nv->z != ov->z ) {
	    printf("vtx %2d: (%g,%g,%g) => (%g,%g,%g)\n",
	      i, ov->x, ov->y, ov->z, nv->x, nv->y, nv->z);
	  }
	  ++nv;
	  ++ov;
	}

	oe = old->edges;
	ne = new->edges;
	for(i=0; i < old->nedge; ++i, ++oe, ++ne) {
	  if( ne->v0 != oe->v0 || ne->v1 != oe->v1 )
	    printf("edge %3d: %d-%d => %d-%d\n",
	      i, oe->v0, oe->v1, ne->v0, ne->v1);
	  if( ne->len != oe->len )
	    printf("edge %3d: %g => %g\n", i, oe->len, ne->len);
	}
}
#endif	/* DEBUG */
