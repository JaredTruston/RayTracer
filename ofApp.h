// This file provides the class definitions Ray, SceneObject, Sphere, Mesh,
// View, ViewPlane, RenderCam, and ofApp
// - author: Jared Bechthold
// - starter files containing initial class defintions of Ray, SceneObject, Sphere,
// Mesh, View, ViewPlane, RenderCam, and ofApp provided by Professor Kevin Smith

#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include <glm/gtx/intersect.hpp>

//  General Purpose Ray class 
//
class Ray {
public:
	// ray constrcutor
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }

	// default ray constructor
	Ray() { this->p = glm::vec3(0, 0, 0); this->d = glm::vec3(0, 0, 0); }

	// draws the ray
	void draw(float t) { ofDrawLine(p, p + t * d); }

	// returns value along the ray for any value of t
	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	// two quantities represent the array
	//	position where the ray starts in space
	//	direction where the ray is fired
	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//	(AKA SurfaceObject)
class SceneObject {
public:
	// every SceneObject has draw() and intersect() methods to be overloaded
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { cout << "SceneObject::intersect" << endl; return false; }
	// returns the color of the scene object
	virtual ofColor getColor(glm::vec3 intersectPt) { return diffuseColor; }

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	// Sphere constructor that sets the position, raidus, and color of the Sphere
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	// Default Sphere constructor
	Sphere() {}

	// tests for intersection of the Sphere with a Ray
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	// draws the Sphere
	void draw() {
		ofFill();
		ofSetColor(diffuseColor);
		ofDrawSphere(position, radius);
	}

	// radius of the Sphere
	float radius = 1.0;
};

//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }
	void draw() { }
};


//  General purpose plane 
//
class Plane : public SceneObject {
public:
	// Plane constructor that sets point, normal, color, height, and width
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		if (normal == glm::vec3(0, 1, 0)) plane.rotateDeg(90, 1, 0, 0);
	}
	// default Plane constructor
	Plane() {
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);
	}

	// applies texture image to the plane
	void applyTexture(ofImage textureToApply) {
		textureImg = textureToApply;
		textureApplied = true;
	}

	// sets amount of tiles in x and y direction for texture mapping
	void setTiles(int x, int y) {
		tilesX = x;
		tilesY = y;
	}

	// overrdes getColor to handle textureMapping
	ofColor getColor(glm::vec3 intersectPt) {
		// check if texture is applied and plane is orthogonal to positive y axis
		if (textureApplied && normal == glm::vec3(0, 1, 0)) {
			// get minimum point on plane
			glm::vec3 minimum = glm::vec3(position.x - width / 2, position.y, position.z - height / 2);
			// get x and y coordinates of current point on plane in plane's own 2d coordinate system with max height/width
			//  equal to the number of tiles in y and x direction respectively
			float nX = ((intersectPt.x - minimum.x) / width) * tilesX;
			float nY = ((intersectPt.z - minimum.z) / height) * tilesY;
			// get pixel coordinates of textureImg for current nX and nY point
			float i = nX * textureImg.getWidth() - 0.5;
			float j = nY * textureImg.getHeight() - 0.5;
			// get color of current point in plane (apply modulus for repeating pattern)
			return textureImg.getColor(fmod(i, textureImg.getWidth()), fmod(j, textureImg.getHeight()));
		}
		else {	// return assigned diffuse color if no texture is applied
			return diffuseColor;
		}
	}

	// tests for intersection of Plane with a Ray
	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	float sdf(const glm::vec3 &p);
	// returns the Plane's normal
	glm::vec3 getNormal(const glm::vec3 &p) { return this->normal; }
	// draws the Plane
	void draw() {
		ofSetColor(diffuseColor);
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawFaces();
	}

	// used for drawing the Plane
	ofPlanePrimitive plane;
	// holds vector normal to the Plane
	glm::vec3 normal;
	// dimensions of the Plane
	float width = 20;
	float height = 20;
	// detects if a texture has been applied to the plane (initialized false)
	bool textureApplied = false;
	// holds texture image (if applied)
	ofImage textureImg;
	// holds amount of tiles used in texture mapping in x and y direction
	//  default to 10 each
	int tilesX = 10;
	int tilesY = 10;
};

// view plane for render camera
// 
class  ViewPlane : public Plane {
public:
	// constructor for ViewPlane defining p0 as bottom left corner
	// and p1 as top right corner
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }
	// default constructor for ViewPlane create reasonable defaults (6x4 aspect)
	ViewPlane() {
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // ViewPlane currently limited to Z axis orientation
	}

	// sets the min and max points of the ViewPlane
	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	// returns the width and height of the ViewPlane
	float getAspect() { return width() / height(); }
	// converts coordinates on ViewPlane to 3D world space
	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]
	// draws the ViewPlane
	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}
	// returns the width and height of the ViewPlane
	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}
	// returns the corners of the ViewPlane
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }


	glm::vec2 min, max;	// defines the boundaries of the ViewPlane
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
class RenderCam : public SceneObject {
public:
	// default Constructor for the RenderCam
	RenderCam() {
		position = glm::vec3(0, 0, 10);
		aim = glm::vec3(0, 0, -1);
		boxDimension = 1.0;
	}

