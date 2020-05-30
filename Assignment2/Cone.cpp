/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cone class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cone.h"
#include <math.h>

/**
* Cone's intersection method.  The input is a ray.
*/
float Cone::intersect(glm::vec3 p0, glm::vec3 dir)
{
    glm::vec3 vdif = p0 - center; 
    
    float rhRatio = radius / height;
    float a = dir.x * dir.x + dir.z * dir.z - rhRatio * rhRatio * dir.y * dir.y;
    float b = 2 * vdif.x * dir.x + 2 * vdif.z * dir.z + rhRatio * rhRatio * 2 * height * dir.y - rhRatio * rhRatio *  2 * vdif.y * dir.y;
    float c = vdif.x * vdif.x + vdif.z * vdif.z - rhRatio * rhRatio * (height - vdif.y) * (height - vdif.y);

    float delta = b * b - 4 * a * c;

    if (fabs(delta) < 0.001) return -1.0;
    if (delta < 0.0) return -1.0;

    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);

    if (fabs(t1) < 0.001)
    {
        if (t2 > 0) return t2;
        else t1 = -1.0;
    }
    if (fabs(t2) < 0.001) t2 = -1.0;

    float tA; // closer point
    float tB; 

    if (t1 > t2) {
        tB = t1;
        tA = t2;
    }
    else {
        tB = t2;
        tA = t1;
    }

    glm::vec3 ptA = p0 + tA * dir;
    glm::vec3 ptB = p0 + tB * dir;
    if ((ptA.y >= (center.y) && ptA.y <= (center.y + height))) {
        return tA;
    }
    else if ((ptB.y >= center.y && ptB.y <= (height + center.y))) {
        return tB;
    }
    else {
        return -1;
    }
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the cone.
*/
glm::vec3 Cone::normal(glm::vec3 p)
{   
    float alpha = atan((p.x - center.x) / (p.z - center.z));
    float theta = atan(radius / height);
    float x = sin(alpha) * cos(theta);
    float y = sin(theta);
    float z = cos(alpha) * cos(theta);
    glm::vec3 n = glm::vec3(x, y, z);
    n = glm::normalize(n);
    return n;
}
