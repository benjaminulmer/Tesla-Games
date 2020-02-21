#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "pch.h"

class Shadow
{
public:
	Shadow();

	void createShadowMap();
	void loadShadowMap();
	void generateShadowMap();
	void renderShadow();

private:
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	unsigned int depthMap;
};

