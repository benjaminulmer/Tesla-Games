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
#include "Skybox.h"


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

//HUD stuff
#include "HUD.h"

// Stuff for imgui
#include "imGui/imgui.h"
#include "imGui/imgui_impl_glfw.h"
#include "imGui/imgui_impl_opengl3.h"

// end of stuff for imgui



/* Rendering variables */
const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
std::vector<HUD*> HUDList;
Camera camera;

Shader hudShader;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;

Skybox skybox;

GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
uniformSpecularIntensity = 0, uniformShininess = 0;

//digit textures
Texture dig0Texture;
Texture dig1Texture;
Texture dig2Texture;
Texture dig3Texture;

//HUD textures
Texture weaponUITexture;
Texture emptyBarTexture;
Texture healthBarTexture;
Texture nitroBarTexture;
Texture plusSymbolTexture;
Texture nitroSymbolTexture;
Texture flagTexture;
Texture personTexture;
Texture cupTexture;

Material shinyMaterial;
Material dullMaterial;

Model xwing;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// Vertex Shader
static const char* vShader = "Shaders/shader.vert";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag";

// Vertex Shader of HUD_shader
static const char* vHshader = "Shaders/HUD_shader.vert";

//Fragment shader of HUD_shader
static const char* fHshader = "Shaders/HUD_shader.frag";

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
	
	hudShader.createHUDFromFiles(vHshader, fHshader);

}

