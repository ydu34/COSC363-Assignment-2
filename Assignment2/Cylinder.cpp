/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cyclinder class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

/**
* Cylinder's intersection method.  The input is a ray.
*/
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
{


    float xdif = p0.x - center.x;
    float zdif = p0.z - center.z;

    float a = dir.x * dir.x + dir.z * dir.z; 
    float b = 2 * (dir.x * xdif + dir.z * zdif);

    float c = xdif * xdif + zdif * zdif - radius * radius;
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

    glm::vec3 pt1 = p0 + t1 * dir;
    if (pt1.y > (center.y + height)) {
        glm::vec3 pt2 = p0 + t2 * dir;
        if (pt2.y > (center.y + height)) {
            return -1;
        }
        else if (cap_) {
            t2 = (center.y + height - p0.y) / dir.y;
            return t2;
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
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    glm::vec3 n = glm::vec3(p.x - center.x, 0, p.z - center.z);
    n = glm::normalize(n);
    return n;
}


void Cylinder::setCap(bool flag) {
    cap_ = flag;
}

bool Cylinder::hasCap() {
    return cap_;
}
