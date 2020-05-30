/*==================================================================================
* COSC 363  Computer Graphics (2020)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf, Lab08.pdf for details.
*===================================================================================
*/
#define _USE_MATH_DEFINES

#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "Cylinder.h"
#include "Cone.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>
#include "Plane.h"
#include "TextureBMP.h"
using namespace std;

const float WIDTH = 20.0;  // Width of the image plane in world units
const float HEIGHT = 20.0;  // Height of the image plane in world units
const float EDIST = 40.0;   // The number of cells (subdivisions of the image plane) along x and y directions
const int NUMDIV = 500;  // The number of cells along x and y directions
const int MAX_STEPS = 5;    // The number of levels (depth) of recursion (to be used next week) 
const float XMIN = -WIDTH * 0.5; // The boundary values of the image plane defined in terms of WIDTH and HEIGHT, such that the view axis passes through its centre
const float XMAX =  WIDTH * 0.5;    //^^
const float YMIN = -HEIGHT * 0.5;   //^^
const float YMAX =  HEIGHT * 0.5;   //^^

vector<SceneObject*> sceneObjects;

TextureBMP texture;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(1);						//Background colour = (1,1,1)
	glm::vec3 lightPos(10, 16, -5);					//Light's position
    glm::vec3 lightPos2(20, 18, 0);                  //Second light's position
	glm::vec3 color(0);
	SceneObject* obj;
    float ambientTerm = 0.2;
    float transparentRefractiveAmbientTerm = 0.7;


    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found


    if (ray.index == 0) {
        //Stripe pattern
        int stripeWidth = 5;
        int iz = (ray.hit.z) / stripeWidth;
        int k = abs(iz) % 2;
 
        int ix = (ray.hit.x) / stripeWidth;
        int j = abs(ix) % 2;
   
        //2 colors
        if (ray.hit.x < 0) {
            if (k == j) color = glm::vec3(1, 1, 1);
            else color = glm::vec3(0.4 , 0.4, 0.4);
        }
        else {
            if (k == j) color = glm::vec3(0.4, 0.4 , 0.4);
            else color = glm::vec3(1, 1, 1);
        }
    
        obj->setColor(color);
    
    }

    if (ray.index == 2) {
        glm::vec3 d = glm::normalize(ray.hit - ((Sphere*)obj)->getCenter());
        float u = 0.5 + (atan2(d.x, d.z) / (2 * M_PI));
        float v = 0.5 + (asin(d.y) / M_PI);
        color = texture.getColorAt(u, v);
        obj->setColor(color);
    }
    color = obj->getColor();

    glm::vec3 lighting = obj->lighting(lightPos, -ray.dir, ray.hit); // (Diffuse1 + Specular1) of first light source
    glm::vec3 lighting2 = obj->lighting(lightPos2, -ray.dir, ray.hit); // (Diffuse2 + Specular2) of second light source
 
    color = lighting + lighting2;


    // first light source shadows
    glm::vec3 lightVec = lightPos - ray.hit;
    Ray shadowRay(ray.hit, lightVec);
    
    shadowRay.closestPt(sceneObjects);  //Find the closest point of intersection on the shadow ray
    
    float lightDist = glm::length(lightVec);

    // second light source shadows
    glm::vec3 lightVec2 = lightPos2 - ray.hit;
    Ray shadowRay2(ray.hit, lightVec2);

    shadowRay2.closestPt(sceneObjects);  //Find the closest point of intersection on the shadow ray

    float lightDist2 = glm::length(lightVec2);

    bool transparentOrRefractive = false;

    if(shadowRay.index > -1 && shadowRay.dist < lightDist) {
        color = color - lighting2; 
        // If the object that was intersected by is a transparent object
        if (sceneObjects[shadowRay.index]->isTransparent() || sceneObjects[shadowRay.index]->isRefractive()) {
            transparentOrRefractive = true;
            if (sceneObjects[shadowRay.index]->isTransparent()) {
                color += sceneObjects[shadowRay.index]->getColor() * 0.05f;
            }
        } 
    }

    if (shadowRay2.index > -1 && shadowRay2.dist < lightDist2) {
        color = color - lighting;  
        // If the object that was intersected by is a transparent object
        if (sceneObjects[shadowRay2.index]->isTransparent() || sceneObjects[shadowRay2.index]->isRefractive()) {
            transparentOrRefractive = true;
            if (sceneObjects[shadowRay2.index]->isTransparent()) {
                color += sceneObjects[shadowRay2.index]->getColor() * 0.05f;
            }
        }
    }

    if (transparentOrRefractive) {
        color = color + transparentRefractiveAmbientTerm * obj->getColor();
        
    }
    else {
        color = color + ambientTerm * obj->getColor();
    }
    
    if (obj->isReflective() && step < MAX_STEPS) {
            float rho = obj->getReflectionCoeff();
            glm::vec3 normalVec = obj->normal(ray.hit);
            if (normalVec.z > 0) {
                glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
                Ray reflectedRay(ray.hit, reflectedDir);
                glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
                color = color + (rho * reflectedColor);
            }
          
    }

    if (obj->isTransparent()) {
        Ray newRay(ray.hit, ray.dir);
        glm::vec3 transparentColor = trace(newRay, step + 1);
        color = color + transparentColor;
    }

    if (obj->isRefractive()) {
        float eta = 1 / obj->getRefractiveIndex();
        float refractionCoeff = obj->getRefractionCoeff();

        glm::vec3 normalVec = obj->normal(ray.hit);
        glm::vec3 refractedDir = glm::refract(ray.dir, normalVec, eta);
        Ray refractedRayIn(ray.hit, refractedDir);
        refractedRayIn.closestPt(sceneObjects);

        glm::vec3 m = obj->normal(refractedRayIn.hit);
        glm::vec3 h = glm::refract(refractedDir, -m, 1.0f / eta);

        Ray refractedRayOut(refractedRayIn.hit, h);
        glm::vec3 refractiveColor = trace(refractedRayOut, step + 1);
        color = color + (refractionCoeff * refractiveColor);
    }

    //Fog calculation

    float z1 = -40;
    float z2 = -200;

    if (ray.hit.z < z1) {
        float t = (ray.hit.z - z1) / (z2 - z1);

        color = (1 - t) * color + t * glm::vec3(1, 1, 1);
    }

	return color;
}

