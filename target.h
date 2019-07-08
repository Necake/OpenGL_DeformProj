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
	Target(const char* modelPath, float falloff, float roughness, float threshold):
		targetModel(modelPath, true), falloff(falloff), roughness(roughness), threshold(threshold)
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

	//Calculates ray falloff given the material parameters, returns intensity in % of original force
	float falloffFunc(float input) 
	{
		std::cout << "registered falloff with: " << input << " ";
		if (input >= falloff) //input needs to be from 0 to falloff, otherwise we go negative
			return 0;		  //since the input is going to be vector distance, it's always going to be >=0

		float res = powf((1 - (input / falloff) * (input / falloff)), roughness);
		if (res>1.0f)
			std::cout << "intensity " << res * 100 << "%\n";
		return res;
	}

private:
	float falloff;
	float roughness;
	float threshold;
};

#endif