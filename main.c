#ifndef lint
static const char rcsid[] =
	"$Id: main.c,v 1.1 2004/11/12 07:13:22 efalk Rel $";
#endif

/**********
 *
 *
 *	 @@@@  @@@@@   @@@   @@@@    @@@   @   @  @@@@@  
 *	@      @      @   @  @   @  @   @  @@ @@  @      
 *	@ @@@  @@@    @   @  @   @  @   @  @ @ @  @@@    
 *	@   @  @      @   @  @   @  @   @  @ @ @  @      
 *	 @@@@  @@@@@   @@@   @@@@    @@@   @ @ @  @@@@@  
 *
 *
 *	This is the main program for the "dome" geodesic dome
 *	design program.
 *
 *
 *	Edward A. Falk
 *	falk@sourceforge.net
 *
 *	November, 2004
 *
 *
 *
 **********/

static const char usage[] =
"\n"
"dome -- design geodesic domes\n"
"\n"
"usage: dome [options] [domefile]\n"
"	-radius N	specify radius (12)\n"
"\n"
"See instructions for more details.\n"
;

#define	MOUSE_SCALE	.1	/* degrees/pixel movement */
#define	SPHERE_SIZE	.02	/* spheres used to represent vertices */
#define	SPHERE_DETAIL	8	/* level of detail for spheres */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <GL/glut.h>

#include "utils.h"
#include "dome.h"



typedef enum {FACES, EDGES, VERTICES, NORMALIZE, NORMALIZING, ROTATING,
		DELETING, DELETE_ROW, PRINT, READ, RAISE_VERT, EXTEND,
		EXTEND2, LEVEL, RAISE} Command;

	float	radius = 12.;

static	Command	runMode = ROTATING;

static	int	mainMenu;

static	GLint	Frames = 0;

static	int	running = 0;
static	int	visible = 0;

static	GLfloat	view_rotx = 3.0, view_roty = 2.0, view_rotz = 0.0;
static	GLfloat	anim_rotz = 0.;

static	int	mx, my;
static	int	target_vertex;
static	int	rotating = 0;
static	int	raising = 0;
static	int	extending = 0;
static	int	show_coords = 0;
static	double	old_value;
static	Point	old_point;
static	double	ymin;

static	double	aspect;

static	GLuint	selectBuf[512];

static	char	*ifilename = NULL;

static	void	set_view(void);
static	void	draw(void);
static	void	draw_picture();
static	void	view_element(int element);

static	void	set_running(int r);

static	void	timeCB(int value);
static	void	normalize_start();
static	void	normalizeTimeCB(int value);
static	void	key(unsigned char k, int x, int y);
static	void	special(int k, int x, int y);
static	void	mouse(int button, int state, int x, int y);
static	void	drag(int x, int y);
static	void	motion(int x, int y);
static	void	reshape(int width, int height);
static	void 	visibleCB(int vis);
static	void	menuCB(int value);
static	void	init();

#ifdef	COMMENT
static	void	processHits (GLint hits, GLuint buffer[]);
#endif	/* COMMENT */
static	int	findSelected(GLint hits, GLuint buffer[]);


#define	streq(a,b)	(strcmp(a,b) == 0)


