// This file provides the implementation of the Ray Tracer App
// - author: Jared Bechthold 
// - starter files containing Plane::intersect, ViewPlane::toWorld, 
// and RenderCam::getRay methods provided by Professor Kevin Smith

#include "ofApp.h"

// Intersect Ray with Plane  (wrapper on glm::intersect*)
// returns a boolean variable denoting if intersection occurred inside Plane
bool Plane::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect) {
	// measures distance along the array
	float dist;
	// detects if intersection was inside plane
	bool insidePlane = false;
	// detects if intersection occurred
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		// determines if intersection point was within range of the Plane's dimensions
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width / 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z + height / 2);
		if (point.x < xrange[1] && point.x > xrange[0] && point.z < zrange[1] && point.z > zrange[0]) {
			insidePlane = true;
		}
	}
	return (insidePlane);
}


// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}

// Updates position of area light vertices to move by position
// of area light
void AreaLight::updatePosition() {
	for (int i = 0; i < verts.size(); i++) {
		verts[i] = glm::vec3(verts[i].x + position.x, verts[i].y + position.y, verts[i].z + position.z);
	}
}

//--------------------------------------------------------------
// Provides initial setup for the cameras, scene, and image instances.
void ofApp::setup() {
	// camera setup
	ofSetBackgroundColor(ofColor::black);
	theCam = &mainCam;
	// sets the mainCam's initial distance from origin in Z direction
	mainCam.setDistance(10);
	mainCam.setNearClip(.1);
	// sets previewCam to renderCam's position and aim
	previewCam.setPosition(renderCam.position);
	previewCam.lookAt(renderCam.aim);
	previewCam.setNearClip(.1);
	// sets sideCam to view Origin from X direction
	sideCam.setPosition(glm::vec3(100, 0, 0));
	sideCam.lookAt(glm::vec3(0, 0, 0));
	sideCam.setNearClip(.1);

	// set plane to be used as floor
	floor = new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0), ofColor::grey);

	// add SceneObject instances to scene vector
	scene.push_back(floor);
	scene.push_back(new Sphere(glm::vec3(3, 1, -5), 2.0, ofColor::green));
	scene.push_back(new Sphere(glm::vec3(-3, -1, 2), 1.0, ofColor::red));
	scene.push_back(new Sphere(glm::vec3(0, 1, 0), 2.0, ofColor::blue));

	// add Light instances to lights vector
	// lights test scene 1
	addLight(new PointLight(glm::vec3(-4, 1, 4), 100, 0.1));
	addLight(new PointLight(glm::vec3(-5, 5, 2), 100, 0.1));
	addLight(new PointLight(glm::vec3(3, 5, -2), 100, 0.1));

	// lights test scene 2
	//addLight(new PointLight(glm::vec3(0, 8, 0), 100, 0.1));
	//addLight(new PointLight(glm::vec3(-6, 7, 4), 100, 0.1));
	//addLight(new PointLight(glm::vec3(8, 8, 4), 100, 0.1));

	// lights test scene 3
	//addLight(new PointLight(glm::vec3 (5, 2, 0), 100, 0.1));
	//addLight(new PointLight(glm::vec3 (0, 2, 7), 100, 0.1));

	// Instantiates AreaLight instance
	areaLight = AreaLight(glm::vec3(0, 9, 2), 100);

	// initializes the image ofImage instance to be drawn by rayTrace method
	image.allocate(imageWidth, imageHeight, ofImageType::OF_IMAGE_COLOR);
	image.save("newImage.png");

	// initializes the planeTexture ofImage instance
	planeTexture.allocate(textureWidth, textureHeight, ofImageType::OF_IMAGE_COLOR);
	planeTexture.load("textureImg.jpg");
	//planeTexture.load("textureImg2.jpg");
	//planeTexture.load("textureImg3.jpg");
	floor->applyTexture(planeTexture);			// applies planeTexture to floor
	// set tile sizes of texture
	//floor->setTiles(20, 25);
	//floor->setTiles(15, 15);
	//floor->setTiles(30, 35);

	// sets up the gui slider
	gui.setup();
	gui.add(power.setup("Phong Power", 20, 0, 100));
	gui.add(intensity.setup("P-Lights Intensity", 10, 0, 100));
	gui.add(areaLightIntensity.setup("Area Light Intensity", 10, 0, 100));
}

