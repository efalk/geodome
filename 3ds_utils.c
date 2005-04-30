#ifndef lint
static const char rcsid[] =
	"$Id: 3ds_utils.c,v 1.1 2004/11/12 07:13:22 efalk Exp $";
#endif


/**
 * A few functions to make life easier when working with lib3ds.
 */

#include <string.h>
#include <math.h>

#include <lib3ds/file.h>
#include <lib3ds/mesh.h>
#include <lib3ds/material.h>
#include <lib3ds/vector.h>
#include <lib3ds/camera.h>
#include <lib3ds/quat.h>

#include "3ds_utils.h"

static Lib3dsMesh *full_rotation(Lib3dsMesh *, Point *, int,
	int, const char *);
static Lib3dsMesh *partial_rotation(Lib3dsMesh *, Point *, int,
	double, double, int, const char *);


/**
 * Create a node for this mesh
 */
Lib3dsNode *
mesh_node(const char *name)
{
	Lib3dsNode	*node;

	node = lib3ds_node_new_object();
	strncpy(node->name, name, sizeof(node->name));
	node->parent_id = LIB3DS_NO_PARENT;
	return node;
}



/**
 * Create a node for this mesh with scale factor 1
 */
Lib3dsNode *
mesh_node_i(const char *name)
{
	Lib3dsNode	*node = mesh_node(name);;

	node_scale(node, 1.,1.,1.);
	return node;
}


/**
 * Add one track element to a node to translate it.
 */
void
node_translate(Lib3dsNode *node, double tx, double ty, double tz)
{
	node->data.object.pos_track.keyL = lib3ds_lin3_key_new();
	node->data.object.pos_track.keyL->value[0] = tx;
	node->data.object.pos_track.keyL->value[1] = ty;
	node->data.object.pos_track.keyL->value[2] = tz;
}



/**
 * Add one track element to a node to scale it.
 */
void
node_scale(Lib3dsNode *node, double sx, double sy, double sz)
{
	node->data.object.scl_track.keyL = lib3ds_lin3_key_new();
	node->data.object.scl_track.keyL->value[0] = sx;
	node->data.object.scl_track.keyL->value[1] = sy;
	node->data.object.scl_track.keyL->value[2] = sz;
}



/**
 * Add one track element to a node to rotate it.
 *
 * \param node	    Node to be rotated
 * \param vx,vy,vz  Axis of rotation
 * \param angle     Rotation angle, radians
 */
void
node_rotate(Lib3dsNode *node, double vx, double vy, double vz, double angle)
{
	Lib3dsVector	axis;
	axis[0] = vx; axis[1] = vy; axis[2] = vz;
	node_rotatev(node, axis, angle);
}



/**
 * Add one track element to a node to rotate it.
 *
 * \param node	Node to be rotated
 * \param axis	Axis of rotation
 * \param angle Rotation angle, radians
 */
void
node_rotatev(Lib3dsNode *node, Lib3dsVector axis, double angle)
{
	node->data.object.rot_track.keyL = lib3ds_quat_key_new();
	memcpy(node->data.object.rot_track.keyL->axis, axis,
		sizeof(Lib3dsVector));
	node->data.object.rot_track.keyL->angle = angle;
}



/**
 * Take a convex polygon and triangulate it, writing the triangles into
 * the given face list.
 *
 * \param nvert	   Number of vertices in the polygon
 * \param pts	   Array[nvert] of vertex indices.
 * \param faces	   Face array to be filled in.
 * \param material Material name.
 *
 * \returns	   Number of faces filled in.
 *
 * \note	results undefined with non-convex polygons.
 */
int
triangulate_convex_polygon(int nvert, int pts[], Lib3dsFace *faces,
	const char *material)
{
	int	nf = nvert - 2;
	int	i;

	for(i=0; i < nf; ++i) {
	  strncpy(faces->material, material, sizeof(faces->material));
	  faces->points[0] = pts[0];
	  faces->points[1] = pts[i+1];
	  faces->points[2] = pts[i+2];
	  ++faces;
	}

	return nf;
}



/**
 * Create a camera and insert it into the file.
 *
 * \param file		3ds file.
 * \param position	camera position
 * \param target	camera target
 * \param near_range	camera near range
 * \param far_range	camera far range
 * \param fov		camera field of view; 45° is a good value.
 * \param name		camera name
 */

void
create_camera(Lib3dsFile *file, Lib3dsVector position, Lib3dsVector target,
	double near_range, double far_range, double fov, const char *name)
{
	Lib3dsCamera *camera = lib3ds_camera_new(name);

	memcpy(camera->position, position, sizeof(camera->position));
	memcpy(camera->target, target, sizeof(camera->target));
	camera->near_range = near_range;
	camera->far_range = far_range;
	camera->fov = fov;
	lib3ds_file_insert_camera(file, camera);
}



/**
 * Generate a surface of rotation.  This is created by taking the specified
 * profile and rotating it about the Z axis.
 *
 * \param profile  Profile points.  x,z coordinates are significant, y ignored
 * \param nprofile Number of profile points
 * \param astart   start angle, degrees
 * \param astop    stop angle, degrees
 * \param nstep    number of steps of rotation
 * \param material material name
 *
 * \returns Generated mesh
 */

