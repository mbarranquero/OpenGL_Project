// ----------------------------------------------
// Base code for practical computer graphics
// assignments.
//
// Copyright (C) 2018 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/glad.h>

#include <cstdlib>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>
#include <exception>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Error.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "MeshLoader.h"
#include "Material.h"
#include "LightSource.h"

static const std::string SHADER_PATH ("Resources/Shaders/");

static const std::string DEFAULT_MESH_FILENAME ("Resources/Models/face.off");

using namespace std;

float f1, f2;
GLuint albedoTex;
GLuint roughnessTex;
GLuint metallicTex;
GLuint occlusionTex;
GLuint xtoonTex;

// Window parameters
static GLFWwindow * windowPtr = nullptr;
LightSource * lPtr = new LightSource();
LightSource * lPtrb = new LightSource();
LightSource * lPtrbi = new LightSource();

// Pointer to the current camera model
static std::shared_ptr<Camera> cameraPtr;

// Pointer to the displayed mesh
static std::shared_ptr<Mesh> meshPtr;

//Pointer to the Material
//static std::shared_ptr<Material> materialPtr;

// Pointer to GPU shader pipeline i.e., set of shaders structured in a GPU program
static std::shared_ptr<ShaderProgram> shaderProgramPtr; // A GPU program contains at least a vertex shader and a fragment shader

// Camera control variables
static float meshScale = 1.0; // To update based on the meshface size, so that navigation runs at scale
static bool isRotating (false);
static bool isPanning (false);
static bool isZooming (false);
static double baseX (0.0), baseY (0.0);
static glm::vec3 baseTrans (0.0);
static glm::vec3 baseRot (0.0);

//static float changingToon = 1.f;
static float laplacian = 0.f;
static float xtoon = 0.f;
static bool simpl = true;
static bool subd = true;
static bool adap = true;

static int resol = 15;

std::string mshFile;

void clear ();

void printHelp () {
	std::cout << "> Help:" << std::endl
			  << "    Mouse commands:" << std::endl
			  << "    * Left button: rotate camera" << std::endl
			  << "    * Middle button: zoom" << std::endl
			  << "    * Right button: pan camera" << std::endl
			  << "    Keyboard commands:" << std::endl
   			  << "    * H: print this help" << std::endl
					<< "    * T: X Toon Mode" << std::endl
					<< "    * S: Simplifaction mode" << std::endl
					<< "    * Q: OctreeNode Simplication mode" << std::endl
					<< "    * P: Subdivision (Loop) mode" << std::endl
					<< "    * L: Laplacian Mode 0.4" << std::endl
					<< "    * 1: Laplacian Mode 0.1" << std::endl
					<< "    * 2: Laplacian Mode 0.5" << std::endl
					<< "    * 3: Laplacian Mode 1" << std::endl
					<< "    * F2: reload the original model" << std::endl
   			  << "    * F1: toggle wireframe rendering" << std::endl
   			  << "    * ESC: quit the program" << std::endl;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback (GLFWwindow * windowPtr, int width, int height) {
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));
	glViewport (0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

/// Executed each time a key is entered.
void keyCallback (GLFWwindow * windowPtr, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS && key == GLFW_KEY_H) {
		printHelp ();
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_F2) {
		//meshPtr->reinitialisation(); / Implémentation d'une fonction mais ne fonctionne pas pour simplify
		meshPtr->clear();
		try {
			MeshLoader::loadOFF (mshFile, meshPtr);
		} catch (std::exception & e) {
		}
		meshPtr->init();
		if (!simpl){
			simpl = true;
		}
		if (!subd){
			subd = true;
		}
		if (xtoon != 0.f){
			xtoon = 0.f;
		}
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_T) {
		xtoon = 1.f - xtoon;
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_S) {
		if (simpl){
			meshPtr->simplify(25);
			simpl = false;
		}
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_S) {
		if (simpl){
			meshPtr->adaptiveSimplify(10);
			simpl = false;
		}
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_P) {
		if (subd){
			std::cout<<" A "<<std::endl;
			meshPtr->subdivide();
			std::cout<<"hello j'arrive jusqu'ici pourtant"<<std::endl;
			subd = false;
		}
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_A) {
		if(adap){
			meshPtr->adaptiveSimplify(100);
			adap = false;
		}
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_L) {
		meshPtr->laplacianFilter();
		meshPtr->recomputePerVertexNormals();
	}
	if (action == GLFW_PRESS && key == 49) { // touche 1
		meshPtr->laplacianFilter(0.1, true);
		meshPtr->recomputePerVertexNormals();
	}
	if (action == GLFW_PRESS && key == 50) { // touche 2
		meshPtr->laplacianFilter(0.5, true);
		meshPtr->recomputePerVertexNormals();
	}
	if (action == GLFW_PRESS && key == 51) { // touche 3
		meshPtr->laplacianFilter(1.f, true);
		meshPtr->recomputePerVertexNormals();
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_F1) {
		GLint mode[2];
		glGetIntegerv (GL_POLYGON_MODE, mode);
		glPolygonMode (GL_FRONT_AND_BACK, mode[1] == GL_FILL ? GL_LINE : GL_FILL);
	} else if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose (windowPtr, true); // Closes the application if the escape key is pressed
	}
}

