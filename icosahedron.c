/* $Id$ */

/* Compute the coordinates of a unit icosahedron.  This is a little
 * difficult to follow without a diagram, but see
 * http://www.vb-helper.com/tutorial_platonic_solids.html
 */

#include <stdio.h>
#include <math.h>

#define	DEG	180./M_PI

typedef struct {double x,y,z;} Point;

static	double	computeIcos(double S);
static	void	showlen(Point *p1, Point *p2);

int
main()
{
	double	radius;

	radius = computeIcos(1.);
	radius = computeIcos(1./radius);

	return 0;
}


	/* Compute an icosahedron with edge length S */

static double
computeIcos(double S)
{
	double	R, t1,t2,t3,t4;
	double	Y1, Y2;
	double	H, H1, H2;
	Point	a,b,c,d,e,f,g,h,i,j,k,l;

	t1 = M_PI * 2. / 5.;
	t2 = M_PI/2. - t1;
	t3 = t1 - t2;
	t4 = t1/2;

	R = S/2./sin(t4);

	H = sqrt(R*R - S*S/4);
	H1 = sqrt(S*S - R*R);
	H2 = sqrt((H+R)*(H+R) - H*H);

	printf("S=%g, t1=%g, t2=%g, t3=%g, t4=%g, R=%g, H1=%g, H2=%g\n",
	  S, t1*DEG, t2*DEG, t3*DEG, t4*DEG, R, H1, H2);

	Y2 = (H2-H1)/2.;
	Y1 = Y2 + H1;

	a.x = 0.; a.y = Y1, a.z = 0.;
	b.x = R; b.y = Y2, b.z = 0.;
	c.x = R*sin(t2); c.y = Y2, c.z = R*cos(t2);
	d.x = -R*cos(t4); d.y = Y2, d.z = R*sin(t4);
	e.x = -R*cos(t4); e.y = Y2, e.z = -R*sin(t4);
	f.x = R*sin(t2); f.y = Y2, f.z = -R*cos(t2);
	g.x = -R; g.y = -Y2, g.z = 0.;
	h.x = -R*sin(t2); h.y = -Y2, h.z = -R*cos(t2);
	i.x = R*cos(t4); i.y = -Y2, i.z = -R*sin(t4);
	j.x = R*cos(t4); j.y = -Y2, j.z = R*sin(t4);
	k.x = -R*sin(t2); k.y = -Y2, k.z = R*cos(t2);
	l.x = 0.; l.y = -Y1, l.z = 0.;

	printf("/* coordinates for icosahedron with edge length %g */\n", S);
	printf("/* Radius = %g\n", Y1);
	printf("    {%g,%g,%g},		/* a */\n", a.x, a.y, a.z);
	printf("    {%g,%g,%g},		/* b */\n", b.x, b.y, b.z);
	printf("    {%g,%g,%g},		/* c */\n", c.x, c.y, c.z);
	printf("    {%g,%g,%g},		/* d */\n", d.x, d.y, d.z);
	printf("    {%g,%g,%g},		/* e */\n", e.x, e.y, e.z);
	printf("    {%g,%g,%g},		/* f */\n", f.x, f.y, f.z);
	printf("    {%g,%g,%g},		/* g */\n", g.x, g.y, g.z);
	printf("    {%g,%g,%g},		/* h */\n", h.x, h.y, h.z);
	printf("    {%g,%g,%g},		/* i */\n", i.x, i.y, i.z);
	printf("    {%g,%g,%g},		/* j */\n", j.x, j.y, j.z);
	printf("    {%g,%g,%g},		/* k */\n", k.x, k.y, k.z);
	printf("    {%g,%g,%g},		/* l */\n", l.x, l.y, l.z);

#ifdef	COMMENT
	showlen(&a,&b);
	showlen(&a,&c);
	showlen(&a,&d);
	showlen(&a,&e);
	showlen(&a,&f);

	showlen(&b,&c);
	showlen(&c,&d);
	showlen(&d,&e);
	showlen(&e,&f);
	showlen(&f,&b);

	showlen(&b,&j);
	showlen(&j,&c);
	showlen(&c,&k);
	showlen(&k,&d);
	showlen(&d,&g);
	showlen(&g,&e);
	showlen(&e,&h);
	showlen(&h,&f);
	showlen(&f,&i);
	showlen(&i,&b);

	showlen(&g,&h);
	showlen(&h,&i);
	showlen(&i,&j);
	showlen(&j,&k);
	showlen(&k,&g);

	showlen(&g,&l);
	showlen(&h,&l);
	showlen(&i,&l);
	showlen(&j,&l);
	showlen(&k,&l);
#endif	/* COMMENT */

	return Y1;
}



static	void
showlen(Point *p1, Point *p2)
{
	double dx = p1->x - p2->x;
	double dy = p1->y - p2->y;
	double dz = p1->z - p2->z;
	printf("%g\n", sqrt(dx*dx+dy*dy+dz*dz));
}
