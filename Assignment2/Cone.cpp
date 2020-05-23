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
        t1 = -1.0;
    }
    if (fabs(t2) < 0.001) t2 = -1.0;

    glm::vec3 pt1 = p0 + t1 * dir;
    if (pt1.y > (center.y + height)) {
        glm::vec3 pt2 = p0 + t2 * dir;
        if (pt2.y > (center.y + height)) {
            return -1;
        }
        else {
            return t2;
        }
    }

    return (t1 < t2) ? t1 : t2;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the cylinder.
*/
glm::vec3 Cone::normal(glm::vec3 p)
{   
    float alpha = atan((p.x - center.x) / (p.z - center.z));
    float theta = atan(radius / height);
    float x = sin(alpha) * cos(theta);
    float y = sin(theta);
    float z = cos(alpha) * cos(theta);
    glm::vec3 n = glm::vec3(x, y, z);
    return n;
}