/// Called each time the mouse cursor moves
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	float normalizer = static_cast<float> ((width + height)/2);
	float dx = static_cast<float> ((baseX - xpos) / normalizer);
	float dy = static_cast<float> ((ypos - baseY) / normalizer);
	if (isRotating) {
		glm::vec3 dRot (-dy * M_PI, dx * M_PI, 0.0);
		cameraPtr->setRotation (baseRot + dRot);
	}
	else if (isPanning) {
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (dx, dy, 0.0));
	} else if (isZooming) {
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (0.0, 0.0, dy));
	}
}

/// Called each time a mouse button is pressed
void mouseButtonCallback (GLFWwindow * window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    	if (!isRotating) {
    		isRotating = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseRot = cameraPtr->getRotation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    	isRotating = false;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    	if (!isPanning) {
    		isPanning = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    	isPanning = false;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    	if (!isZooming) {
    		isZooming = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    	isZooming = false;
    }
}

void initGLFW () {
	// Initialize GLFW, the library responsible for window management
	if (!glfwInit ()) {
		std::cerr << "ERROR: Failed to init GLFW" << std::endl;
		std::exit (EXIT_FAILURE);
	}

	// Before creating the window, set some option flags
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint (GLFW_RESIZABLE, GL_TRUE);

	// Create the window
	windowPtr = glfwCreateWindow (1024, 768, "Computer Graphics - Practical Assignment", nullptr, nullptr);
	if (!windowPtr) {
		std::cerr << "ERROR: Failed to open window" << std::endl;
		glfwTerminate ();
		std::exit (EXIT_FAILURE);
	}

	// Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
	glfwMakeContextCurrent (windowPtr);

	/// Connect the callbacks for interactive control
	glfwSetWindowSizeCallback (windowPtr, windowSizeCallback);
	glfwSetKeyCallback (windowPtr, keyCallback);
	glfwSetCursorPosCallback(windowPtr, cursorPosCallback);
	glfwSetMouseButtonCallback (windowPtr, mouseButtonCallback);
}

void exitOnCriticalError (const std::string & message) {
	std::cerr << "> [Critical error]" << message << std::endl;
	std::cerr << "> [Clearing resources]" << std::endl;
	clear ();
	std::cerr << "> [Exit]" << std::endl;
	std::exit (EXIT_FAILURE);
}

void initOpenGL () {
	// Load extensions for modern OpenGL
	if (!gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress))
		exitOnCriticalError ("[Failed to initialize OpenGL context]");

	glEnable (GL_DEBUG_OUTPUT); // Modern error callback functionnality
	glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS); // For recovering the line where the error occurs, set a debugger breakpoint in DebugMessageCallback
    glDebugMessageCallback (debugMessageCallback, 0); // Specifies the function to call when an error message is generated.
	glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
	glEnable (GL_DEPTH_TEST); // Enable the z-buffer test in the rasterization
	glClearColor (0.0f, 0.0f, 0.0f, 1.0f); // specify the background color, used any time the framebuffer is cleared
	// Loads and compile the programmable shader pipeline
	try {
		shaderProgramPtr = ShaderProgram::genBasicShaderProgram (SHADER_PATH + "VertexShader.glsl",
													         	 SHADER_PATH + "FragmentShader.glsl");
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading shader program]") + e.what ());
	}
}