int
main(int argc, char *argv[])
{
	FILE	*ifile;

	glutInit(&argc, argv);

	for(++argv; --argc > 0; ++argv)
	{
	  if( streq(*argv, "-help") || streq(*argv, "--help") ) {
	    printf(usage);
	    exit(0);
	  }
	  else if( streq(*argv, "-radius") && --argc > 0)
	    radius = atof(*++argv);
	  else if( ifilename == NULL )
	    ifilename = *argv;
	}

	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

	glutInitWindowSize(600, 600);
	glutCreateWindow("Dome");

	init();

	glutDisplayFunc(draw);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMotionFunc(drag);
	glutPassiveMotionFunc(motion);
	glutVisibilityFunc(visibleCB);

	mainMenu = glutCreateMenu(menuCB);
	glutAddMenuEntry("Faces (f)", FACES);
	glutAddMenuEntry("Edges (e)", EDGES);
	glutAddMenuEntry("Vertices (v)", VERTICES);
	glutAddMenuEntry("Normalize (n)", NORMALIZE);
	glutAddMenuEntry("Rotate", ROTATING);
	glutAddMenuEntry("Delete Elements", DELETING);
	glutAddMenuEntry("Delete Bottom row", DELETE_ROW);
	glutAddMenuEntry("Raise Vertex", RAISE_VERT);
	glutAddMenuEntry("Extend Vertex", EXTEND);
	glutAddMenuEntry("Extend Vertex radial", EXTEND2);
	glutAddMenuEntry("Normalize Vertex", NORMALIZING);
	glutAddMenuEntry("Level bottom (l)", LEVEL);
	glutAddMenuEntry("Raise dome", RAISE);
	glutAddMenuEntry("Save", PRINT);
	glutAddMenuEntry("Read", READ);

	glutAttachMenu(GLUT_RIGHT_BUTTON);


	init_dome(radius);

	if( ifilename != NULL && (ifile = fopen(ifilename, "r")) != NULL )
	{
	  delete_dome(&cdome);
	  read_dome(&cdome, ifile);
	  /* TODO: calling edge_lengths will override the lengths
	   * in the file; this may be undesirable
	   */
	  edge_lengths(&cdome);
	  render_dome(&cdome);
	  fclose(ifile);
	}

	glutMainLoop();

	return 0;
}



/*---------------------------------------------------------------------------
 * Set viewing transformations, lighting and so on.
 */

static void
init()
{
	static GLfloat lightpos[4] = {-5.0, 5.0, 10.0, 0.0};
	GLUquadricObj *qobj;

	glEnable(GL_DEPTH_TEST);

	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);


	/* Create a few common primitives. */

	qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_FILL);
	gluQuadricNormals(qobj, GLU_SMOOTH);

	sphereList = glGenLists(1);
	glNewList(sphereList, GL_COMPILE);
	  gluSphere(qobj, radius * SPHERE_SIZE, SPHERE_DETAIL, SPHERE_DETAIL);
	glEndList();

	glEnable(GL_NORMALIZE);
}


static	void
set_view(void)
{
	glFrustum(-1.0, 1.0, -aspect, aspect, 5.0, 500.0);
	glTranslatef(0.0, 0.0, -10.0 * radius);
}



/*---------------------------------------------------------------------------
 * Draw the dome
 */
static void
draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_picture();

	glutSwapBuffers();

	Frames++;
}



/*---------------------------------------------------------------------------
 * Do the actual drawing (or picking)
 */
static void
draw_picture(void)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	  /* This first rotation switches OpenGL coordinates (Y up) to
	   * world coordinates (Z up).
	   */
	  glRotatef(-90., 1.0, 0.0, 0.0);
	  glRotatef(view_rotx, 1.0, 0.0, 0.0);
	  glRotatef(view_roty, 0.0, 1.0, 0.0);
	  glRotatef(view_rotz, 0.0, 0.0, 1.0);

	  if( show_verts )
	    glCallList(domeList);

	  if( show_edges )
	    glCallList(domeList+1);

	  if( show_faces )
	    glCallList(domeList+2);

#ifdef	COMMENT
	  glClear(GL_DEPTH_BUFFER_BIT);

	  draw_element(0);
	  draw_element(2);
	  draw_element(9);
#endif	/* COMMENT */


	  if( show_coords ) {
	    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
	    draw_sphere(0., 0., 0.);

	    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
	    draw_sphere(4., 0., 0.);

	    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
	    draw_sphere(0., 4., 0.);

	    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
	    draw_sphere(0., 0., 4.);
	  }

	glPopMatrix();
}



/*---------------------------------------------------------------------------
 * Do the actual drawing (or picking)
 */
static void
view_element(int element)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	  glRotatef(-90., 1.0, 0.0, 0.0);
	  glRotatef(view_rotx, 1.0, 0.0, 0.0);
	  glRotatef(view_roty, 0.0, 1.0, 0.0);
	  glRotatef(view_rotz, 0.0, 0.0, 1.0);

	  draw_element(element);

	glPopMatrix();
}



