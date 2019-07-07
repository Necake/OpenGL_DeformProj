#ifndef PROJECTILE_H
#define PROJECTILE_H
//-------------------------------------------------------------------------------------
// Projectile class, contains 2 classes: Projectile (with a mesh), and PointProjectile.
// Both classes are ray casters, PointProjectile casts a single ray, Projectile casts
// a ray per vertex
//-------------------------------------------------------------------------------------

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
		speed = acceleration;
		std::cout << "Successfuly constructed point projectile\n";
	}

	bool CastRay(Model &target, int indexv0, int indexv1, int indexv2, glm::mat4 model)
	{
		std::cout << "Ray cast at: " << indexv0 << " " << indexv1 << " " << indexv2 << "\n";
		glm::vec3 vert0 = (model * glm::vec4(target.meshes[0].vertices[target.meshes[0].indices[indexv0]].Position, 1.0f));
		glm::vec3 vert1 = (model * glm::vec4(target.meshes[0].vertices[target.meshes[0].indices[indexv1]].Position, 1.0f));
		glm::vec3 vert2 = (model * glm::vec4(target.meshes[0].vertices[target.meshes[0].indices[indexv2]].Position, 1.0f));
		bool rayResult = RayUtil::MTRayCheck(vert0, vert1, vert2, projectilePosition, glm::normalize(rayDirection));
		if (rayResult)
		{
			acceleration = -rayDirection;
			std::cout << "ray hit at " << indexv0 << " " << indexv1 << " " << indexv2 << 
				"\n accel: " << acceleration.x << acceleration.y << acceleration.z <<
				"\n speed: " << speed.x << speed.y << speed.z << "\n";
			return true;
		}
		return false;
	}

	void processTarget(Model& target, glm::mat4 model)
	{
		for (int i = 0; i < target.meshes[0].indices.size(); i += 3)
		{
			bool rayResult = CastRay(target, i, i + 1, i + 2, model);
			if (rayResult)
			{
				std::cout << "pushed " << i << " " << i + 1 << " " << i + 2 << " indices\n";
				affectedIndices.push_back(i);
				affectedIndices.push_back(i + 1);
				affectedIndices.push_back(i + 2);
			}
		}
	}

	void dentTarget(Model& target, float time, glm::mat4 model) //for now, this dents the appropriate tri, but the wrong way lol
	{
		for (int i = 0; i < affectedIndices.size(); i += 3)
		{
			target.TranslateVertex(0, target.meshes[0].indices[affectedIndices[i]], speed);
			target.TranslateVertex(0, target.meshes[0].indices[affectedIndices[i+1]], speed);
			target.TranslateVertex(0, target.meshes[0].indices[affectedIndices[i+2]], speed);
			target.meshes[0].UpdateBuffer(affectedIndices[i]);
		}
		projectilePosition += speed;

		if (glm::dot(speed, rayDirection) < __EPSILON)
		{
			speed = glm::vec3(0, 0, 0);
		}
		else
			speed += acceleration * time;
	}

	//Renders a ray that has length of acceleration
	void RenderRay(glm::mat4 view, glm::mat4 model, glm::mat4 projection)
	{
		RayUtil::renderRay(projectilePosition, rayDirection, view, model, projection, rayShader);
	}
	//Renders a ray with infinite length
	void RenderInfiniteRay(glm::mat4 view, glm::mat4 model, glm::mat4 projection)
	{
		RayUtil::renderRay(projectilePosition, rayDirection * 1000000.0f, view, model, projection, rayShader);
	}
	void Update(float time)
	{
		//lol nothing for now	
	}
	glm::vec3 projectilePosition;
	glm::vec3 acceleration;
	glm::vec3 rayDirection;
	glm::vec3 speed; //TODO: make private again
private:
	bool firstHit = true;
	vector<int> affectedIndices;


	Shader rayShader;
};




#endif