void initScene (const std::string & meshFilename) {
	// Camera
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	cameraPtr = std::make_shared<Camera> ();
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));

	// Mesh
	meshPtr = std::make_shared<Mesh> ();
	try {
		MeshLoader::loadOFF (meshFilename, meshPtr);
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading mesh]") + e.what ());
	}

	meshPtr->computeCotangentWeights();
	meshPtr->init ();

	// Lighting
	//glm::vec3 lightSourcePosition (10.0, 10.0, 10.0);
	//glm::vec3 lightSourceColor (1.0, 1.0, 1.0);
	//float lightSourceIntensity = 1.f;
	lPtr->setPosition (shaderProgramPtr, glm::vec3(0.0, 0.0, 10.0));
	lPtr->setColor (shaderProgramPtr, glm::vec3(1.0, 0.0, 0.0));
	lPtr->setIntensity (shaderProgramPtr, 1);
	lPtr->setAc (shaderProgramPtr, 1);
	lPtr->setAl (shaderProgramPtr, 0);
	lPtr->setAq (shaderProgramPtr, 0);

	LightSource * lPtrb = new LightSource();
	lPtrb->setPositionB (shaderProgramPtr, glm::vec3(0.0, 10.0, 0.0));
	lPtrb->setColorB (shaderProgramPtr, glm::vec3(0.0, 0.0, 1.0));
	lPtrb->setIntensityB (shaderProgramPtr, 1);
	lPtrb->setAcB (shaderProgramPtr, 1);
	lPtrb->setAlB (shaderProgramPtr, 0);
	lPtrb->setAqB (shaderProgramPtr, 0);

	LightSource * lPtrbi = new LightSource();
	lPtrbi->setPositionBi (shaderProgramPtr, glm::vec3(0.0, 0.0, 10.0));
	lPtrbi->setColorBi (shaderProgramPtr, glm::vec3(0.0, 1.0, 0.0));
	lPtrbi->setIntensityBi (shaderProgramPtr, 1);
	lPtrbi->setAcBi (shaderProgramPtr, 1);
	lPtrbi->setAlBi (shaderProgramPtr, 0);
	lPtrbi->setAqBi (shaderProgramPtr, 0);

	// Material
	//glm::vec3 materialAlbedo (0.4, 0.6, 0.2);
	//shaderProgramPtr->set ("material.albedo", materialAlbedo);
	Material * mPtr = new Material();
	mPtr->setAlbedo(shaderProgramPtr, glm::vec3(1.0, 1.0, 1.0));
	float Ks = 1.0;
	mPtr->setKs(shaderProgramPtr, Ks);
	float S = 15.0;
	mPtr->setS(shaderProgramPtr, S);
	float alpha = 0.1;
	mPtr->setAlpha(shaderProgramPtr, alpha);
	float f0 = 0.91;
	mPtr->setF(shaderProgramPtr, f0);

	albedoTex = mPtr->loadTextureFromFileToGPU ("Sources/Materials/Metal/Base_Color.png");
	roughnessTex = mPtr->loadTextureFromFileToGPU ("Sources/Materials/Metal/Roughness.png");
	metallicTex = mPtr->loadTextureFromFileToGPU ("Sources/Materials/Metal/Metallic.png");
	occlusionTex = mPtr->loadTextureFromFileToGPU ("Sources/Materials/Metal/Ambient_Occlusion.png");
	xtoonTex = mPtr->loadTextureFromFileToGPU ("Sources/essai.bmp");

	// Adjust the camera to the actual mesh
	glm::vec3 center;
	meshPtr->computeBoundingSphere (center, meshScale);
	cameraPtr->setTranslation (center + glm::vec3 (0.0, 0.0, 3.0 * meshScale));
	cameraPtr->setNear (meshScale / 100.f);
	cameraPtr->setFar (6.f * meshScale);
}

void init (const std::string & meshFilename) {
	initGLFW (); // Windowing system
	initOpenGL (); // OpenGL Context and shader pipeline
	initScene (meshFilename); // Actual scene to render
}

void clear () {
	cameraPtr.reset ();
	meshPtr.reset ();
	shaderProgramPtr.reset ();
	glfwDestroyWindow (windowPtr);
	glfwTerminate ();
}

