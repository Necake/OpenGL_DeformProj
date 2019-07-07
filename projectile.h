#ifndef PROJECTILE_H
#define PROJECTILE_H

//ovde ce da stoji model i par vektora
#include<GLAD\glad.h>
#include<GLFW\glfw3.h>
#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<glm\gtc\type_ptr.hpp>
#include<glm\gtc\constants.hpp>

#include<iostream>
#include<string>
#include<fstream>
#include<set>
#include "shader.h"
#include "model.h"
#include "rayUtil.h"

class Projectile
{
public:
	Projectile(std::string meshPath, glm::vec3 accel) :
		projectileMesh(meshPath.c_str(), false), acceleration(accel), 
		rayShader("../OpenGL_DeformProj/ray.vert", "../OpenGL_DeformProj/ray.frag")
	{
		std::cout << "Successfully constructed projectile\n";
		std::ofstream vertOutputFile;
		vertOutputFile.open("vertOutput.txt");
		optimizeVertices();
		std::cout << "optimized verts size: " << optimizedVerts.size();
		vertOutputFile.close();
		//std::cout << projectileMesh.meshes[0].vertices.size();//.vertices.size();
	}

	void update()
	{
		speed += acceleration;
	}

	void castRays(glm::mat4 view, glm::mat4 model, glm::mat4 projection)
	{
		
		for (auto vertex : optimizedVerts)
		{
			//for every single vertex in the mesh, render a ray
			//renderRay(vertex, glm::normalize(acceleration), view, model, projection, rayShader);
			bool intersect = RayUtil::MTRayCheck(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), vertex, glm::normalize(acceleration));
		}
	}

	void draw(Shader shader, glm::mat4 view, glm::mat4 model, glm::mat4 projection, glm::vec3 viewPos)
	{
		shader.use();

		shader.setVec3("material.diffuse", projectileMesh.material.diffuse);
		shader.setVec3("material.specular", projectileMesh.material.specular);
		shader.setFloat("material.shininess", 32.0f);

		projectileMesh.Draw(shader);
	}

	Model projectileMesh;
	glm::vec3 acceleration;
private:
	void optimizeVertices()
	{
		for (int i = 0; i < projectileMesh.meshes[0].vertices.size(); i++)
		{
			bool found = false;
			for (int j = 0; j < optimizedVerts.size(); j++)
			{
				if (projectileMesh.meshes[0].vertices[i].Position == optimizedVerts[j])
					found = true;
			}
			if(!found)
				optimizedVerts.push_back(projectileMesh.meshes[0].vertices[i].Position);
		}
	}
	glm::vec3 speed;
	std::vector<glm::vec3> optimizedVerts;
	Shader rayShader;
};


class PointProjectile
{
public:
	PointProjectile(glm::vec3 pos, glm::vec3 acc):
		projectilePosition(pos), acceleration(acc), 
		rayShader("../OpenGL_DeformProj/ray.vert", "../OpenGL_DeformProj/ray.frag"), rayDirection(acceleration)
	{
		speed = glm::vec3(0, 0, 0);
		std::cout << "Successfuly constructed point projectile\n";
	}

	void CastRay(Model target, int indexv0, int indexv1, int indexv2, glm::mat4 model, float time)
	{
		glm::vec3 vert0 = (model * glm::vec4(target.meshes[0].vertices[target.meshes[0].indices[indexv0]].Position, 1.0f));
		glm::vec3 vert1 = (model * glm::vec4(target.meshes[0].vertices[target.meshes[0].indices[indexv1]].Position, 1.0f));
		glm::vec3 vert2 = (model * glm::vec4(target.meshes[0].vertices[target.meshes[0].indices[indexv2]].Position, 1.0f));
		bool rayResult = RayUtil::MTRayCheck(vert0, vert1, vert2, projectilePosition, glm::normalize(acceleration));
		if (rayResult)
		{
			if (firstHit)
			{
				acceleration = glm::vec3(0, 0, 0);
				firstHit = false;
			}
			target.TranslateVertex(0, target.meshes[0].indices[indexv0], speed);
			target.TranslateVertex(0, target.meshes[0].indices[indexv1], speed);
			target.TranslateVertex(0, target.meshes[0].indices[indexv2], speed);
			Update(time);
		}
	}
	void RenderRay(glm::mat4 view, glm::mat4 model, glm::mat4 projection)
	{
		RayUtil::renderRay(projectilePosition, acceleration, view, model, projection, rayShader);
	}
	void Update(float time)
	{
		speed += acceleration * time;
	}
	glm::vec3 projectilePosition;
	glm::vec3 acceleration;
	glm::vec3 rayDirection;
private:
	bool firstHit = true;
	glm::vec3 speed;
	Shader rayShader;
};




#endif