void CreateHUDs() {
	unsigned int HUDindecis[] = {						// 0 -----3
		0, 1, 3,										// |	  |
		2, 1, 3											// 1 -----2
	};

	GLfloat weaponUIVertices[] = {
	//	x	 y	  z			u	 v
		0.0, 474.0, 0.0,	0.0, 0.0,								//bottom left
		0.0, 600.0, 0.0,	0.0, 1.0,								//top left
		259.0, 600, 0.0,	1.0, 1.0,								//top right
		259.0, 474.0, 0.0,	1.0, 0.0								//bottom right
	};
	
	GLfloat numOfWeaponVertices[] = {
		120.125, 500.5, 1.0,		0.0, 0.0,
		120.125, 532.0, 1.0,		0.0, 1.0,
		138.875, 532.0, 1.0,		1.0, 1.0,
		138.875, 500.5, 1.0,		1.0, 0.0
	};

	GLfloat emptyBar1Vertices[] = {
		600.0, 580.0, 0.0,		0.0, 0.0,
		600.0, 600.0, 0.0,		0.0, 1.0,
		800.0, 600.0, 0.0,		1.0, 1.0,
		800.0, 580.0, 0.0,		1.0, 0.0
	};	
	
	GLfloat emptyBar2Vertices[] = {
		600.0, 555.0, 0.0,		0.0, 0.0,
		600.0, 575.0, 0.0,		0.0, 1.0,
		800.0, 575.0, 0.0,		1.0, 1.0,
		800.0, 555.0, 0.0,		1.0, 0.0
	};

	GLfloat plusVertices[] = {
		690.0, 580.0, 0.0,		0.0, 0.0,
		690.0, 600.0, 0.0,		0.0, 1.0,
		710.0, 600.0, 0.0,		1.0, 1.0,
		710.0, 580.0, 0.0,		1.0, 0.0
	};	
	
	GLfloat nitroSymbolVertices[] = {
		690.0, 555.0, 0.0,		0.0, 0.0,
		690.0, 575.0, 0.0,		0.0, 1.0,
		710.0, 575.0, 0.0,		1.0, 1.0,
		710.0, 555.0, 0.0,		1.0, 0.0
	};

	GLfloat cupVertices[] = {
		695.0, 10.0, 0.0,		0.0, 0.0,
		695.0, 40.0, 0.0,		0.0, 1.0,
		725.0, 40.0, 0.0,		1.0, 1.0,
		725.0, 10.0, 0.0,		1.0, 0.0
	};
	
	GLfloat rank1numVertices[] = {
		730.0, 10.0, 0.0,		0.0, 0.0,
		730.0, 40.0, 0.0,		0.0, 1.0,
		760.0, 40.0, 0.0,		1.0, 1.0,
		760.0, 10.0, 0.0,		1.0, 0.0
	};
	
	GLfloat rank2numVertices[] = {
		765.0, 10.0, 0.0,		0.0, 0.0,
		765.0, 40.0, 0.0,		0.0, 1.0,
		795.0, 40.0, 0.0,		1.0, 1.0,
		795.0, 10.0, 0.0,		1.0, 0.0
	};
	
	GLfloat flagVertices[] = {
		695.0, 50.0, 0.0,		0.0, 0.0,
		695.0, 80.0, 0.0,		0.0, 1.0,
		725.0, 80.0, 0.0,		1.0, 1.0,
		725.0, 50.0, 0.0,		1.0, 0.0
	};

	GLfloat lap1numVertices[] = {
		730.0, 50.0, 0.0,		0.0, 0.0,
		730.0, 80.0, 0.0,		0.0, 1.0,
		760.0, 80.0, 0.0,		1.0, 1.0,
		760.0, 50.0, 0.0,		1.0, 0.0
	};

	GLfloat lap2numVertices[] = {
		765.0, 50.0, 0.0,		0.0, 0.0,
		765.0, 80.0, 0.0,		0.0, 1.0,
		795.0, 80.0, 0.0,		1.0, 1.0,
		795.0, 50.0, 0.0,		1.0, 0.0
	};

	GLfloat personVertices[] = {
		695.0, 90.0, 0.0,		0.0, 0.0,
		695.0, 120.0, 0.0,		0.0, 1.0,
		725.0, 120.0, 0.0,		1.0, 1.0,
		725.0, 90.0, 0.0,		1.0, 0.0
	};

	GLfloat alive1numVertices[] = {
		730.0, 90.0, 0.0,		0.0, 0.0,
		730.0, 120.0, 0.0,		0.0, 1.0,
		760.0, 120.0, 0.0,		1.0, 1.0,
		760.0, 90.0, 0.0,		1.0, 0.0
	};

	GLfloat alive2numVertices[] = {
		765.0, 90.0, 0.0,		0.0, 0.0,
		765.0, 120.0, 0.0,		0.0, 1.0,
		795.0, 120.0, 0.0,		1.0, 1.0,
		795.0, 90.0, 0.0,		1.0, 0.0
	};

	HUD* weaponUI = new HUD();
	weaponUI->createHUD(weaponUIVertices, HUDindecis, 20, 6);
	HUDList.push_back(weaponUI);

	HUD* numOfWeapon = new HUD();
	numOfWeapon->createHUD(numOfWeaponVertices, HUDindecis, 20, 6);
	HUDList.push_back(numOfWeapon);

	HUD* bar1 = new HUD();
	bar1->createHUD(emptyBar1Vertices, HUDindecis, 20, 6);
	HUDList.push_back(bar1);

	HUD* bar2 = new HUD();
	bar2->createHUD(emptyBar2Vertices, HUDindecis, 20, 6);
	HUDList.push_back(bar2);

	HUD* healthBar = new HUD();
	healthBar->createHUD(emptyBar1Vertices, HUDindecis, 20, 6);
	HUDList.push_back(healthBar);

	HUD* nitroBar = new HUD();
	nitroBar->createHUD(emptyBar2Vertices, HUDindecis, 20, 6);
	HUDList.push_back(nitroBar);

	HUD* plusSymbol = new HUD();
	plusSymbol->createHUD(plusVertices, HUDindecis, 20, 6);
	HUDList.push_back(plusSymbol);

	HUD* nitroSymbol = new HUD();
	nitroSymbol->createHUD(nitroSymbolVertices, HUDindecis, 20, 6);
	HUDList.push_back(nitroSymbol);

	HUD* cupUI = new HUD();
	cupUI->createHUD(cupVertices, HUDindecis, 20, 6);
	HUDList.push_back(cupUI);

	HUD* rank1num = new HUD();
	rank1num->createHUD(rank1numVertices, HUDindecis, 20, 6);
	HUDList.push_back(rank1num);
	
	HUD* rank2num = new HUD();
	rank2num->createHUD(rank2numVertices, HUDindecis, 20, 6);
	HUDList.push_back(rank2num);

	HUD* flagUI = new HUD();
	flagUI->createHUD(flagVertices, HUDindecis, 20, 6);
	HUDList.push_back(flagUI);

	HUD* lap1num = new HUD();
	lap1num->createHUD(lap1numVertices, HUDindecis, 20, 6);
	HUDList.push_back(lap1num);

	HUD* lap2num = new HUD();
	lap2num->createHUD(lap2numVertices, HUDindecis, 20, 6);
	HUDList.push_back(lap2num);

	HUD* personUI = new HUD();
	personUI->createHUD(personVertices, HUDindecis, 20, 6);
	HUDList.push_back(personUI);

	HUD* alive1num = new HUD();
	alive1num->createHUD(alive1numVertices, HUDindecis, 20, 6);
	HUDList.push_back(alive1num);
	
	HUD* alive2num = new HUD();
	alive2num->createHUD(alive2numVertices, HUDindecis, 20, 6);
	HUDList.push_back(alive2num);
}

