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

    if (fabs(t1) < 0.01) t1 = -1.0;
    if (fabs(t2) < 0.01) t2 = -1.0;

    if (t1 == -1 && t2 == -1) {
        return -1;
    }

    float tA;
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
   
    if (cap_ && ptA.y > (center.y + height) && ptB.y <= (center.y + height) && ptB.y >= center.y) {
        float t = (center.y + height - p0.y) / dir.y;
        if (fabs(t) < 0.001) {
            t = -1;
        }
        return t;
    } else if ((ptA.y >= (center.y) && ptA.y <= (center.y + height))) {
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
* Assumption: The input point p lies on the cylinder.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    glm::vec3 n;
    if (fabs(p.y - center.y - height) < 0.01 && cap_) {
        n = glm::vec3(0, 1, 0);
    }
    else {
        n = glm::vec3(p.x - center.x, 0, p.z - center.z);
    }
 
    n = glm::normalize(n);
    return n;
}


void Cylinder::setCap(bool flag) {
    cap_ = flag;
}

bool Cylinder::hasCap() {
    return cap_;
}
