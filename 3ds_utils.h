#ifndef	DOME_3DS_H
#define	DOME_3DS_H


#include <lib3ds/node.h>

extern	Lib3dsNode *mesh_node(const char *name);
extern	Lib3dsNode *mesh_node_i(const char *name);
extern	void	node_translate(Lib3dsNode *, double tx, double ty, double tz);
extern	void	node_scale(Lib3dsNode *, double sx, double sy, double sz);
extern	void	node_rotate(Lib3dsNode *, double, double, double, double angle);
extern	void	node_rotatev(Lib3dsNode *, Lib3dsVector axis, double angle);
extern	void	create_camera(Lib3dsFile *file, Lib3dsVector position,
			Lib3dsVector target, double near_range,
			double far_range, double fov, const char *name);


#endif	/* DOME_3DS_H */