//--------------------------------------------------------------
// Update each light's intensity and the power of phong shading
//  to values shown in gui
void ofApp::update() {

	// Sets each light's intensity value to current value in the gui
	for (int i = 0; i < lights.size(); i++) {
		lights[i]->setIntensity(intensity);
	}
	// Sets area light intensity
	areaLight.setIntensity(areaLightIntensity);
	// Sets phong shading power to current value on gui
	phongPower = power;
}

//--------------------------------------------------------------
// Draws the 3D image of the spheres and plane within the Camera
// perspective. 
// User can also toggle to see drawing of prevImage 
// of rendering.
void ofApp::draw() {
	// draws the SceneObjects in the 3D view if bShowImage = false
	// can toggle drawing of RenderCam and its fields
	if (!bShowImage) {
		// show gui
		ofDisableDepthTest();
		gui.draw();
		ofEnableDepthTest();
		// 3D transformation for the camera
		theCam->begin();

		// draw all SceneObject instances in scene vector
		for (int i = 0; i < scene.size(); i++) {
			scene[i]->draw();
		}

		// draw all Lights in light vector
		for (int i = 0; i < lights.size(); i++) {
			lights[i]->draw();
		}

		// draws area light mesh in scene
		areaLight.draw();

		// draws RenderCam and RenderCam fields if false
		if (!bHide) {
			// draws the RenderCam
			ofSetColor(ofColor::white);
			ofNoFill();
			renderCam.draw();
			// draws the ViewPlane
			renderCam.view.draw();
			// draws the Frustum
			renderCam.drawFrustum();
		}

		// end 3D transformation for the camera
		theCam->end();
	}
	else { // bShowImage = true and shows preview of the rendered ofImage prevImage
		ofSetColor(ofColor::white);
		// loads the prevImage to be displayed
		prevImage.load("newImage.png");
		// draws prevImage
		prevImage.draw(ofGetWidth() / 2 - imageWidth / 2, ofGetHeight() / 2 - imageHeight / 2);
	}
}

