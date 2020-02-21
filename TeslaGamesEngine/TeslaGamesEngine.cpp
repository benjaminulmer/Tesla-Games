// TeslaGamesEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
// The TeslaGamesEngine will be changed to a .lib, not .exe and then be included in the TeslaGames Project

#include "pch.h"
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

// Rendering includes
#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtx\rotate_vector.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
#include "Model.h"

//Health stuff
#include "Combat.h"
#include "HealthComponent.h"
#include "DamagingObject.h"
#include "Vehicle.h"
#include "Game.h"

//AudioStuff
#include "AudioEngine.h"
#include "AudioBoomBox.h"

//Controller stuff
#include "Controller.h"

//PhysX and Physics Engine
#include "PhysicsEngine.h"

//HUD stuff
#include "HUDcreator.h"

//Shadow stuff
#include "Shadow.h"

// Stuff for imgui
#include "imGui/imgui.h"
#include "imGui/imgui_impl_glfw.h"
#include "imGui/imgui_impl_opengl3.h"

// end of stuff for imgui

struct localAxis {
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
};


/* Rendering variables */
const float toRadians = 3.14159265f / 180.0f;

//sWindow mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
std::vector<HUD*> HUDList;
Camera camera;
Shader *depthShader;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture floorTexture;

Material shinyMaterial;
Material dullMaterial;

PhysicsEngine physEng;

Model TeslaCar;
Model racetrack;
Model bulletobj;
Model boxTest;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];


//helper variables
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

bool bullet_shot = false; //after shooting there will be a cooldown for the player before he can shoot again
bool bullet_sound_played = true;



float shoot_distance_x = 0; // Bullet vector movement for x
float shoot_distance_y = 0; // Bullet vector movement for y
float shoot_distance_z = 0; //Bullet vector movement for z
float bullet_speed = 0.5f;  //velocity of bullet when traversing

float bullet_boundary = 15;

//Mesh positioning and rotation debugging for player/car obj (current position for CAR)
float pos_x = 0;
float pos_y = 0;
float pos_z = 0;

//Angle of rotation for player/car obj  
float car_rotation = 90;
float current_rotation; //Calculates the angle at the moment of firing lazer
glm::vec3 car_front;

// Vertex Shader
static const char* vShader = "Shaders/shader.vert";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag";

//Depth shader
static const char* vDepthShader = "Shaders/depthShader.vert";
static const char* fDepthShader = "Shaders/depthShader.frag";

//Shadow stuff
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

struct yawPitch {
	float yaw;
	float pitch;
};

void update(localAxis a, float yaw, float pitch) {
	a.front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	a.front.y = sin(glm::radians(pitch));
	a.front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	a.front = glm::normalize(a.front);

	a.right = glm::normalize(glm::cross(a.front, glm::vec3(0,1,0)));
	a.up = glm::normalize(glm::cross(a.right, a.front));
}

/* End of rendering variables */

// function for interpolating normals of a mesh
void calcAverageNormals(unsigned int * indices, unsigned int indiceCount, GLfloat * vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

// create basic objects for testing
void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
}

void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
	
	depthShader = new Shader();
	depthShader->createDepthShaderFromFiles(vDepthShader, fDepthShader);
}


