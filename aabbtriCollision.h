/********************************************************/
/* AABB-triangle overlap test code                      */
/* by Tomas Akenine-Möller                              */
/* Function: int triBoxOverlap(float boxcenter[3],      */
/*          float boxhalfsize[3],float triverts[3][3]); */
/* History:                                             */
/*   2001-03-05: released the code in its first version */
/*   2001-06-18: changed the order of the tests, faster */
/*                                                      */
/* Acknowledgement: Many thanks to Pierre Terdiman for  */
/* suggestions and discussions on how to optimize code. */
/* Thanks to David Hunt for finding a ">="-bug!         */
/********************************************************/

#ifndef AABBTRI_H
#define AABBTRI_H


#include <math.h>
#include <stdio.h>
#include<glm/glm.hpp>

inline void findMinMax(float x0, float x1, float x2, float& min, float& max) {
	min = x0; max = x0;
	if (x1 < min)
		min = x1;
	if (x1 > max)
		max = x1;
	if (x2 < min)
		min = x2;
	if (x2 > max)
		max = x2;
}

bool planeBoxOverlap(glm::vec3 normal, glm::vec3 vert, glm::vec3 maxBox)
{
	int q;
	glm::vec3 vmin, vmax;
	float v;

	v = vert.x;
	if (normal.x > 0.0f)
	{
		vmin.x = -maxBox.x - v;
		vmax.x = maxBox.x - v;
	}
	else
	{
		vmin.x = maxBox.x - v;
		vmax.x = -maxBox.x - v;
	}

	v = vert.y;
	if (normal.y > 0.0f)
	{
		vmin.y = -maxBox.y - v;
		vmax.y = maxBox.y - v;
	}
	else
	{
		vmin.y = maxBox.y - v;
		vmax.y = -maxBox.y - v;
	}

	v = vert.z;
	if (normal.z > 0.0f)
	{
		vmin.z = -maxBox.z - v;
		vmax.z = maxBox.z - v;
	}
	else
	{
		vmin.z = maxBox.z - v;
		vmax.z = -maxBox.z - v;
	}

	if (glm::dot(normal, vmin) > 0.0f) 
		return false;
	if (glm::dot(normal, vmax) >= 0.0f) 
		return true;

	return false;
}


/*======================== X-tests ========================*/
inline bool axisTestX01(float a, float b, float fa, float fb, const glm::vec3& v0, const glm::vec3& v2,
	const glm::vec3& boxhalfsize)
{
	float p0 = a * v0.y - b * v0.z;
	float p2 = a * v2.y - b * v2.z;
	float min, max;
	if (p0 < p2)
	{
		min = p0;
		max = p2;
	}
	else
	{
		min = p2;
		max = p0;
	}
	float rad = fa * boxhalfsize.y + fb * boxhalfsize.z;
	if (min > rad || (max < -rad))
		return false;

	return true;
}
inline bool axisTestX2(float a, float b, float fa, float fb, const glm::vec3& v0, const glm::vec3& v1,
	const glm::vec3& boxhalfsize)
{
	float p0 = a * v0.y - b * v0.z;
	float p1 = a * v1.y - b * v1.z;
	float min, max;
	if (p0 < p1)
	{
		min = p0;
		max = p1;
	}
	else
	{
		min = p1;
		max = p0;
	}
	float rad = fa * boxhalfsize.y + fb * boxhalfsize.z;
	if (min > rad || (max < -rad))
		return false;

	return true;
}

/*======================== Y-tests ========================*/

inline bool axisTestY02(float a, float b, float fa, float fb, const glm::vec3& v0, const glm::vec3& v2,
	const glm::vec3& boxhalfsize)
{
	float p0 = -a * v0.x - b * v0.z;
	float p2 = -a * v2.x - b * v2.z;
	float min, max;
	if (p0 < p2)
	{
		min = p0;
		max = p2;
	}
	else
	{
		min = p2;
		max = p0;
	}
	float rad = fa * boxhalfsize.x + fb * boxhalfsize.z;
	if (min > rad || (max < -rad))
		return false;

	return true;
}

inline bool axisTestY1(float a, float b, float fa, float fb, const glm::vec3& v0, const glm::vec3& v1,
	const glm::vec3& boxhalfsize)
{
	float p0 = -a * v0.x - b * v0.z;
	float p1 = -a * v1.x - b * v1.z;
	float min, max;
	if (p0 < p1)
	{
		min = p0;
		max = p1;
	}
	else
	{
		min = p1;
		max = p0;
	}
	float rad = fa * boxhalfsize.x + fb * boxhalfsize.z;
	if (min > rad || (max < -rad))
		return false;

	return true;
}

/*======================== Z-tests ========================*/