Lib3dsMesh *
surface_of_rotation(Point *profile, int nprofile, const char *name,
  double astart, double astop, int nstep, const char *material)
{
	Lib3dsMesh	*mesh;
	Lib3dsPoint	*points;
	Lib3dsFace	*faces;
	int		i, j;
	double		x, y, z;
	double		r, a;

	/* TODO: texture mapping */

	if( (mesh = lib3ds_mesh_new(name)) == NULL )
	  return NULL;

	if( astart == 0. && astop == 360. )
	  return full_rotation(mesh, profile, nprofile, nstep, material);
	else
	  return partial_rotation(mesh, profile, nprofile,
	  		astart, astop, nstep, material);

#ifdef	COMMENT
	if( !lib3ds_mesh_new_point_list(mesh, nprofile * (nstep+1)) )
	  return NULL;

	if( !lib3ds_mesh_new_face_list(mesh, nprofile * nstep * 2) )
	  return NULL;

	points = mesh->pointL;

	for(i=0; i < nprofile; ++i) {
	  r = profile->x;
	  a = astart;
	  z = profile->z;
	  for(j=0; j <= nstep; ++j) {
	    x = cos(a*M_PI/180.) * r;
	    y = sin(a*M_PI/180.) * r;
	    points->pos[0] = x; points->pos[1] = y; points->pos[2] = z;
	    ++points;
	    a += astep;
	  }
	  ++profile;
	}

	faces = mesh->faceL;
	for(i=0; i < nprofile; ++i)
	{
	  for(j=0; j < nstep; ++j) {
	    strncpy(faces->material, material->name, sizeof(faces->material));
	    faces->points[0] = i*(nstep+1) + j;
	    faces->points[1] = (i+1)*(nstep+1) + j;
	    faces->points[2] = i*(nstep+1) + j + 1;
	    faces->flags = 0x7;
	    faces->smoothing = 1;
	    ++faces;
	    strncpy(faces->material, material->name, sizeof(faces->material));
	    faces->points[0] = (i+1)*(nstep+1) + j;
	    faces->points[1] = (i+1)*(nstep+1) + j + 1;
	    faces->points[2] = i*(nstep+1) + j + 1;
	    faces->flags = 0x7;
	    faces->smoothing = 1;
	    ++faces;
	  }
	}

	return mesh;
#endif	/* COMMENT */
}


static Lib3dsMesh *
partial_rotation(Lib3dsMesh *mesh, Point *profile, int nprofile,
	double astart, double astop, int nstep, const char *material)
{
	Lib3dsPoint	*points;
	Lib3dsFace	*faces;
	int		i, j, idx;
	int		np;
	double		r, angle, da;
	double		x, y, z;

	lib3ds_mesh_new_point_list(mesh, nprofile * (nstep+1));
	points = mesh->pointL;

	da = (astart - astop)/nstep;
	np = nstep + 1;

	idx = 0;
	for(i=0; i < nprofile; ++i) {
	  angle = astart;
	  r = profile[i].x;
	  z = profile[i].z;
	  for(j=0; j < np; ++j) {
	    points[idx].pos[0] = r * cos(angle*M_PI/180.);
	    points[idx].pos[1] = r * sin(angle*M_PI/180.);
	    points[idx].pos[2] = z;
	    ++idx;
	    angle += da;
	  }
	}


	lib3ds_mesh_new_face_list(mesh, (nprofile-1) * nstep * 2);
	faces = mesh->faceL;

	idx = 0;
	for(i=0; i < nprofile-1; ++i) {
	  for(j=0; j < nstep; ++j) {
	    strncpy(faces[idx].material, material, sizeof(faces[idx].material));
	    faces[idx].points[0] = i * np + j;
	    faces[idx].points[1] = i * np + j + 1;
	    faces[idx].points[2] = (i+1) * np + j;
	    faces[idx].smoothing = 1;
	    ++idx;
	    strncpy(faces[idx].material, material, sizeof(faces[idx].material));
	    faces[idx].points[0] = i * np + j + 1;
	    faces[idx].points[1] = (i+1) * np + j + 1;
	    faces[idx].points[2] = (i+1) * np + j;
	    faces[idx].smoothing = 1;
	    ++idx;
	  }
	}

	return mesh;
}


static Lib3dsMesh *
full_rotation(Lib3dsMesh *mesh, Point *profile, int nprofile,
	int nstep, const char *material)
{
	Lib3dsPoint	*points;
	Lib3dsFace	*faces;
	int		i, j, idx;
	int		np;
	double		r, angle, da;
	double		x, y, z;

	lib3ds_mesh_new_point_list(mesh, nprofile * nstep);
	points = mesh->pointL;

	angle = 0.;
	da = 360./nstep;

	idx = 0;
	for(i=0; i < nprofile; ++i) {
	  r = profile[i].x;
	  z = profile[i].z;
	  for(j=0; j < nstep; ++j) {
	    points[idx].pos[0] = r * cos(angle*M_PI/180.);
	    points[idx].pos[1] = r * sin(angle*M_PI/180.);
	    points[idx].pos[2] = z;
	    ++idx;
	    angle += da;
	  }
	}


	lib3ds_mesh_new_face_list(mesh, (nprofile-1) * nstep * 2);
	faces = mesh->faceL;

	idx = 0;
	for(i=0; i < nprofile-1; ++i) {
	  for(j=0; j < nstep; ++j) {
	    strncpy(faces[idx].material, material, sizeof(faces[idx].material));
	    faces[idx].points[0] = (i+1) * nstep + (j+1)%nstep;
	    faces[idx].points[1] = i * nstep + (j+1)%nstep;
	    faces[idx].points[2] = (i+1) * nstep + j;
	    faces[idx].smoothing = 1;
	    ++idx;
	    strncpy(faces[idx].material, material, sizeof(faces[idx].material));
	    faces[idx].points[0] = i * nstep + (j+1)%nstep;
	    faces[idx].points[1] = i * nstep + j;
	    faces[idx].points[2] = (i+1) * nstep + j;
	    faces[idx].smoothing = 1;
	    ++idx;
	  }
	}

	return mesh;
}