// A function to obtain input, called each frame
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
	}
	if (controller->isButtonDown(XButtons.R_Shoulder)) {
		std::cout << controller->getIndex() << " " << "RB PRESSED and HELD" << std::endl;
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

	//Sticks and triggers may hurt some n********...
	// It was 'neighbors' geez....
	if (!controller->LStick_InDeadzone()) {
		std::cout << controller->getIndex() << " " << "LS: " << controller->leftStick_X() << std::endl;
	}
	if (!controller->RStick_InDeadzone()) {
		std::cout << controller->getIndex() << " " << "RS: " << controller->rightStick_X() << std::endl;
	}
	if (controller->rightTrigger() > 0.0) {
		std::cout << controller->getIndex() << " " << "Right Trigger: " << controller->rightTrigger() << std::endl;
	}
	if (controller->leftTrigger() > 0.0) {
		std::cout << controller->getIndex() << " " << "Left Trigger: " << controller->leftTrigger() << std::endl;
	}

	// Update the gamepad for next frame MUST BE LAST
	controller->refreshState();
}

void RenderScene()
{
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	brickTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 4.0f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTexture.UseTexture();
	dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[1]->RenderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[2]->RenderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-7.0f, 0.0f, 10.0f));
	model = glm::scale(model, glm::vec3(0.006f, 0.006f, 0.006f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	xwing.RenderModel();
}

void RenderPass(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
	glViewport(0, 0, 1366, 768);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skybox.DrawSkybox(viewMatrix, projectionMatrix);

	shaderList[0].UseShader();

	uniformModel = shaderList[0].GetModelLocation();
	uniformProjection = shaderList[0].GetProjectionLocation();
	uniformView = shaderList[0].GetViewLocation();
	uniformModel = shaderList[0].GetModelLocation();
	uniformEyePosition = shaderList[0].GetEyePositionLocation();
	uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
	uniformShininess = shaderList[0].GetShininessLocation();

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

	shaderList[0].SetDirectionalLight(&mainLight);

	glm::vec3 lowerLight = camera.getCameraPosition();
	lowerLight.y -= 0.3f;
	spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

	shaderList[0].Validate();

	RenderScene();
}

int main()
{
	const char* glsl_version = "#version 130"; // USED FOR IMGUI SETTING
	mainWindow = Window(800, 600);
	mainWindow.Initialise();

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
	CreateHUDs();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 5.0f, 0.5f);
	mainLight = DirectionalLight(1.0f, 0.53f, 0.3f, 0.1f, 0.9f, -10.0f, -12.0f, 18.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureAlpha();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureAlpha();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureAlpha();

	//load digits textures
	dig0Texture = Texture("Textures/numbers/0.png");
	dig0Texture.LoadTextureAlpha();
	dig1Texture = Texture("Textures/numbers/1.png");
	dig1Texture.LoadTextureAlpha();
	dig2Texture = Texture("Textures/numbers/2.png");
	dig2Texture.LoadTextureAlpha();
	dig3Texture = Texture("Textures/numbers/3.png");
	dig3Texture.LoadTextureAlpha();

	//load HUD textures
	weaponUITexture = Texture("Textures/HUD/WeaponsUI.png");
	weaponUITexture.LoadTextureAlpha();
	emptyBarTexture = Texture("Textures/HUD/emptybar.png");
	emptyBarTexture.LoadTextureAlpha();
	healthBarTexture = Texture("Textures/HUD/healthbar.png");
	healthBarTexture.LoadTextureAlpha();
	nitroBarTexture = Texture("Textures/HUD/nitro.png");
	nitroBarTexture.LoadTextureAlpha();
	plusSymbolTexture = Texture("Textures/HUD/plus.png");
	plusSymbolTexture.LoadTextureAlpha();
	nitroSymbolTexture = Texture("Textures/HUD/nitrosymbol.png");
	nitroSymbolTexture.LoadTextureAlpha();
	personTexture = Texture("Textures/HUD/alive.png");
	personTexture.LoadTextureAlpha();
	cupTexture = Texture("Textures/HUD/cup.png");
	cupTexture.LoadTextureAlpha();
	flagTexture = Texture("Textures/HUD/flags.png");
	flagTexture.LoadTextureAlpha();

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

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

	xwing.LoadModel("Models/x-wing.obj");

	std::vector<std::string> skyboxFaces;
	/*skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");*/

	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_right.jpg");
	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_left.jpg");
	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_up.jpg");
	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_down.jpg");
	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_back.jpg");
	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_front.jpg");

	skybox = Skybox(skyboxFaces);


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
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

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

	//The key is now that multiple sounds can be played at once. As long as sound card can support it
	//Comment out one sound if you dont wanna hear it
	audioObject.playSound();
	audioObject2.playSound();

	//Controller
	Controller player1 = Controller(1);
	Controller player2 = Controller(2);

	bool P1Connected = player1.isConnected();
	bool P2Connected = player2.isConnected();

	std::cout << "Player1 connected: " << P1Connected << std::endl;
	std::cout << "Player2 connected: " << P2Connected << std::endl;

	//End of audio system setup/demo
	while (!mainWindow.getShouldClose())
	{

		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// Get + Handle User Input
		glfwPollEvents();
		if (P1Connected)
			parseControllerInput(&player1);
		if (P2Connected)
			parseControllerInput(&player2);

		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		RenderPass(camera.calculateViewMatrix(), projection);


		//Rendering HUD
		hudShader.UseShader();
		uniformModel = hudShader.GetModelLocation();
		uniformProjection = hudShader.GetProjectionLocation();

		glm::mat4 ortho = glm::ortho(0.0f, (float)mainWindow.getWidth(), (float)mainWindow.getHeight(), 0.0f);						//orthograohic projection

		glDisable(GL_DEPTH_TEST);																									//disable the depth-testing

		glm::mat4 model = glm::mat4(1.0f);
		//glm::mat4 model = glm::mat4(1.0f);

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(ortho));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		//weapon UI
		weaponUITexture.UseTexture();
		HUDList[0]->renderHUD();
		
		//number of charges
		dig3Texture.UseTexture();
		HUDList[1]->renderHUD();

		//TODO: if out of charges, change ui

		//bars
		//empty bar1
		emptyBarTexture.UseTexture();
		HUDList[2]->renderHUD();

		//empty bar2
		emptyBarTexture.UseTexture();
		HUDList[3]->renderHUD();

		//health bar
		healthBarTexture.UseTexture();
		HUDList[4]->renderHUD();

		//nitro bar
		nitroBarTexture.UseTexture();
		HUDList[5]->renderHUD();
		
		//plus symbol
		plusSymbolTexture.UseTexture();
		HUDList[6]->renderHUD();

		//nitro symbol
		nitroSymbolTexture.UseTexture();
		HUDList[7]->renderHUD();

		//race info
		//current rank
		cupTexture.UseTexture();
		HUDList[8]->renderHUD();
		dig2Texture.UseTexture();
		HUDList[10]->renderHUD();

		//current laps
		flagTexture.UseTexture();
		HUDList[11]->renderHUD();
		dig3Texture.UseTexture();
		HUDList[13]->renderHUD();

		//current alive
		personTexture.UseTexture();
		HUDList[14]->renderHUD();
		dig1Texture.UseTexture();
		HUDList[15]->renderHUD();
		dig0Texture.UseTexture();
		HUDList[16]->renderHUD();

		glEnable(GL_DEPTH_TEST);


		//HUD ends here

		// End of rendering 

		// Start the Dear ImGui frame

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("FPS COUNTER");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("Frame per Second counter");               // Display some text (you can use a format strings too)

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
	}
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


	return 0;
}