// A function to obtain input, called each frame
//add vehicle movement to these FOR NOW
//TO DO: Someone comment all the controls for each button
void parseControllerInput(Controller* controller)
{
	// Update controller object with current input MUST BE FIRST
	controller->update();

	//IMPLEMENT THINGS In the IFs

	//Is button Pressed demo
	if (controller->isButtonPressed(XButtons.A)) {
		std::cout << controller->getIndex() << " " <<"A PRESSED" << std::endl;
	}
	if (controller->isButtonPressed(XButtons.X)) {
		std::cout << controller->getIndex() << " " << "X PRESSED" << std::endl;
	}
	
	//Is button down demo (more useful IMO)
	if (controller->isButtonDown(XButtons.Y)) {
		std::cout << controller->getIndex() << " " << "Y PRESSED and HELD" << std::endl;
	}
	if (controller->isButtonDown(XButtons.B)) {
		std::cout << controller->getIndex() << " " << "B PRESSED and HELD" << std::endl;
	}
	if (controller->isButtonDown(XButtons.L_Shoulder)) {
		std::cout << controller->getIndex() << " " << "LB PRESSED and HELD" << std::endl;
		bullet_shot = true; //Allows for bullets to be rendered
		bullet_sound_played = false;
		current_rotation = car_rotation;

	}
	if (controller->isButtonDown(XButtons.R_Shoulder)) {
		std::cout << controller->getIndex() << " " << "RB PRESSED and HELD" << std::endl;
		bullet_shot= true; // Alllows for bullets to be rendered
		bullet_sound_played = false;
		current_rotation = car_rotation;

	}
	if (controller->isButtonDown(XButtons.DPad_Up)) {
		std::cout << controller->getIndex() << " " << "D-Pad Up PRESSED and HELD" << std::endl;
	}
	if (controller->isButtonDown(XButtons.DPad_Down)) {
		std::cout << controller->getIndex() << " " << "D-Pad Down PRESSED and HELD" << std::endl;
	}
	if (controller->isButtonDown(XButtons.DPad_Right)) {
		std::cout << controller->getIndex() << " " << "D-Pad Right PRESSED and HELD" << std::endl;
	}
	if (controller->isButtonDown(XButtons.DPad_Left)) {
		std::cout << controller->getIndex() << " " << "D-Pad Left PRESSED and HELD" << std::endl;
	}
	if (controller->isButtonDown(XButtons.Start)) {
		std::cout << controller->getIndex() << " " << "Start PRESSED and HELD" << std::endl;
	}
	if (controller->isButtonDown(XButtons.Back)) {
		std::cout << controller->getIndex() << " " << "Back PRESSED and HELD" << std::endl;
	}
	if (controller->isButtonDown(XButtons.Back)) {
		std::cout << controller->getIndex() << " " << "Back PRESSED and HELD" << std::endl;
	}
	if (controller->isButtonDown(XButtons.L_Thumbstick)) {
		std::cout << controller->getIndex() << " " << "L3 PRESSED and HELD" << std::endl;
	}
	if (controller->isButtonDown(XButtons.R_Thumbstick)) {
		std::cout << controller->getIndex() << " " << "R3 PRESSED and HELD" << std::endl;
	}

	//Sticks and triggers
	if (!controller->LStick_InDeadzone()) {
		//physEng.turn(controller->leftStick_X());
		std::cout << controller->getIndex() << " " << "LS: " << controller->leftStick_X() << std::endl;
	}
	if (!controller->RStick_InDeadzone()) {
		//physEng.turn(controller->leftStick_X());
		std::cout << controller->getIndex() << " " << "RS: " << controller->rightStick_X() << std::endl;
	}
	if (controller->rightTrigger() > 0.0) {
		physEng.forwards(controller->rightTrigger());
		
		std::cout << controller->getIndex() << " " << "Right Trigger: " << controller->rightTrigger() << std::endl;
	}
	else {
		physEng.forwards(0.1f);
	}
	if (controller->leftTrigger() > 0.0) {
		physEng.reverse(controller->leftTrigger());
		std::cout << controller->getIndex() << " " << "Left Trigger: " << controller->leftTrigger() << std::endl;
	}
	physEng.turn(controller->leftStick_X());

	// Update the gamepad for next frame MUST BE LAST
	controller->refreshState();
}