glm::vec3 noAA(float xp, float yp, float cellX, float cellY, glm::vec3 eye) {
    glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray
    Ray ray = Ray(eye, dir);
    glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value
    return col;
}

glm::vec3 superSampling(float xp, float yp, float cellX, float cellY, glm::vec3 eye) {
    glm::vec3 dir1(xp + 0.25 * cellX, yp + 0.25 * cellY, -EDIST);
    glm::vec3 dir2(xp + 0.75 * cellX, yp + 0.25 * cellY, -EDIST);
    glm::vec3 dir3(xp + 0.25 * cellX, yp + 0.75 * cellY, -EDIST);
    glm::vec3 dir4(xp + 0.75 * cellX, yp + 0.75 * cellY, -EDIST);

    Ray ray1 = Ray(eye, dir1);
    Ray ray2 = Ray(eye, dir2);
    Ray ray3 = Ray(eye, dir3);
    Ray ray4 = Ray(eye, dir4);

    glm::vec3 col = (trace(ray1, 1) + trace(ray2, 1) + trace(ray3, 1) + trace(ray4, 1)) / glm::vec3(4);
    return col;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for(int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;
          
            glm::vec3 col = superSampling(xp, yp, cellX, cellY, eye);

			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}

void drawCube2(glm::vec3 centre, glm::vec3 dimensions, glm::vec3 color) {

}


//---Draw Cube---
// Draws a cube into the scene
// dimensions(length, height, width)
//---------------
void drawCube(glm::vec3 centre, glm::vec3 dimensions, glm::vec3 color) 
{
    glm::vec3 vertFrontTopLeft = glm::vec3(centre.x - dimensions.x / 2, centre.y + dimensions.y / 2, centre.z + dimensions.z / 2);
    glm::vec3 vertFrontTopRight = glm::vec3(centre.x + dimensions.x / 2, centre.y + dimensions.y / 2, centre.z + dimensions.z / 2);
    glm::vec3 vertFrontBotLeft = glm::vec3(centre.x - dimensions.x / 2, centre.y - dimensions.y / 2, centre.z + dimensions.z / 2);
    glm::vec3 vertFrontBotRight = glm::vec3(centre.x + dimensions.x / 2, centre.y - dimensions.y / 2, centre.z + dimensions.z / 2);
    glm::vec3 vertBackTopLeft = glm::vec3(centre.x - dimensions.x / 2, centre.y + dimensions.y / 2, centre.z - dimensions.z / 2);
    glm::vec3 vertBackTopRight = glm::vec3(centre.x + dimensions.x / 2, centre.y + dimensions.y / 2, centre.z - dimensions.z / 2);
    glm::vec3 vertBackBotLeft = glm::vec3(centre.x - dimensions.x / 2, centre.y - dimensions.y / 2, centre.z - dimensions.z / 2);
    glm::vec3 vertBackBotRight = glm::vec3(centre.x + dimensions.x / 2, centre.y - dimensions.y / 2, centre.z - dimensions.z / 2);

    Plane* PlaneFront = new Plane(vertFrontBotLeft, vertFrontBotRight, vertFrontTopRight, vertFrontTopLeft);
    Plane* PlaneTop = new Plane(vertFrontTopLeft, vertFrontTopRight, vertBackTopRight, vertBackTopLeft);
    Plane* PlaneLeft = new Plane(vertBackBotLeft, vertFrontBotLeft, vertFrontTopLeft, vertBackTopLeft);
    Plane* PlaneRight = new Plane(vertFrontBotRight, vertBackBotRight, vertBackTopRight, vertFrontTopRight);
    Plane* PlaneBack = new Plane(vertBackBotLeft, vertBackTopLeft, vertBackTopRight, vertBackBotRight);
    Plane* PlaneBot = new Plane(vertBackBotLeft, vertBackBotRight, vertFrontBotRight, vertFrontBotLeft);

  

    PlaneFront->setColor(color);
    PlaneTop->setColor(color);
    PlaneLeft->setColor(color);
    PlaneRight->setColor(color);
    PlaneBack->setColor(color);
    PlaneBot->setColor(color);

    sceneObjects.push_back(PlaneFront);
    sceneObjects.push_back(PlaneTop);
    sceneObjects.push_back(PlaneLeft);
    sceneObjects.push_back(PlaneRight);
    sceneObjects.push_back(PlaneBack);
    sceneObjects.push_back(PlaneBot);
}

//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

    Plane* plane = new Plane(glm::vec3(-50., -15, -40),      //Point A
        glm::vec3(50., -15, -40),       //Point B
        glm::vec3(50., -15, -200),      //Point C
        glm::vec3(-50., -15, -200));    //Point D
    plane->setSpecularity(false);
    sceneObjects.push_back(plane);

    // Big ball at the back
	Sphere *sphere1 = new Sphere(glm::vec3(-9.0, 0.0, -120.0), 15.0);
    sphere1->setReflectivity(true, 0.9);
	sphere1->setColor(glm::vec3(0, 0, 1));   
	sceneObjects.push_back(sphere1);		 //Add sphere to scene objects
    
    // Earth
    Sphere *sphere2 = new Sphere(glm::vec3(-10.0, -10.0, -68.0), 3.0);
    sceneObjects.push_back(sphere2);        //Add sphere to scene objects

    // Refractive sphere
    Sphere *sphere3 = new Sphere(glm::vec3(0, -10.0, -68.0), 3.0);
    sphere3->setColor(glm::vec3(0, 0, 0)); 
    sphere3->setRefractivity(true, 1, 1.01);
    sphere3->setReflectivity(true, 0.1);
    sceneObjects.push_back(sphere3);
    
    // Hollow sphere
    Sphere *sphere4 = new Sphere(glm::vec3(10.0, -10.0, -68.0), 3.0);
    sphere4->setColor(glm::vec3(0, 0, 1));  
    sphere4->setTransparency(true, 0.5);
    sphere4->setReflectivity(true, 0.1);
    sceneObjects.push_back(sphere4);
    


    Cylinder* cylinder1 = new Cylinder(glm::vec3(-10, -15, -68), 3, 2);
    cylinder1->setColor(glm::vec3(0.5, 0.5, 0));
    cylinder1->setCap(true);
    sceneObjects.push_back(cylinder1);

    Cylinder* cylinder2 = new Cylinder(glm::vec3(0, -15, -68), 3, 2);
    cylinder2->setColor(glm::vec3(0.5, 0.5, 0));
    cylinder2->setCap(true);
    sceneObjects.push_back(cylinder2);

    Cylinder* cylinder3 = new Cylinder(glm::vec3(10, -15, -68), 3, 2);
    cylinder3->setColor(glm::vec3(0.5, 0.5, 0));
    cylinder3->setCap(true);
    sceneObjects.push_back(cylinder3);

    Cylinder* cylinder4 = new Cylinder(glm::vec3(19, -15, -124), 4, 20);
    cylinder4->setColor(glm::vec3(0.5, 0, 0.5));
    cylinder4->setCap(true);
    sceneObjects.push_back(cylinder4);

    Cone* cone1 = new Cone(glm::vec3(7, -15, -95), 5, 15);
    cone1->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(cone1);

    drawCube(glm::vec3(16, -12.5, -85.0), glm::vec3(5.0), glm::vec3(0, 1, 0));

    
    
    texture = TextureBMP("earthmap1k.bmp");
}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