	// returns a Ray from the current RenderCam position to the (u, v) position of the ViewPlane
	Ray getRay(float u, float v);
	// draws the RenderCam
	void draw() { ofDrawBox(position, boxDimension); };
	// draws lines connecting camera to the view plane
	void drawFrustum() {
		// line from top left of RenderCam to top left of View Plane
		ofDrawLine(glm::vec3(position.x - boxDimension / 2, position.y + boxDimension / 2, position.z - boxDimension / 2), glm::vec3(view.topLeft(), view.position.z));
		// line from bottom left of RenderCam to bottom left of View Plane
		ofDrawLine(glm::vec3(position.x - boxDimension / 2, position.y - boxDimension / 2, position.z - boxDimension / 2), glm::vec3(view.bottomLeft(), view.position.z));
		// line from top right of RenderCam to bottom right of View Plane
		ofDrawLine(glm::vec3(position.x + boxDimension / 2, position.y + boxDimension / 2, position.z - boxDimension / 2), glm::vec3(view.topRight(), view.position.z));
		// line from bottom right of RenderCam to bottom right of View Plane
		ofDrawLine(glm::vec3(position.x + boxDimension / 2, position.y - boxDimension / 2, position.z - boxDimension / 2), glm::vec3(view.bottomRight(), view.position.z));
	}

	float boxDimension;	// defines the width, length, and height of the RenderCam
	glm::vec3 aim;		// the position that the RenderCam aims at			
	ViewPlane view;		// The camera viewplane, this is the view that we will render 
};

// base light class
//
class Light : public SceneObject {
public:
	// sets intensity of light
	void setIntensity(float newIntensity) {
		intensity = newIntensity;
	}

	// tracks light intensity
	float intensity;
};

// point light class
//
class PointLight : public Light {
public:
	// PointLight constructor
	PointLight(glm::vec3 position, float intensity, float radius, ofColor diffuse = ofColor::white) {
		this->position = position;
		this->intensity = intensity;
		this->radius = radius;
		this->diffuseColor = diffuse;
	}

	// draws the Sphere representing the light
	void draw() {
		ofNoFill();
		ofSetColor(diffuseColor);
		ofDrawSphere(position, radius);
	}

	// tracks size of drawble light
	float radius;
};

// triangle class
//
class Triangle {
public:
	// adds vertex indices to vertInd integer array
	Triangle(int i1, int i2, int i3) {
		vertInd[0] = i1;
		vertInd[1] = i2;
		vertInd[2] = i3;
	}

	int vertInd[3];	// Holds three vertices of triangle
};

// area light class
//
class AreaLight : public Light {
public:
	// AreaLight constructor
	AreaLight(glm::vec3 position, float intensity) {
		this->position = position;
		this->intensity = intensity;
	}

	// Default AreaLight constructor
	AreaLight() {}

	// Methods
	// Iterates through and draws all triangles of the AreaLight
	void draw()
	{
		// Set color and no fill for Triangle
		ofSetColor(ofColor::white);
		ofNoFill();

		// Integer variables to hold vertices of Triangle
		int v1, v2, v3;

		// Iterates through and draw each Triangle in area light
		for (Triangle t : triangles) {
			// Record indices of vertices of triangle
			v1 = t.vertInd[0];
			v2 = t.vertInd[1];
			v3 = t.vertInd[2];
			// Draw triangle using record vertices
			ofDrawTriangle(verts[v1], verts[v2], verts[v3]);
		}
	}

	void updatePosition();	// updates the position 

	// Fields
	vector<glm::vec3> verts;	// holds all vertex values of area light
	vector<Triangle> triangles;	// holds all triangles of the area light
};

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	// Loads obj file
	void loadFile(string fileName);
	// Adds phong shading to given pixel in scene
	ofColor phong(Ray ray, const glm::vec3 & point, const glm::vec3 & normal, const ofColor diffuse, const ofColor specular, float power);
	// adds lambert shading to given pixel in scene
	ofColor lambert(Ray ray, const glm::vec3 &point, const glm::vec3 &normal, const ofColor diffuse);
	// adds phong shading to given pixel using an area light instance
	ofColor phongAreaLight(Ray ray, const glm::vec3 & point, const glm::vec3 & normal, const ofColor diffuse, const ofColor specular, float power);
	// adds Light instances to lights vector
	void addLight(PointLight* newLight) { lights.push_back(newLight); }
	// checks ray fired from object to light for intersction with other SceneObjects
	bool shadowCheck(Ray ray, glm::vec3 intersection, glm::vec3 normal, glm::vec3 lightPosition);
	// draws RenderCam view to ofImage instance
	void rayTrace();

	// toggles drawing of RenderCam, ViewPlane, and Frustom on and off
	bool bHide = true;
	// toggles preview of rendered image on and off
	// only shows image if render has been called
	bool bShowImage = false;
	// camera that can move about the scene starting from RenderCam POV
	ofEasyCam  mainCam;
	// shows view of the scene from the side
	ofCamera sideCam;
	// provides preview of what RenderCam sees
	ofCamera previewCam;
	// set to current camera either mainCam, sideCam, or previewCam
	ofCamera  *theCam;
	// set up one render camera to render image
	RenderCam renderCam;
	// image to write to in order to save on to disk
	ofImage image;
	// second image to preview
	ofImage prevImage;
	// holds image to map to plane
	ofImage planeTexture;
	// to add scene objects to the scene
	vector<SceneObject *> scene;
	// floor of scene
	Plane* floor;
	// to add light objects to the scene
	vector<Light *> lights;
	// dimensions of the image to be rendered
	int imageWidth = 1200;
	int imageHeight = 800;
	// dimensions of the textureImage
	int textureWidth = 1000;
	int textureHeight = 1000;
	// variables to be passed into the intersect method calls in raytrace method
	glm::vec3 intersectPt;
	glm::vec3 intersectNormal;
	// power of phong shading
	float phongPower;
	// GUI slider
	ofxFloatSlider power;
	ofxFloatSlider intensity;
	ofxFloatSlider areaLightIntensity;
	ofxPanel gui;
	// Holds AreaLight instance;
	AreaLight areaLight;
};
