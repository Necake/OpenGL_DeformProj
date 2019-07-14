#ifndef OPT_TARGET_H
#define OPT_TARGET_H

#include<GLAD\glad.h>
#include<GLFW\glfw3.h>
#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<glm\gtc\type_ptr.hpp>
#include<glm\gtc\constants.hpp>

#include<iostream>
#include<string>
#include<vector>
#include "shader.h"
#include "model.h"
#include "rayUtil.h"
#include "triangleOctree.h"

struct VertInfo
{
	//ray distance from the other object
	float hitDistance = 0.0f;
	//is the vertex calculated?
	bool isInitialized = false;
	//is the current vertex colliding?
	bool isColliding = false;
	//the force multiplier
	float hitIntensity = 0.0f;
};

class OctreeTarget
{
public:
	OctreeTarget(const char* modelPath, float falloff, float roughness, float threshold, Octree& tree) :
		targetModel(modelPath, true), falloff(falloff), roughness(roughness), threshold(threshold)
	{
		VertInfo vi;
		std::vector<VertInfo> vInfo(targetModel.meshes[0].vertices.size(), vi);
		vertInfo = vInfo;

		model = glm::mat4(1.0f);
		std::cout << "Loaded model info, setting up vertices...\n";
		OptimizeVertices();

		std::vector<Triangle> modelTris;
		for (int i = 0; i < targetModel.meshes[0].indices.size(); i += 3) //for each triangle, add to octree
		{
			modelTris.push_back(Triangle(targetModel.meshes[0].indices[i], targetModel.meshes[0].indices[i+1], targetModel.meshes[0].indices[i+2]));

		}

		tree.InsertTriangles(modelTris);

		std::cout << "Successfully set up target\n";
	}

	void Draw(Shader& shader)
	{
		//this convention is assumed
		shader.setMat4("model", model);

		shader.setVec3("material.diffuse", targetModel.material.diffuse);
		shader.setVec3("material.specular", targetModel.material.specular);
		shader.setFloat("material.shininess", 32.0f);

		targetModel.Draw(shader);
	}

	//Calculates ray falloff given the material parameters, returns intensity in % of original force, or direct 0 if greater than falloff
	float falloffFunc(float input)
	{
		//std::cout << "registered falloff with: " << input << " ";
		if (input >= falloff) //input needs to be from 0 to falloff, otherwise we go negative
			return 0;		  //since the input is going to be vector distance, it's always going to be >=0

		float res = powf((1 - (input / falloff) * (input / falloff)), roughness);
		return res;
	}

	Model targetModel;
	glm::mat4 model;
	std::vector<glm::vec3> optimizedVerts;
	std::vector<VertInfo> vertInfo;
	float falloff;
private:
	void OptimizeVertices()
	{
		for (int i = 0; i < targetModel.meshes[0].vertices.size(); i++)
		{
			bool found = false;
			for (int j = 0; j < optimizedVerts.size(); j++)
			{
				if (targetModel.meshes[0].vertices[i].Position == optimizedVerts[j])
					found = true;
			}
			if (!found)
				optimizedVerts.push_back(targetModel.meshes[0].vertices[i].Position);
			if (!(i % 250))
			{
				system("CLS");
				std::cout << "optimizing: " << float(i) * 100.0f / targetModel.meshes[0].vertices.size() << "%\n";

			}
		}
	}
	float roughness;
	float threshold;
};

#endif