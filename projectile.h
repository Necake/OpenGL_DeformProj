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
#include <fstream>
#include "shader.h"
#include "model.h"
#include "rayUtil.h"

class Projectile
{
public:
	Projectile(std::string meshPath, glm::vec3 accel) :
		projectileMesh(meshPath.c_str()), acceleration(accel), 
		rayShader("../OpenGL_DeformProj/ray.vert", "../OpenGL_DeformProj/ray.frag")
	{
		std::cout << "Successfully constructed projectile\n";
		std::ofstream vertOutputFile;
		vertOutputFile.open("vertOutput.txt");
		for (int i = 0; i < projectileMesh.meshes[0].vertices.size(); i++)
		{
			vertOutputFile << "v  " << projectileMesh.meshes[0].vertices[i].Position.x << " " <<
				projectileMesh.meshes[0].vertices[i].Position.y << " " << projectileMesh.meshes[0].vertices[i].Position.z << "\n";
			
		}
		vertOutputFile.close();
		//std::cout << projectileMesh.meshes[0].vertices.size();//.vertices.size();
	}

	void update()
	{
		speed += acceleration;
	}

	void castRays(glm::mat4 view, glm::mat4 model, glm::mat4 projection)
	{
		
		for (int i = 0; i < 163; i++)
		{
			//for every single vertex in the mesh, render a ray
			renderRay(projectileMesh.meshes[0].vertices[projectileMesh.meshes[0].indices[i]].Position,
				glm::normalize(acceleration), view, model, projection, rayShader);
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
	glm::vec3 speed;
	Shader rayShader;
};

/*
class PointProjectile
{
public:
private:
};*/




#endif