// The main rendering call
void render () {
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
	shaderProgramPtr->use (); // Activate the program to be used for upcoming primitive

	if (xtoon)	{
		shaderProgramPtr->set("changingToon",1.f);
	}
	else {
		shaderProgramPtr->set("changingToon",0.f);
	}

	if (xtoon)	{
		shaderProgramPtr->set("xtoon",1.f);
	}
	else {
		shaderProgramPtr->set("xtoon",0.f);
	}

	glClearColor(0.0, 0.1, 0.6, 1);

	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, albedoTex);
	shaderProgramPtr->set ("material.albedoTex", 0);

	glActiveTexture (GL_TEXTURE1);
	glBindTexture (GL_TEXTURE_2D, roughnessTex);
	shaderProgramPtr->set ("material.roughnessTex", 1);

	glActiveTexture (GL_TEXTURE2);
	glBindTexture (GL_TEXTURE_2D, metallicTex);
	shaderProgramPtr->set ("material.metallicTex", 2);

	glActiveTexture (GL_TEXTURE3);
	glBindTexture (GL_TEXTURE_2D, occlusionTex);
	shaderProgramPtr->set ("material.occlusionTex", 3);

	glActiveTexture (GL_TEXTURE4);
	glBindTexture (GL_TEXTURE_2D, xtoonTex);
	shaderProgramPtr->set ("material.xtoonTex", 4);

	LightSource * lPtr = new LightSource();
	lPtr->setPosition (shaderProgramPtr, glm::vec3(f1, f2, 10.0));
	glm::mat4 projectionMatrix = cameraPtr->computeProjectionMatrix ();
	shaderProgramPtr->set ("projectionMat", projectionMatrix); // Compute the projection matrix of the camera and pass it to the GPU program

	glm::mat4 modelMatrix = meshPtr->computeTransformMatrix ();
	glm::mat4 viewMatrix = cameraPtr->computeViewMatrix ();
	glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
	glm::mat4 normalMatrix = glm::transpose (glm::inverse (modelViewMatrix));
	shaderProgramPtr->set ("modelViewMat", modelViewMatrix);
	shaderProgramPtr->set ("normalMat", normalMatrix);
	meshPtr->render ();
	shaderProgramPtr->stop ();
}

// Update any accessible variable based on the current time
void update (float currentTime) {
	// Animate any entity of the program here
	shaderProgramPtr->use ();
	glm::vec4 lPosition1 = glm::vec4(0.0, 0.0, 10.0, 1.0);
	glm::vec4 lPosition2 = glm::vec4(0.0, 10.0, 0.0, 1.0);
	glm::vec4 lPosition3 = glm::vec4(0.0, 0.0, 10.0, 1.0);
	glm::mat4 modelMatrix = meshPtr->computeTransformMatrix ();
	glm::mat4 viewMatrix = cameraPtr->computeViewMatrix ();

	glm::vec4 v1 = viewMatrix * lPosition1;
	glm::vec4 v2 = viewMatrix * lPosition2;
	glm::vec4 v3 = viewMatrix * lPosition3;

	glm::vec3 lPos1 = glm::vec3(v1.x/v1.w, v1.y/v1.w, v1.z/v1.w);
	glm::vec3 lPos2 = glm::vec3(v2.x/v2.w, v2.y/v2.w, v2.z/v2.w);
	glm::vec3 lPos3 = glm::vec3(v3.x/v3.w, v3.y/v3.w, v3.z/v3.w);

	lPtr->setPosition(shaderProgramPtr, lPos1);
	lPtrb->setPositionB(shaderProgramPtr, lPos2);
	lPtrbi->setPositionBi(shaderProgramPtr, lPos3);

	static const float initialTime = currentTime;
	float dt = currentTime - initialTime;
	// <---- Update here what needs to be animated over time ---->

	//f1 = 100*sin(dt);
	//f2 = 10*sin(dt);
	f1 = 1;
	f2 = 1;

}

void usage (const char * command) {
	std::cerr << "Usage : " << command << " [<file.off>]" << std::endl;
	std::exit (EXIT_FAILURE);
}

int main (int argc, char ** argv) {
	if (argc > 2)
		usage (argv[0]);
	init (argc == 1 ? DEFAULT_MESH_FILENAME : argv[1]); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
	mshFile = DEFAULT_MESH_FILENAME;
	while (!glfwWindowShouldClose (windowPtr)) {
		update (static_cast<float> (glfwGetTime ()));
		render ();
		glfwSwapBuffers (windowPtr);
		glfwPollEvents ();
	}
	clear ();
	std::cout << " > Quit" << std::endl;
	return EXIT_SUCCESS;
}