//--------------------------------------------------------------
// Provides implementation for Keys to switch between camera
// perspectives, display different outputs in drawing method,
// and call the rayTrace method.
void ofApp::keyPressed(int key) {
	switch (key) {
	case 'r':
	case 'R':		// calls the rayTrace method
		cout << "rendering..." << endl;
		rayTrace();
		cout << "done" << endl;
		break;
	case OF_KEY_F1:	// switches POV to mainCam
		theCam = &mainCam;
		break;
	case OF_KEY_F2:	// switches POV to sideCam
		theCam = &sideCam;
		break;
	case OF_KEY_F3:	// switches POV to previewCam
		theCam = &previewCam;
		break;
	case 'p':
	case 'P':		// toggles drawing of prevImage
		bShowImage = !bShowImage;
		break;
	case 'v':
	case 'V':		// toggles drawing of the RenderCam, ViewPlane, and Frustom
		bHide = !bHide;
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
// uses file io to update area light with verts and triangles
// of passed in obj file
void ofApp::loadFile(string fileName) {
	// Clear verts and triangles vectors
	areaLight.verts.clear();
	areaLight.triangles.clear();

	ifstream inputStream;	// Input stream
	string read;			// Reads from input stream
	float ver1, ver2, ver3;	// Temporarily stores vertices of triangles
	int i1, i2, i3;			// Temporarily stores indices of triangle vertices
	string tempString;		// Temporarily stores string read from input stream

	// Open file
	inputStream.open(fileName);

	if (!inputStream) // Check if file opening failed
	{
		cout << "File open failed";
		exit();	// Special system call to abort program
	}
	else {
		// Read from input stream
		while (inputStream >> read) {
			if (read == "v") {		// Check for a v to denote vertex
				// Read vertices from input stream
				inputStream >> ver1 >> ver2 >> ver3;

				// Adds vertices to area light's verticies vector
				areaLight.verts.push_back(glm::vec3(ver1, ver2, ver3));
			}
			else if (read == "f") { // Check for an f to denote face
				// Reads indices of triangle vertices from input stream
				inputStream >> tempString;
				i1 = stoi(tempString.substr(0, tempString.find("/"))) - 1;
				inputStream >> tempString;
				i2 = stoi(tempString.substr(0, tempString.find("/"))) - 1;
				inputStream >> tempString;
				i3 = stoi(tempString.substr(0, tempString.find("/"))) - 1;

				// Adds indices of triangle's vertices to area light's triangle vector
				areaLight.triangles.push_back(Triangle(i1, i2, i3));
			}
		}
	}
	// Close file
	inputStream.close();

	// Updates location of triangle verticies relative to area light position
	areaLight.updatePosition();

	// Print Area Light diagnostic information
	cout << "Number of Vertices: " << areaLight.verts.size() << endl;
	cout << "Total Number of Faces: " << areaLight.triangles.size() << endl;
}

//--------------------------------------------------------------
// Reads in files dragged into window
void ofApp::dragEvent(ofDragInfo dragInfo) {
	// uses dragged in obj file to set fields of area light
	loadFile(dragInfo.files[0]);
}

//--------------------------------------------------------------
// Iterates through the dimensions of the image ofImage instance 
// given by imageHeight and imageWidth and draws a color at each
// pixel given by the closest SceneObject viewed by the RenderCam
// at that position
void ofApp::rayTrace()
{
	Ray ray;						// holds the current ray set by the current pixel in the iteration
	bool hit;						// determines if the ray hit a SceneObject
	float shortestDistance;			// holds the distance from the ray start position to the closest SceneObject
	float currentDistance;			// holds the distance between the ray start position at current pixel in iteration and the current SceneObject
	SceneObject *closestObject;		// refers to the object that is closest to the RenderCam where a hit occurred
	ofColor color;					// holds color of closest object after phong shading has been applied
	ofColor objColor;				// holds color of closest object before any shading has been applied

	// for each pixel in the image
	for (int i = 0; i < imageWidth; i++) {
		for (int j = 0; j < imageHeight; j++) {
			// get current pixel in u and v coordinates
			float u = (i + 0.5) / imageWidth;
			float v = (j + 0.5) / imageHeight;
			// get the current ray from renderCam to point(u, v)
			ray = renderCam.getRay(u, v);
			// hit is initially set to false
			hit = false;
			// distance is initially infinity since no object has been tested for intersection yet
			shortestDistance = std::numeric_limits<float>::infinity();
			// set closest object equal to NULL to begin since no object has been tested for intersection yet
			closestObject = NULL;
			// for each object in the scene
			for (int k = 0; k < scene.size(); k++) {
				// tests the current SceneObject for intersection with Ray from current pixel in iteration
				if (scene[k]->intersect(ray, intersectPt, intersectNormal)) {
					// gets the distance of the Ray at current pixel to Intersection Point with current SceneObject
					currentDistance = sqrt(pow(ray.p.x - intersectPt.x, 2) + pow(ray.p.y - intersectPt.y, 2)
						+ pow(ray.p.z - intersectPt.z, 2));
					// checks if the currentDistance is shortest
					if (currentDistance < shortestDistance) {
						// sets currentDistance to be shortest and sets closestObject to current SceneObject
						shortestDistance = currentDistance;
						closestObject = scene[k];
					}
					// sets hit to true since Ray intersected with a SceneObject
					hit = true;
				}
				if (hit) {	// if a hit occurred color current pixel with currentObject's color and shade it according to light placement
					// reset intersectPt and intersectNormal to closestObject
					closestObject->intersect(ray, intersectPt, intersectNormal);

					// assign color of closest object to objColor (use texture for plane if applied)
					objColor = closestObject->getColor(intersectPt);

					// Shades the current pixel with ambient and lambert shading
					//color = lambert(ray, intersectPt, intersectNormal, closestObject->diffuseColor);
					// Shades the current pixel with ambient, lambert and phong shading
					color = phong(ray, intersectPt, intersectNormal, objColor, ofColor::white, phongPower);
					// Shades the current pixel with ambient, lambert and phong shading using areaLight instance
					color += phongAreaLight(ray, intersectPt, intersectNormal, objColor, ofColor::white, phongPower);

					// colors the current pixel in iteration
					image.setColor(i, imageHeight - 1 - j, color);
				}
				else {		// if hit did not occur color current pixel with background color
					image.setColor(i, imageHeight - 1 - j, ofGetBackgroundColor());
				}
			}
		}
	}
	// save changes to the image
	image.save("newImage.png");
}

//--------------------------------------------------------------
// Adds lambert shading to given pixel in the scene
ofColor ofApp::lambert(Ray ray, const glm::vec3 & point, const glm::vec3 & normal, const ofColor diffuse)
{
	// Sets ambient shading
	ofColor result = 0.25 * diffuse;			// ambient shading value to not make image completely dark
	// Variables used in checking for shadows
	glm::vec3 blockIntersectPt;					// point where ray to light from given point intersects another surface
	glm::vec3 blockIntersectNormal;				// normal where ray to light from point intersects another surface
	glm::vec3 shadowRayPt;						// point where light intersects (+ small value towards normal)
	Ray shadingRay;								// ray from	shadowRayPt to light origin
	bool blocked;								// dictates whether point is blocked from current light
	// Variables used in calculating the light
	glm::vec3 directionToCam;					// vector from point to camera
	glm::vec3 directionToLight;					// vector from point to light
	glm::vec3 norm = glm::normalize(normal);	// normal at point
	float illumination;							// light intensity/(distance to light)^2
	float dotProdNormLight;						// dot product of norm vector and directionToLight vector


	// iterates through all lights
	for (int i = 0; i < lights.size(); i++) {
		// Sets direction of ray pointing to camera from intersection point on SceneObject
		directionToCam = -glm::normalize(ray.d);
		// Sets direction of ray pointing to light from intersection point on SceneObject
		directionToLight = glm::normalize(lights[i]->position - point);

		// Determines point near surface where shadingRay begins
		shadowRayPt = point + 0.0001*norm;
		// Initializes ray fired from shadowRayPt
		shadingRay = Ray(shadowRayPt, directionToLight);
		// Checks for shadows and sets blocked to true if point is blocked from light
		blocked = shadowCheck(shadingRay, blockIntersectPt, blockIntersectNormal, lights[i]->position);

		// Only adds lambert shading to result if point is not blocked from current light
		if (blocked == false) {
			// Gets the illumination from source
			illumination = lights[i]->intensity / pow(glm::distance(lights[i]->position, point), 2);
			// Gets dot product of normal and directionToLight vectors
			dotProdNormLight = glm::dot(norm, directionToLight);
			// Adds lambert shaded color to result
			result = result + diffuse * illumination * glm::max(0.0f, dotProdNormLight);
		}
	}
	return result;
}

//--------------------------------------------------------------
// Adds phong shading to given pixel in the scene
ofColor ofApp::phong(Ray ray, const glm::vec3 & point, const glm::vec3 & normal, const ofColor diffuse, const ofColor specular, float power)
{
	// Sets ambient shading
	ofColor result = 0.15 * (diffuse);			// ambient shading value to not make image completely dark
	// Variables used in checking for shadows
	glm::vec3 blockIntersectPt;					// point where ray to light from given point intersects another surface
	glm::vec3 blockIntersectNormal;				// normal where ray to light from point intersects another surface
	glm::vec3 shadowRayPt;						// point where light intersects (+ small value towards normal)
	Ray shadingRay;								// ray from	shadowRayPt to light origin
	bool blocked;								// dictates whether point is blocked from current light
	// Variables used in calculating the diffuse and phong shading
	glm::vec3 directionToCam;					// vector from point to camera
	glm::vec3 directionToLight;					// vector from point to light
	glm::vec3 norm = glm::normalize(normal);	// normal at point
	float illumination;							// light intensity/(distance to light)^2
	float dotProdNormLight;						// dot product of norm vector and directionToLight vector
	glm::vec3 bisectingVec;						// bisecting vector between directionToLight and directionToCam vectors
	float dotProdNormBis;						// dot product of norm vector and bisectingVec vector


	// iterates through all lights
	for (int i = 0; i < lights.size(); i++) {
		// Sets direction of ray pointing to camera from intersection point on SceneObject
		directionToCam = glm::normalize(renderCam.position - point);
		// Sets direction of ray pointing to light from intersection point on SceneObject
		directionToLight = glm::normalize(lights[i]->position - point);

		// Determines point near surface where shadingRay begins
		shadowRayPt = point + 0.0001*norm;
		// Initializes ray fired from shadowPt
		shadingRay = Ray(shadowRayPt, directionToLight);
		// Checks for shadows and sets blocked to true if point is blocked from light
		blocked = shadowCheck(shadingRay, blockIntersectPt, blockIntersectNormal, lights[i]->position);

		// Only adds lambert and phong shading to result if point is not blocked from current light
		if (blocked == false) {
			// Gets the illumination from source
			illumination = lights[i]->intensity / pow(glm::distance(lights[i]->position, point), 2);
			// Gets dot product of normal and directionToLight vectors
			dotProdNormLight = glm::dot(norm, directionToLight);
			// Calculate and add diffuse shading to result
			result += diffuse * illumination * glm::max(0.0f, dotProdNormLight);
			// Obtains the bisecting vector between vector to cam and vector to light
			bisectingVec = glm::normalize(directionToCam + directionToLight);
			// Dot product of bisecting vector and normal
			dotProdNormBis = glm::dot(norm, bisectingVec);
			// Adds phong shaded color to result
			result += specular * illumination * pow(glm::max(0.0f, dotProdNormBis), power);
		}
	}
	return result;
}

//--------------------------------------------------------------
// Adds phong shading to given pixel in the scene using verticies
// of area light instance
ofColor ofApp::phongAreaLight(Ray ray, const glm::vec3 & point, const glm::vec3 & normal, const ofColor diffuse, const ofColor specular, float power)
{
	// Sets initial shading to 0
	ofColor result = 0;							// initializes result to 0 since ambient shading is aleady provided in phong method
	// Variables used in checking for shadows
	glm::vec3 blockIntersectPt;					// point, where ray to area light's current vertex from given point intersects another surface
	glm::vec3 blockIntersectNormal;				// normal, where ray to area light's current vertex from given point intersects another surface
	glm::vec3 shadowRayPt;						// point where area light's current vertex intersects (+ small value towards normal)
	Ray shadingRay;								// ray from	shadowRayPt to area light's current vertex
	bool blocked;								// dictates whether point is blocked from current area light vertex
	// Variables used in calculating the diffuse and phong shading
	glm::vec3 directionToCam;					// vector from point to camera
	glm::vec3 directionToLight;					// vector from point to area light's current vertex
	glm::vec3 norm = glm::normalize(normal);	// normal at point
	float illumination;							// light intensity/(distance to area light's current vertex)^2
	float dotProdNormLight;						// dot product of norm vector and directionToLight vector
	glm::vec3 bisectingVec;						// bisecting vector between directionToLight and directionToCam vectors
	float dotProdNormBis;						// dot product of norm vector and bisectingVec vector


	// iterates through verticies of area light
	for (int i = 0; i < areaLight.verts.size(); i++) {
		// Sets direction of ray pointing to camera from intersection point on SceneObject
		directionToCam = glm::normalize(renderCam.position - point);
		// Sets direction of ray pointing to given vertex from intersection point on SceneObject
		directionToLight = glm::normalize(areaLight.verts[i] - point);

		// Determines point near surface where shadingRay begins
		shadowRayPt = point + 0.0001*norm;
		// Initializes ray fired from shadowPt
		shadingRay = Ray(shadowRayPt, directionToLight);
		// Checks for shadows and sets blocked to true if point is blocked from given area light vertex
		blocked = shadowCheck(shadingRay, blockIntersectPt, blockIntersectNormal, areaLight.verts[i]);

		// Only adds lambert and phong shading to result if point is not blocked from current area light vertex
		if (blocked == false) {
			// Gets the illumination from source
			illumination = areaLight.intensity / pow(glm::distance(areaLight.verts[i], point), 2);
			// Gets dot product of normal and directionToLight vectors
			dotProdNormLight = glm::dot(norm, directionToLight);
			// Calculate and add diffuse shading to result
			result += diffuse * illumination * glm::max(0.0f, dotProdNormLight);
			// Obtains the bisecting vector between vector to cam and vector to light
			bisectingVec = glm::normalize(directionToCam + directionToLight);
			// Dot product of bisecting vector and normal
			dotProdNormBis = glm::dot(norm, bisectingVec);
			// Adds phong shaded color to result
			result += specular * illumination * pow(glm::max(0.0f, dotProdNormBis), power);
		}
	}
	return result;
}

//--------------------------------------------------------------
// Checks for intersection between lights and other objects in scene
bool ofApp::shadowCheck(Ray ray, glm::vec3 intersection, glm::vec3 normal, glm::vec3 lightPosition) {
	for (int k = 0; k < scene.size(); k++) {
		// only return true if an intersection occurs with a surface before ray reaches the light
		if (scene[k]->intersect(ray, intersection, normal) && (glm::distance(ray.p, intersection) < glm::distance(ray.p, lightPosition)))
			return true;
	}
	return false;
}
