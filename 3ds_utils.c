#ifndef lint
static const char rcsid[] =
	"$Id$";
#endif


/**
 * A few functions to make life easier when working with lib3ds.
 */

#ifdef	COMMENT
#include <lib3ds/mesh.h>
#include <lib3ds/material.h>
#include <lib3ds/light.h>
#include <lib3ds/node.h>
#include <lib3ds/matrix.h>
#endif	/* COMMENT */

#include <string.h>

#include <lib3ds/file.h>
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <lib3ds/camera.h>
#include <lib3ds/quat.h>

#include "3ds_utils.h"



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