/*---------------------------------------------------------------------------
 * Given cursor coordinates x,y, find the primitive(s) underlying the
 * pick window.  Results in selectBuf.
 */
static	int
pick(int x, int y)
{
	GLint hits;
	GLint viewport[4];

	glSelectBuffer (NA(selectBuf), selectBuf);
	(void) glRenderMode (GL_SELECT);

	glInitNames();
	glPushName(0);


	glMatrixMode(GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity();
	glGetIntegerv (GL_VIEWPORT, viewport);
	/*  create 5x5 pixel picking region near cursor location */
	gluPickMatrix ((double)x, (double)(viewport[3]-y), 3., 3., viewport);
	set_view();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_picture();
	glutSwapBuffers();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix ();

	glFlush ();

	hits = glRenderMode (GL_RENDER);
	return hits;
}



#ifdef	COMMENT
/*---------------------------------------------------------------------------
 * processHits prints out the contents of the
 * selection array.
 */

static	void
processHits (GLint hits, GLuint buffer[])
{
	unsigned int i, j;
	GLuint names, *ptr;

	printf ("hits = %d\n", hits);
	ptr = (GLuint *) buffer;
	for (i = 0; i < hits; i++) {	/*  for each hit  */
	  names = *ptr;
	  printf (" number of names for this hit = %d\n", names); ptr++;
	  printf(" z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
	  printf(" z2 is %g\n", (float) *ptr/0x7fffffff); ptr++;
	  printf ("   names are ");
	  for (j = 0; j < names; j++) { /*  for each name */
	     printf ("%d ", *ptr);
	     ptr++;
	  }
	  printf ("\n");
	}
}
#endif	/* COMMENT */


/*---------------------------------------------------------------------------
 * Decide which item in the hit buffer was selected.  Preference goes
 * first to vertices (10000), then edges (5000), then faces.  Topmost item
 * is returned.
 */
static	int
findSelected(GLint hits, GLuint buffer[])
{
	int	bestType = 0, best = -1;
	double	z1, z2, topZ = 99.;
	int	i;
	GLuint	nnames, *ptr;
	int	name;

	ptr = (GLuint *) buffer;
	for (i = 0; i < hits; i++) {	/*  for each hit  */
	  nnames = *ptr++;
	  z1 = (double)*ptr++ / 0x7fffffff;
	  z2 = (double)*ptr++ / 0x7fffffff;
	  name = *ptr;
	  ptr += nnames;
	  if( name >= VTX_BASE ) bestType = VTX_BASE;
	  else if( name >= EDGE_BASE && bestType == 0 ) bestType = EDGE_BASE;
	  if( name >= bestType && z1 < topZ ) {
	    best = name;
	    topZ = z2;
	  }
	}

	return best;
}



static	void
menuCB(int value)
{
	switch( (Command)value ) {
	  case ROTATING:
	  case DELETING:
	  case RAISE_VERT:
	  case EXTEND:
	  case EXTEND2:
	  case NORMALIZING:
	    runMode = value;
	    break;
	  case FACES:
	    show_faces = !show_faces;
	    break ;
	  case EDGES:
	    show_edges = !show_edges;
	    break ;
	  case VERTICES:
	    show_verts = !show_verts;
	    break ;
	  case NORMALIZE:
	    normalize_start();
	    break;
	  case DELETE_ROW:
	    delete_row(&cdome);
	    render_dome(&cdome);
	    break;
	  case LEVEL:
	    level_bottom(&cdome);
	    render_dome(&cdome);
	    break;
	  case RAISE:
	    raise_dome(&cdome);
	    render_dome(&cdome);
	    break;
	  case PRINT:
	    {
	      FILE *ofile;
	      edge_lengths(&cdome);
	      write_dome(&cdome, (ofile = fopen("dome.data", "w")));
	      fclose(ofile);
	    }
	    break;
	  case READ:
	    {
	      FILE *ifile;
	      if( (ifile = fopen("dome.data", "r")) == NULL )
		fprintf(stderr, "cannot open dome.data, %s\n",
		  strerror(errno));
		else {
		  delete_dome(&cdome);
		  read_dome(&cdome, ifile);
		  fclose(ifile);
		  render_dome(&cdome);
		}
	    }
	}
}



static	void
timeCB(int value)
{
	/* generate frame of animation */
	view_rotz += anim_rotz;

	glutPostRedisplay();

	if( running )
	  glutTimerFunc(33, timeCB, 0);
}




static	void
normalize_now()
{
	normalize_cmd(radius, 1.);
}


static	double	normalize_f;

static	void
normalize_start()
{
	normalize_f = 0.;
	glutTimerFunc(33, normalizeTimeCB, 0);
}


static	void
normalizeTimeCB(int value)
{
	normalize_f += .01;
	if( normalize_f > 1. )
	  return;

	copy_dome(&tdome, &cdome);
	normalize_cmd(radius, normalize_f);

	glutPostRedisplay();

	glutTimerFunc(33, normalizeTimeCB, 0);
}



static	void
set_running(int r)
{
	if( r != running )
	{
	  if( !running )
	    glutTimerFunc(33, timeCB, 0);
	  running = r;
	}
}



/* change view angle, exit upon ESC */
/* ARGSUSED1 */
static void
key(unsigned char k, int x, int y)
{
	switch (k) {
	  case '2': tesselate_cmd(2, radius) ; break;
	  case '3': tesselate_cmd(3, radius) ; break;
	  case '4': tesselate_cmd(4, radius) ; break;
	  case '5': tesselate_cmd(5, radius) ; break;
	  case '6': tesselate_cmd(6, radius) ; break;
	  case '7': tesselate_cmd(7, radius) ; break;
	  case '8': tesselate_cmd(8, radius) ; break;
	  case '9': tesselate_cmd(9, radius) ; break;
	  case 'z': view_roty += 4.0; break;
	  case 'Z': view_roty -= 4.0; break;
	  case 'r': anim_rotz = view_rotx = view_roty = view_rotz = 0.; break;
	  case 'R':
	    set_running(!running);
	    break;
	  case 'a': anim_rotz += .05; break;
	  case 'A': anim_rotz -= .05; break;
	  case 'v': show_verts = !show_verts; break ;
	  case 'e': show_edges = !show_edges; break ;
	  case 'f': show_faces = !show_faces; break ;
	  case 'c': show_coords = !show_coords; break ;
	  case 'n': normalize_start(); break;
	  case 'N': normalize_now(); break;
	  case 'l':
	    level_bottom(&cdome);
	    render_dome(&cdome);
	    break;
	  case 'L':
	    level_bottom2(&cdome);
	    render_dome(&cdome);
	    break;
	  case 27:  /* Escape */
	    exit(0);
	    break;
	  default:
	    return;

	  case 'T':
	    glMatrixMode(GL_PROJECTION);
	    glTranslatef(0., 0.0, -.5);
	    break;
	}

	glutPostRedisplay();
}




static void
special(int k, int x, int y)
{
	switch (k) {
	  case GLUT_KEY_UP: view_rotx += 4.0; break;
	  case GLUT_KEY_DOWN: view_rotx -= 4.0; break;
	  case GLUT_KEY_LEFT: view_rotz += 4.0; break;
	  case GLUT_KEY_RIGHT: view_rotz -= 4.0; break;
	  default: return;
	}
	glutPostRedisplay();
}




static	void
mouse(int button, int state, int x, int y)
{
	int	hits, selection;

	mx = x; my = y;
	switch( button ) {
	  case GLUT_LEFT_BUTTON:
	    switch( runMode ) {
	      case ROTATING:
		rotating = state == GLUT_DOWN ;
		break;

	      case DELETING:
		if( state == GLUT_DOWN ) {
		  hits = pick(x, y);
		  selection = findSelected (hits, selectBuf);
		  glClear(GL_DEPTH_BUFFER_BIT);
		  view_element(selection);
		  delete_element(&cdome, selection);
		  glutSwapBuffers();
		}
		else
		  draw();
		break;

	      case NORMALIZING:
		if( state == GLUT_DOWN ) {
		  hits = pick(x, y);
		  selection = findSelected (hits, selectBuf);
		  if( selection >= VTX_BASE ) {
		    normalize_vertex(&cdome.vertices[selection-VTX_BASE],
		    	radius, 1.);
		  }
		  edge_lengths(&cdome);
		  render_dome(&cdome);
		  glClear(GL_DEPTH_BUFFER_BIT);
		  view_element(selection);
		  glutSwapBuffers();
		}
		else
		  draw();
		break;

	      case RAISE_VERT:
		if( state == GLUT_DOWN )
		{
		  hits = pick(x, y);
		  selection = findSelected (hits, selectBuf);
		  if( selection >= VTX_BASE ) {
		    target_vertex = selection-VTX_BASE;
		    raising = 1;
		    old_value = cdome.vertices[target_vertex].z;
		    ymin = dome_min(&cdome);
		  }
		}
		else
		  raising = 0;
		break;

	      case EXTEND:
	      case EXTEND2:
		if( state == GLUT_DOWN )
		{
		  hits = pick(x, y);
		  selection = findSelected (hits, selectBuf);
		  if( selection >= VTX_BASE ) {
		    target_vertex = selection-VTX_BASE;
		    extending = 1;
		    old_point = cdome.vertices[target_vertex];
		  }
		}
		else
		  extending = 0;
		break;

	      default:
		break;
	    }
	    break;

	  case GLUT_MIDDLE_BUTTON:
	    rotating = state == GLUT_DOWN ;
	    break;

	  default:
	    break;
	}
}



static void
drag(int x, int y)
{
	if (rotating) {
	  view_rotz += MOUSE_SCALE * (x - mx);
	  view_rotx += MOUSE_SCALE * (y - my);
	  mx = x;
	  my = y;
	}

	else if( raising )
	{
	  char buffer[20];
	  Point *vtx = &cdome.vertices[target_vertex];
	  vtx->z = old_value + .01 * (my - y);
	  sprintf(buffer, "hgt = %.2f", vtx->z - ymin);
	  glutSetWindowTitle(buffer);

	  edge_lengths(&cdome);
	  render_dome(&cdome);
	}

	else if( extending )
	{
	  char buffer[20];
	  Point *vtx = &cdome.vertices[target_vertex];
	  static Point c = {0,0,0};
	  c.z = runMode == EXTEND ? vtx->z : 0.;
	  projectPointOnLine(&c, &old_point, &old_point, .02 * (mx - x), vtx);
	  sprintf(buffer, "R = %.2f", lenXY(vtx));
	  glutSetWindowTitle(buffer);

	  edge_lengths(&cdome);
	  render_dome(&cdome);
	}

	glutPostRedisplay();
}



static void
motion(int x, int y)
{
	int	hits, selection;

	switch( runMode ) {
	  case DELETING:
	  case RAISE_VERT:
	  case EXTEND:
	  case EXTEND2:
	  case NORMALIZING:
	    hits = pick(x, y);
	    selection = findSelected (hits, selectBuf);
#ifdef	COMMENT
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    draw_picture();
	    glClear(GL_DEPTH_BUFFER_BIT);
#endif	/* COMMENT */
	    glDrawBuffer(GL_FRONT);
	    glClear(GL_DEPTH_BUFFER_BIT);
	    view_element(selection);
	    glDrawBuffer(GL_BACK);
#ifdef	COMMENT
	    glutSwapBuffers();
#endif	/* COMMENT */
	    break;

	  default:
	    break;
	}
}



/* new window size or exposure */
static void
reshape(int width, int height)
{
	aspect = (GLfloat) height / (GLfloat) width;

	glViewport(0, 0, (GLint) width, (GLint) height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	set_view();
}



/*---------------------------------------------------------------------------
 * Called when the drawing window becomes visible or invisible.
 */
static void 
visibleCB(int vis)
{
#ifdef	COMMENT
	if (!running && vis == GLUT_VISIBLE)
	  set_running(1);
#endif	/* COMMENT */

	if( running && vis != GLUT_VISIBLE )
	  set_running(0);

	visible = vis;
}
