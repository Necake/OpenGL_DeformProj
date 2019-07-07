#ifndef TARGET_H
#define TARGET_H

#include<GLAD\glad.h>
#include<GLFW\glfw3.h>
#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<glm\gtc\type_ptr.hpp>
#include<glm\gtc\constants.hpp>

#include<iostream>
#include<string>
#include "shader.h"
#include "model.h"
#include "rayUtil.h"

class Target
{
public:
	Target(const char* modelPath, float falloff, float softness, float threshold):
		targetModel(modelPath, true), falloff(falloff), softness(softness), threshold(threshold)
	{
		model = glm::mat4(1.0f);
		std::cout << "Successfully set up target\n";
	}

	void Draw(Shader& shader)
	{
		//this convention is assumed
		shader.setMat4("model", model);
		targetModel.Draw(shader);
	}

	Model targetModel;
	glm::mat4 model;
private:
	float falloff;
	float softness;
	float threshold;
};

#endif