inline bool axisTestZ12(float a, float b, float fa, float fb, const glm::vec3& v1, const glm::vec3& v2,
	const glm::vec3& boxhalfsize)
{
	float p1 = a * v1.x - b * v1.y;
	float p2 = a * v2.x - b * v2.y;
	float min, max;
	if (p2 < p1)
	{
		min = p2;
		max = p1;
	}
	else
	{
		min = p1;
		max = p2;
	}
	float rad = fa * boxhalfsize.x + fb * boxhalfsize.y;
	if (min > rad || (max < -rad))
		return false;

	return true;
}

inline bool axisTestZ0(float a, float b, float fa, float fb, const glm::vec3& v0, const glm::vec3& v1,
	const glm::vec3& boxhalfsize)
{
	float p0 = a * v0.x - b * v0.y;
	float p1 = a * v1.x - b * v1.y;
	float min, max;
	if (p0 < p1)
	{
		min = p0;
		max = p1;
	}
	else
	{
		min = p1;
		max = p0;
	}
	float rad = fa * boxhalfsize.x + fb * boxhalfsize.y;
	if (min > rad || (max < -rad))
		return false;

	return true;
}

bool triBoxOverlap(glm::vec3 boxcenter, glm::vec3 boxhalfsize, glm::vec3 triverts[3])
{
	/*    use separating axis theorem to test overlap between triangle and box */
	/*    need to test for overlap in these directions: */
	/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
	/*       we do not even need to test these) */
	/*    2) normal of the triangle */
	/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
	/*       this gives 3x3=9 more tests */

	glm::vec3 v0, v1, v2;
	//   float axis[3];
	float min, max, p0, p1, p2, rad, fex, fey, fez;		// -NJMP- "d" local variable removed

	/* This is the fastest branch on Sun */
	/* move everything so that the boxcenter is in (0,0,0) */
	v0 = triverts[0] - boxcenter;
	v1 = triverts[1] - boxcenter;
	v2 = triverts[2] - boxcenter;

	/* compute triangle edges */
	glm::vec3 e0 = v1 - v0;      /* tri edge 0 */
	glm::vec3 e1 = v2 - v1;      /* tri edge 1 */
	glm::vec3 e2 = v0 - v2;      /* tri edge 2 */

	/* Bullet 3:  */
	/*  test the 9 tests first (this was faster) */
	fex = fabsf(e0.x);
	fey = fabsf(e0.y);
	fez = fabsf(e0.z);

	if (!axisTestX01(e0.z, e0.y, fez, fey, v0, v2, boxhalfsize))
		return false;
	if (!axisTestY02(e0.z, e0.x, fez, fex, v0, v2, boxhalfsize))
		return false;
	if (!axisTestZ12(e0.y, e0.x, fey, fex, v1, v2, boxhalfsize))
		return false;


	fex = fabsf(e1.x);
	fey = fabsf(e1.y);
	fez = fabsf(e1.z); 
	if (!axisTestX01(e1.z, e1.y, fez, fey, v0, v2, boxhalfsize))
		return false;
	if (!axisTestY02(e1.z, e1.x, fez, fex, v0, v2, boxhalfsize))
		return false;
	if (!axisTestZ0(e1.y, e1.x, fey, fex, v0, v1, boxhalfsize))
		return false;

	fex = fabsf(e2.x);
	fey = fabsf(e2.y);
	fez = fabsf(e2.z);

	if (!axisTestX2(e2.z, e2.y, fez, fey, v0, v1, boxhalfsize))
		return false;
	if (!axisTestY1(e2.z, e2.x, fez, fex, v0, v1, boxhalfsize))
		return false;
	if (!axisTestZ12(e2.y, e2.z, fey, fex, v1, v2, boxhalfsize))
		return false;

	/* Bullet 1: */
	/*  first test overlap in the {x,y,z}-directions */
	/*  find min, max of the triangle each direction, and test for overlap in */
	/*  that direction -- this is equivalent to testing a minimal AABB around */
	/*  the triangle against the AABB */

	/* test in X-direction */
	findMinMax(v0.x, v1.x, v2.x, min, max);
	if (min > boxhalfsize.x || max < -boxhalfsize.x) 
		return false;

	/* test in Y-direction */
	findMinMax(v0.y, v1.y, v2.y, min, max);
	if (min > boxhalfsize.y || max < -boxhalfsize.y) 
		return false;

	/* test in Z-direction */
	findMinMax(v0.z, v1.z, v2.z, min, max);
	if (min > boxhalfsize.z || max < -boxhalfsize.z) 
		return false;

	/* Bullet 2: */
	/*  test if the box intersects the plane of the triangle */
	/*  compute plane equation of triangle: normal*x+d=0 */
	glm::vec3 normal = glm::cross(e0, e1);
	// -NJMP- (line removed here)
	if (!planeBoxOverlap(normal, v0, boxhalfsize)) 
		return false;	// -NJMP-

	return true;   /* box and triangle overlaps */
}

#endif