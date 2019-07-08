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
#include<utility>
#include "target.h"
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
			bool intersect = RayUtil::MTRayCheck(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), vertex, glm::normalize(acceleration), hitDistance);
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
	float hitDistance;
	std::vector<glm::vec3> optimizedVerts;
	Shader rayShader;
};

//Projectile that has only one ray
class PointProjectile
{
public:
	//Constructor, takes the position/origin of ray and the acceleration
	PointProjectile(glm::vec3 pos, glm::vec3 acc):
		projectilePosition(pos), acceleration(acc), 
		rayShader("../OpenGL_DeformProj/ray.vert", "../OpenGL_DeformProj/ray.frag"), rayDirection(acceleration)
	{
		speed = acceleration;
		std::cout << "Successfuly constructed point projectile\n";
	}

	//Casts a single ray on a given triangle of a target (transformed using a model matrix)
	bool CastRay(Target &target, int indexv0, int indexv1, int indexv2, glm::mat4 model)
	{
		std::cout << "Ray cast at: " << indexv0 << " " << indexv1 << " " << indexv2 << "\n";
		glm::vec3 vert0 = (model * glm::vec4(target.targetModel.meshes[0].vertices[target.targetModel.meshes[0].indices[indexv0]].Position, 1.0f));
		glm::vec3 vert1 = (model * glm::vec4(target.targetModel.meshes[0].vertices[target.targetModel.meshes[0].indices[indexv1]].Position, 1.0f));
		glm::vec3 vert2 = (model * glm::vec4(target.targetModel.meshes[0].vertices[target.targetModel.meshes[0].indices[indexv2]].Position, 1.0f));
		bool rayResult = RayUtil::MTRayCheck(vert0, vert1, vert2, projectilePosition, glm::normalize(rayDirection), hitDistance);
		if (rayResult)
		{
			acceleration = -rayDirection; //reverse acceleration direction on hit (start slowing down)
			std::cout << "ray hit at " << indexv0 << " " << indexv1 << " " << indexv2 << 
				"\n accel: " << acceleration.x << acceleration.y << acceleration.z <<
				"\n speed: " << speed.x << speed.y << speed.z << "\n";
			return true;
		}
		return false;
	}

	//Mesh preprocessing, detects all intersections, bruteforce
	void processTarget(Target& target, glm::mat4 model)
	{
		//For each triangle in the mesh, do tuff
		for (int i = 0; i < target.targetModel.meshes[0].indices.size(); i += 3)
		{
			//Cast rays on each triangle
			bool rayResult = CastRay(target, i, i + 1, i + 2, model);
			if (rayResult) //If we get a collision, push the vertices into those that need to be deformed
			{
				hitPoint = projectilePosition + hitDistance * rayDirection;
				std::cout << "hitpoint: x: " << hitPoint.x << " y: " << hitPoint.y << " z: " << hitPoint.z << "\n";
				std::cout << "pushed " << i << " " << i + 1 << " " << i + 2 << " indices\n";
				for (int j = i; j < i + 3; j++)
				{
					
				}
			}
		}
		for (int i = 0; i < target.targetModel.meshes[0].vertices.size(); i++)
		{
			std::pair<int, float> newVert;
			newVert.first = i;
			glm::vec3 vect = (hitPoint - glm::vec3(model * glm::vec4(target.targetModel.meshes[0].vertices[i].Position, 1.0f)));
			newVert.second = target.falloffFunc(glm::length(vect));
			std::cout << "vecLength: " << newVert.second << " x: " << vect.x << " y: " << vect.y << " z: " << vect.z << "\n";
			affectedVertices.push_back(newVert);
		}
	}

	//Dents a single triangle on a given target, and slows down the projectile appropriately
	void dentTarget(Target& target, float time, glm::mat4 model)
	{
		//For each of the affected triangles, get the indices and translade according verts
		for (int i = 0; i < affectedVertices.size(); i++)
		{
			target.targetModel.TranslateVertex(0, affectedVertices[i].first, speed * affectedVertices[i].second);
			//Update the deformed vertices in the vertex buffer
			target.targetModel.meshes[0].UpdateBufferVertexDirect(affectedVertices[i].first);
		}
		/*for (int i = 0; i < indirectIndices.size(); i++)
		{
			target.TranslateVertex(0, target.meshes[0].indices[indirectIndices[i]], speed * 0.5f);
			target.meshes[0].UpdateBufferVertex(indirectIndices[i]);
		}*/
		projectilePosition += speed; //Change projectile position according to current speed

		//If the speed beomes the opposite direction of the ray, we hammer it at zero,
		//because we don't want backwards movement
		if (glm::dot(speed, rayDirection) < __EPSILON)
		{
			speed = glm::vec3(0, 0, 0);
		}
		else
		{ //else, we update the speed appropriately
			speed += acceleration * time;
		}
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
	
	glm::vec3 projectilePosition; //Position of projectile, also ray origin
	glm::vec3 acceleration; //Acceleration of body
	glm::vec3 rayDirection; //Direction of the actual ray
	
private:
	float hitDistance;
	glm::vec3 speed; //Current speed of projectile
	glm::vec3 hitPoint;
	//Indices of verts affected by rays, along with the % of force acting upon them
	std::vector<std::pair<int,float>> affectedVertices; 
	Shader rayShader; //Shader of the ray itself
};




#endif