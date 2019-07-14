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

/*
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
};*/

class OctreeTarget
{
public:
	OctreeTarget(const char* modelPath, float falloff, float roughness, float threshold) :
		targetModel(modelPath, true), falloff(falloff), roughness(roughness), threshold(threshold)
	{
		VertInfo vi;
		std::vector<VertInfo> vInfo(targetModel.meshes[0].vertices.size(), vi);
		vertInfo = vInfo;

		model = glm::mat4(1.0f);
		std::cout << "Loaded model info, setting up vertices...\n";
		OptimizeVertices();

		std::cout << "Successfully set up target\n";
		float minX, minY, minZ, maxX, maxY, maxZ;
		minX = targetModel.meshes[0].vertices[0].Position.x; maxX = minX;
		minY = targetModel.meshes[0].vertices[0].Position.y; maxY = minY;
		minZ = targetModel.meshes[0].vertices[0].Position.z; maxZ = minZ;
		for (int i = 1; i < targetModel.meshes[0].vertices.size(); i++)
		{
			if (targetModel.meshes[0].vertices[i].Position.x <= minX)
				minX = targetModel.meshes[0].vertices[i].Position.x;
			if (targetModel.meshes[0].vertices[i].Position.x >= maxX)
				maxX = targetModel.meshes[0].vertices[i].Position.x;
			
			if (targetModel.meshes[0].vertices[i].Position.y <= minY)
				minY = targetModel.meshes[0].vertices[i].Position.y;
			if (targetModel.meshes[0].vertices[i].Position.y >= maxY)
				maxY = targetModel.meshes[0].vertices[i].Position.y;
			
			if (targetModel.meshes[0].vertices[i].Position.z <= minZ)
				minZ = targetModel.meshes[0].vertices[i].Position.z;
			if (targetModel.meshes[0].vertices[i].Position.z >= maxZ)
				maxZ = targetModel.meshes[0].vertices[i].Position.z;

		}
		std::cout << "min/max X: " << minX << " " << maxX << "\nmin/max Y: " << minY << " " << maxY <<
			"\nmin/max Z: " << minZ << " " << maxZ << "\n";
		boundingBoxSize = fmaxf(fmaxf(maxX - minX, maxY - minY), maxZ - minZ);
		boundingBoxCenter = glm::vec3((maxX + minX) / 2, (maxY + minY) / 2, (maxZ + minZ) / 2);
		std::cout << "bounding box size: " << boundingBoxSize << "\n";
	}

	void SetupTree(Octree& tree)
	{
		std::vector<Triangle> modelTris;
		for (int i = 0; i < targetModel.meshes[0].indices.size(); i += 3) //for each triangle, add to octree
		{
			modelTris.push_back(Triangle(targetModel.meshes[0].indices[i], targetModel.meshes[0].indices[i + 1], targetModel.meshes[0].indices[i + 2]));

		}

		tree.InsertTriangles(modelTris);
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
	float boundingBoxSize;
	glm::vec3 boundingBoxCenter;
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