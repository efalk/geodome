#ifndef	DOME_H
#define	DOME_H

/* $Id$ */

#define	EDGE_BASE	5000
#define	VTX_BASE	10000

typedef	int	Face[3];

typedef	struct	{
	  int v0, v1;
	  double len;
	  char *name;
	} Edge;

typedef struct {
	  int	vtx;
	  int	nedge;
	  int	edges[6];
	} Vertex;

typedef	struct {
	  Point	*vertices;
	  int	nvert;
	  Edge	*edges;
	  int	nedge;
	  Face	*faces;
	  int	nface;
	} Dome;


/* Describe one group of identical struts. */
typedef	struct {
	  char *name;
	  double len;
	  float a0, a1;
	  int count;
	} StrutInfo;


extern	Dome	tdome, cdome;

extern const float red[4];
extern const float green[4];
extern const float blue[4];
extern const float white[4];
extern const float lgrey[4];
extern const float lgreya[4];

extern	float	radius;
extern	int	picking;
extern	int	show_verts;
extern	int	show_edges;
extern	int	show_faces;

extern	int	sphereList;
extern	int	domeList;
extern	int	cylList;

extern	Dome	f0ball;
extern	Dome	f0dome;

extern	Dome	octoball;
extern	Dome	octodome;

extern	void	init_dome(double);
extern	void	clear_dome(Dome *dome);
extern	void	delete_dome(Dome *dome);
extern	void	copy_dome_values(const Dome *in, Dome *out);
extern	void	copy_dome(const Dome *in, Dome *out);
extern	void	tesselate(const Dome *in, Dome *out, int f);
extern	void	normalize_vertex(Point *vtx, double r, double frac);
extern	void	normalize_dome(Dome *dome, double r, double frac);
extern	void	delete_row(Dome *dome);
extern	void	scale_dome(Dome *dome, double f);
extern	void	edge_lengths(Dome *dome);
extern	int	projectPointOnLine(Point *l0, Point *l1, Point *pt,
			double offset, Point *rval);
extern	int	projectPoint(Point *p0, Point *p1, double dist, Point *rval);
extern	void	level_bottom(Dome *dome);
extern	void	raise_dome(Dome *dome);
extern	void	level_bottom2(Dome *dome);
extern	int	assign_labels(Dome *dome, StrutInfo **, double, double);

extern	double	dome_max(Dome *dome);
extern	double	dome_min(Dome *dome);

extern	void	render_dome(Dome *);

extern	void	tesselate_cmd(int, double);
extern	int	normalize_cmd(double, double);

extern	void	draw_element(int element);
extern	void	draw_sphere(double, double, double);

extern	void	delete_face(Dome *dome, int face);
extern	void	delete_edge(Dome *dome, int edge);
extern	void	delete_vertex(Dome *dome, int vertex);
extern	void	delete_element(Dome *, int element);

extern	void	build_vertex_list(Dome *dome, Vertex vertices[]);
extern	int	find_bottom_vertices(Dome *dome, Vertex *, int *);

extern	void	write_dome(Dome *dome, FILE *ofile);
extern	void	read_dome(Dome *dome, FILE *ifile);


#endif	/* DOME_H */
