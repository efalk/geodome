#ifndef	UTILS_H
#define	UTILS_H

typedef	struct {float x,y,z;} Point;


#ifdef	DEBUG
#define	assert(e)	do { if(!(e)) assfail(#e, __FILE__, __LINE__);} while(0)
extern	void	assfail(const char *, const char *, int);
#else
#define	assert(e)
#endif

extern	void	error(char *msg);
extern	void	ptAdd(const Point *a, const Point *b, Point *out);
extern	void	ptSub(const Point *a, const Point *b, Point *out);
extern	void	ptMul(const Point *a, double m, Point *out);
extern	void	ptAvg(const Point *a, const Point *b, Point *out);
extern	void	ptAvg3(const Point *a, const Point *b, const Point *c,
			Point *out);
extern	void	ptInterp(const Point *a, const Point *b, double f, Point *out);
extern	double	ptDist(const Point *a, const Point *b);
extern	double	len(const Point *v);
extern	double	lenXY(const Point *v);
extern	void	normalize(Point *);
extern	void	normcrossprod(const Point *v1, const Point *v2, Point *out);

#define	NA(array)	(sizeof(array)/sizeof(array[0]))
#define	MIN(a,b)	((a)<(b)?(a):(b))
#define	MAX(a,b)	((a)>(b)?(a):(b))

#endif