int main()
{
	const char* glsl_version = "#version 130"; // USED FOR IMGUI SETTING
	mainWindow = Window(1280, 720);
	mainWindow.Initialise();

	HUDcreator hud;
	hud.load();

	Shadow shadow = Shadow::Shadow();

	Renderer r = Renderer(mainWindow, camera);

	Game mainGame = Game(r);
	Object* car = new Vehicle(1);
	Object* car2 = new Vehicle(2);
	Object* bullet = new DamagingObject(20, 1);

	mainGame.AddObject(car);
	mainGame.AddObject(car2);
	mainGame.AddObject(bullet);

	mainGame.Play();

	// Rendering setup
	CreateObjects();
	CreateShaders();


	camera = Camera(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 20.0f, -20.0f, 5.0f, 0.5f);
	yawPitch yp;
	yp.yaw = 90.f;
	yp.pitch = -20.0f;


	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureAlpha();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureAlpha();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureAlpha();
	floorTexture = Texture("Textures/floorConcrete.png");
	floorTexture.LoadTexture();
	
	shinyMaterial = Material(4.0f, 256);
	dullMaterial = Material(0.3f, 4);

	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 
								0.5f, 0.5f,
								0.0f, 0.0f, -1.0f);

	unsigned int pointLightCount = 0;
	pointLights[0] = PointLight(0.0f, 0.0f, 1.0f,
								0.0f, 0.1f,
								0.0f, 0.0f, 0.0f,
								0.3f, 0.2f, 0.1f);
	//pointLightCount++;
	pointLights[1] = PointLight(0.0f, 1.0f, 0.0f,
								0.0f, 0.1f,
								-4.0f, 2.0f, 0.0f,
								0.3f, 0.1f, 0.1f);
	//pointLightCount++;
	
	unsigned int spotLightCount = 0;
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
						0.0f, 2.0f,
						0.0f, 0.0f, 0.0f,
						0.0f, -1.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						20.0f);
	spotLightCount++;
	spotLights[1] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, -1.5f, 0.0f,
		-100.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);
	spotLightCount++;
	
	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformLightSpaceMatrix = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

	TeslaCar.LoadModel("Models/TeslaGamesTruck2.obj");
	boxTest.LoadModel("Models/wall.obj");
	//TeslaCar.LoadModel("Models/TeslaGamesTruck.obj");
	racetrack.LoadModel("Models/track2.obj");
	bulletobj.LoadModel("Models/bullet.obj");

	shadow.createShadowMap();

	// TODO: Put FPS code into Game.Play()
	// Loop until window closed

	glfwSwapInterval(1);
	// imGui setting BEGINNING
	/*
#if __APPLE__
// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
*/
// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings

	ImGui_ImplGlfw_InitForOpenGL(mainWindow.getWindow(), true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Imgui setting END

	//Audio system setup
	AudioEngine audioSystem = AudioEngine();
	AudioBoomBox audioObject = audioSystem.createBoomBox(audioConstants::SOUND_FILE_TTG_MAIN_MENU);
	AudioBoomBox audioObject2 = audioSystem.createBoomBox(audioConstants::SOUND_FILE_TTG_RACE);
	AudioBoomBox audioObject3 = audioSystem.createBoomBox(audioConstants::SOUND_FILE_BOUNCE);

	//The key is now that multiple sounds can be played at once. As long as sound card can support it
	//Comment out one sound if you dont wanna hear it
	//audioObject.playSound();
	audioObject2.playSound();

	//Controller
	Controller player1 = Controller(1);
	Controller player2 = Controller(2);

	bool P1Connected = player1.isConnected();
	bool P2Connected = player2.isConnected();

	std::cout << "Player1 connected: " << P1Connected << std::endl;
	std::cout << "Player2 connected: " << P2Connected << std::endl;

	//physEng.upwards();
	//End of audio system setup/demo

	glm::vec3 lightPos(-10.f, 10.0f, -10.f);

	while (!mainWindow.getShouldClose())
	{
		physEng.stepPhysics();

		const physx::PxVehicleDrive4W* vehicle = physEng.gVehicle4W;	//get vehicle
		const physx::PxRigidDynamic* vDynamic = vehicle->getRigidDynamicActor();
		physx::PxQuat vehicleQuaternion = vDynamic->getGlobalPose().q;
		physx::PxVec3 v_dir = vehicleQuaternion.getBasisVector2();
		const physx::PxVec3 vehiclePositionPhysx = vDynamic->getGlobalPose().p;
		glm::vec3 vehiclePosition(vehiclePositionPhysx.x, vehiclePositionPhysx.y, vehiclePositionPhysx.z);

		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// Get + Handle User Input
		glfwPollEvents();
		if (P1Connected)
			parseControllerInput(&player1);
		if (P2Connected)
			parseControllerInput(&player2);

		//camera.keyControl(mainWindow.getsKeys(), deltaTime);
		//camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		glm::mat4 model = glm::mat4(1.0f);
		float near_plane = 1.0f, far_plane = 75;
		lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		//use depth shader
		unsigned int depthMapFBO = 0;
		depthShader->UseShader();
		uniformLightSpaceMatrix = depthShader->GetLightSpaceMatrix();
		glUniformMatrix4fv(uniformLightSpaceMatrix, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, -2.f));
		model = glm::scale(model, glm::vec3(10.f, 10.f, 10.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		racetrack.RenderModel();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//reset view port
		glViewport(0, 0, mainWindow.getWidth(), mainWindow.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Setup shader
		// 2. render scene as normal using the generated depth/shadow map  
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();
		uniformLightSpaceMatrix = shaderList[0].GetLightSpaceMatrix();

		glm::vec3 lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		physx::PxVec3 carPos = physEng.GetPosition();	//position of TeslaCar

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniformMatrix4fv(uniformLightSpaceMatrix, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();

		// Draw pyramid one
		
/*
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
		//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		brickTexture.UseTexture();
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[0]->RenderMesh();

		// Draw pyramid two
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 4.0f, -2.5f));
		//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dirtTexture.UseTexture();
		dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[1]->RenderMesh();
		// Draw base
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dirtTexture.UseTexture();
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();
		*/

		//render box
		//get position of actual wall
		physx::PxVec3 wallPos = physEng.GetBoxPos();
		glm::vec3 wallp(wallPos.x, wallPos.y, wallPos.z);

		model = glm::mat4(1.0f);
		model = glm::translate(model, wallp);
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		boxTest.RenderModel();
		

//////////////////////////////////////////////////////////////////////////
		

		// Draw racing track
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, -2.f));
		model = glm::scale(model, glm::vec3(10.f,10.f, 10.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		racetrack.RenderModel();

		///////////////////////////////////////////////////////////////////////
		physx::PxVec3 forwardvec = physx::PxVec3(vehicleQuaternion.x, 0, vehicleQuaternion.z);	//holds camera vectors that match the car

		physx::PxVec3  Direction = vehicleQuaternion.getBasisVector2();
/////////////////////////////////////////////////////////////////////////////////
		//Draw bullets
		
		if (bullet_shot) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(vehiclePosition.x +shoot_distance_x, vehiclePosition.y+0.5f+shoot_distance_y, vehiclePosition.z+shoot_distance_z));
			
			model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			bulletobj.RenderModel();

			//PLAY LASER SOUND
			if (!bullet_sound_played) {
				audioObject3.playSound();
				bullet_sound_played = true; //Stop once its played once

			}
		
			shoot_distance_x += Direction.x *bullet_speed;
			shoot_distance_z += Direction.z * bullet_speed;

			if (shoot_distance_x > bullet_boundary || shoot_distance_x < -bullet_boundary || shoot_distance_z > bullet_boundary ||  shoot_distance_z < -bullet_boundary) {
				shoot_distance_x = 0;
				shoot_distance_y= 0;
				shoot_distance_z = 0;
				bullet_shot = false;
				std::cout <<"LAZER COOLDOWN IS OVER!" << std::endl;
			}
			
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		


		physx::PxMat44 modelMat(vDynamic->getGlobalPose());	//make model matrix from transform of rigid dynamic
		modelMat.scale(physx::PxVec4(0.3f, 0.3f, 0.3f, 1.f));	//scales the model
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, modelMat.front());
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(carPos.x, carPos.y, carPos.z));	//translate to physx vehicle pos




		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		TeslaCar.RenderModel();

		glm::vec3 dir = glm::normalize(glm::vec3(v_dir.x, 0, v_dir.z));
		float dist = 6.5;														//distance between camera and vehicle
		camera.setFront(dir.x, -0.5, dir.z);
		float xoffset = dist * dir.x;
		float zoffset = dist * dir.z;
		camera.setPosition(carPos.x - xoffset, carPos.y + 5 , carPos.z - zoffset);


		car_rotation = vehicleQuaternion.getAngle();
		

		//HUD staars here
		hud.use();

/*
		
		//HUD ends here

*/
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		// End of rendering 

		// Start the Dear ImGui frame

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		{
			static float f = 0.0f;
			static int counter = 0;

/*
			ImGui::Begin("FPS COUNTER");                          // Create a window called "Hello, world!" and append into it.

		


			ImGui::SliderFloat("OBJ X pos debug", &pos_x, 0.0f, 20.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::SliderFloat("OBJ Y pos debug", &pos_y, 0.0f, 20.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::SliderFloat("OBJ Z pos debug", &pos_z, 0.0f, 20.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

			ImGui::Text("OBJ angle debug");               // Display some text (you can use a format strings too)
			ImGui::SliderFloat("angle",&car_rotation , 0.0f, 360.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

			ImGui::Text("Frame per Second counter");               // Display some text (you can use a format strings too)

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
*/
			ImGui::Begin("Debug");
			ImGui::Text("Driving mode and Position");
			ImGui::Text("Frame per Second counter");               // Display some text (you can use a format strings too)
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Drivemode: %i Xpos: %f Ypos: %f Zpos: %f", physEng.getModeType(), carPos.x, carPos.y, carPos.z);
			ImGui::Text("Drivemode: %i Xvec: %f Yvec: %f Zvec: %f", physEng.getModeType(), vehicleQuaternion.x, vehicleQuaternion, vehicleQuaternion.z);
			ImGui::Text("Drivemode: %i Xvec: %f Yvec: %f Zvec: %f", physEng.getModeType(), v_dir.x, v_dir.y, v_dir.z);

			ImGui::End();
		}

		// Rendering imgui
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(mainWindow.getWindow(), &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		//glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		//glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// imgui ends here

		// TODO: Load shader in a material struct in the model (Basically all of the following code refactored to being in model


		mainWindow.swapBuffers();


		//Shooting actions
		
		
	}
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	


	return 0;
}