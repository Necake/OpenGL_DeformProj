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
		projectileMesh(meshPath.c_str(), true), acceleration(accel), 
		rayShader("../OpenGL_DeformProj/ray.vert", "../OpenGL_DeformProj/ray.frag")
	{
		rayDirection = acceleration;
		speed = acceleration;
		std::cout << "Successfully constructed projectile ";
		OptimizeVertices();
		std::cout << "optimized verts size: " << optimizedVerts.size() << "\n";

		//std::cout << projectileMesh.meshes[0].vertices.size();//.vertices.size();
	}

	//Casts a single ray on a given triangle of a target, given the ray origin (transformed using a model matrix)
	bool CastRay(Target& target, int indexv0, int indexv1, int indexv2, glm::vec3 rayOrigin, glm::mat4 model, float& hitDistance)
	{
		glm::vec3 vert0 = (model * glm::vec4(target.targetModel.meshes[0].vertices[target.targetModel.meshes[0].indices[indexv0]].Position, 1.0f));
		glm::vec3 vert1 = (model * glm::vec4(target.targetModel.meshes[0].vertices[target.targetModel.meshes[0].indices[indexv1]].Position, 1.0f));
		glm::vec3 vert2 = (model * glm::vec4(target.targetModel.meshes[0].vertices[target.targetModel.meshes[0].indices[indexv2]].Position, 1.0f));
		return RayUtil::MTRayCheck(vert0, vert1, vert2, this->model * glm::vec4(rayOrigin, 1.0f), glm::normalize(rayDirection), hitDistance);
	}

	bool CastInverseRay(int indexv0, int indexv1, int indexv2, glm::vec3 rayOrigin, glm::mat4 model, float& hitDistance)
	{
		glm::vec3 vert0 = (this->model * glm::vec4(projectileMesh.meshes[0].vertices[projectileMesh.meshes[0].indices[indexv0]].Position, 1.0f));
		glm::vec3 vert1 = (this->model * glm::vec4(projectileMesh.meshes[0].vertices[projectileMesh.meshes[0].indices[indexv1]].Position, 1.0f));
		glm::vec3 vert2 = (this->model * glm::vec4(projectileMesh.meshes[0].vertices[projectileMesh.meshes[0].indices[indexv2]].Position, 1.0f));
		return RayUtil::MTRayCheck(vert0, vert1, vert2, model * glm::vec4(rayOrigin, 1.0f), glm::normalize(-rayDirection), hitDistance);
	}

	void ProcessRays(Target& target, glm::mat4 model)
	{
		//cast rays from projectile onto target
		for (auto vertexPos : optimizedVerts)
		{
			//for every single vertex in the mesh, cast a ray on every triangle of the target
			for (int i = 0; i < target.targetModel.meshes[0].indices.size(); i += 3)
			{
				float hitDistance = 0.0f;
				bool rayResult = CastRay(target, i, i + 1, i + 2, vertexPos, model, hitDistance);
				if (rayResult)
				{
					collision = true;
					if (!target.vertInfo[target.targetModel.meshes[0].indices[i]].isInitialized)
					{
						target.vertInfo[target.targetModel.meshes[0].indices[i]].isInitialized = true;
						target.vertInfo[target.targetModel.meshes[0].indices[i]].hitDistance = hitDistance;
						target.vertInfo[target.targetModel.meshes[0].indices[i]].hitIntensity = 1.0f;
					}
				}
			}
		}
		//cast rays from target onto projectile (inverse)
		for (int i = 0; i < target.targetModel.meshes[0].vertices.size(); i++)
		{
			for (int j = 0; j < projectileMesh.meshes[0].indices.size(); j += 3)
			{
				float hitDistance = 0.0f;
				bool rayResult = CastInverseRay(j, j + 1, j + 2, target.targetModel.meshes[0].vertices[i].Position, model, hitDistance);
				if (rayResult)
				{
					//std::cout << "inverse ray hit!\t"; todo delete
					if (!target.vertInfo[i].isInitialized)
					{
						target.vertInfo[i].isInitialized = true;
						target.vertInfo[i].hitDistance = hitDistance;
						target.vertInfo[i].hitIntensity = 1.0f;
						collision = true;
					}
					//std::cout << hitDistance << "\n"; todo delete
				}
			}
		}
	}

	//Mesh preprocessing, detects all intersections, bruteforce
	void ProcessTarget(Target& target, glm::mat4 model)
	{
		if (collision) //If (at least one) ray has intersected with the target
		{
			for (int i = 0; i < target.targetModel.meshes[0].vertices.size(); i++)
			{
				if (!target.vertInfo[i].isInitialized)
				{
					float maxIntensity = 0.0f;
					for (int j = 0; j < target.targetModel.meshes[0].vertices.size(); j++)
					{
						float dist = glm::length(target.targetModel.meshes[0].vertices[i].Position - target.targetModel.meshes[0].vertices[j].Position);
						float currIntensity = target.falloffFunc(dist);
						if ((dist < target.falloff) && (target.vertInfo[j].isInitialized) && currIntensity > maxIntensity)
						{
							maxIntensity = currIntensity;
							target.vertInfo[i].isColliding = true;
						}
					}

					target.vertInfo[i].hitIntensity = maxIntensity;
				}
			}
		}
	}

	void Draw(Shader shader)
	{
		shader.use();

		shader.setVec3("material.diffuse", projectileMesh.material.diffuse);
		shader.setVec3("material.specular", projectileMesh.material.specular);
		shader.setFloat("material.shininess", 32.0f);

		projectileMesh.Draw(shader);
	}

	void DentVertexDirect(Target& target, int index, glm::mat4 model)
	{
		target.targetModel.TranslateVertex(0, index, speed * target.vertInfo[index].hitIntensity);
		//Update the deformed vertices in the vertex buffer
		target.targetModel.meshes[0].UpdateBufferVertexDirect(index);
	}

	void Update(Target& target, float time, glm::mat4 model)
	{
		if (collision)
		{
			for (int i = 0; i < optimizedVerts.size(); i++)
			{
				optimizedVerts[i] += speed; //Change position of all ray origins according to current speed
			}
			for (int i = 0; i < projectileMesh.meshes[0].vertices.size(); i++)
			{
				projectileMesh.meshes[0].vertices[i].Position += speed; //Change position of all vertices according to current speed
				projectileMesh.meshes[0].UpdateBufferVertexDirect(i);
			}
			
			//Update distances to impact on vertices
			for (int i = 0; i < target.targetModel.meshes[0].vertices.size(); i++)
			{
				if (target.vertInfo[i].isInitialized)
				{
					target.vertInfo[i].hitDistance -= glm::length(speed);
					if (target.vertInfo[i].hitDistance < __EPSILON)
					{
						//the given vertex is colliding
						target.vertInfo[i].isColliding = true;
						acceleration = -rayDirection; //reverse acceleration direction on hit (start slowing down)

						// calculate the falloff around the collided vertex
						for (int j = 0; j < target.targetModel.meshes[0].vertices.size(); j++)
						{
							float falloff = target.falloffFunc(glm::length(target.targetModel.meshes[0].vertices[i].Position - target.targetModel.meshes[0].vertices[j].Position));
							if (falloff > target.vertInfo[j].hitIntensity)
							{
								target.vertInfo[j].hitIntensity = falloff;
								target.vertInfo[j].isColliding = true;
							}
						}
					}
				}
			}

			for (int i = 0; i < target.targetModel.meshes[0].vertices.size(); i++)
			{
				if(target.vertInfo[i].isColliding)
					DentVertexDirect(target, i, model);
			}

			//If the speed beomes the opposite direction of the ray, we hammer it at zero,
			//because we don't want backwards movement
			if (glm::dot(speed, rayDirection) < __EPSILON)
			{
				speed = glm::vec3(0, 0, 0);
				isDone = true;
			}
			else
			{ //else, we update the speed appropriately
				speed += acceleration * time;
			}

		}
	}

	//Renders a ray that has length of acceleration
	void RenderRays(glm::mat4 view, glm::mat4 projection)
	{
		for(int i = 0; i < optimizedVerts.size(); i++)
			RayUtil::renderRay(optimizedVerts[i], rayDirection, view, model, projection, rayShader);
	}

	//Renders a ray with infinite length
	void RenderInfiniteRays(glm::mat4 view, glm::mat4 projection)
	{
		for (int i = 0; i < optimizedVerts.size(); i++)
			RayUtil::renderRay(optimizedVerts[i], rayDirection * 1000000.0f, view, model, projection, rayShader);
	}

	Model projectileMesh;
	glm::mat4 model; //the projectile's model matrix
	glm::vec3 acceleration;
	glm::vec3 rayDirection;
	bool isDone = false; //is the sim over?
private:
	void OptimizeVertices()
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
	bool collision = false; //is there going to be a collision? (has any ray hit the target?
	bool isColliding = false; //is it colliding right now?
	bool hasProcessed = false; //has the model been processed
	glm::vec3 speed;
	float minHitDistance = FLT_MAX; //equivalent to the min distance of vertex to the body
	glm::vec3 nearestVert; //the position of the nearest vertex
	glm::vec3 nearestOrigin; //the position of the origin targeting the nearest vert
	std::vector<glm::vec3> optimizedVerts;
	std::vector<std::pair<int, float>> affectedVertices;
	std::vector<std::pair<glm::vec3, float>> hitPoints; //keeps track of hitpoints and their distances from the projectile
	Shader rayShader;
};




//----------------------------------------------------------------------------------------
//Projectile that has only one ray
//----------------------------------------------------------------------------------------


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
		//std::cout << "Ray cast at: " << indexv0 << " " << indexv1 << " " << indexv2 << "\n";
		glm::vec3 vert0 = (model * glm::vec4(target.targetModel.meshes[0].vertices[target.targetModel.meshes[0].indices[indexv0]].Position, 1.0f));
		glm::vec3 vert1 = (model * glm::vec4(target.targetModel.meshes[0].vertices[target.targetModel.meshes[0].indices[indexv1]].Position, 1.0f));
		glm::vec3 vert2 = (model * glm::vec4(target.targetModel.meshes[0].vertices[target.targetModel.meshes[0].indices[indexv2]].Position, 1.0f));
		bool rayResult = RayUtil::MTRayCheck(vert0, vert1, vert2, projectilePosition, glm::normalize(rayDirection), hitDistance);
		if (rayResult)
		{
			std::cout << "ray hit at " << indexv0 << " " << indexv1 << " " << indexv2 << 
				"\n accel: " << acceleration.x << acceleration.y << acceleration.z <<
				"\n speed: " << speed.x << speed.y << speed.z << "\n";
			return true;
		}
		return false;
	}

	void ProcessRay(Target& target, glm::mat4 model)
	{
		//For each triangle in the mesh, do stuff
		for (int i = 0; i < target.targetModel.meshes[0].indices.size(); i += 3)
		{
			//Cast rays on each triangle
			bool rayResult = CastRay(target, i, i + 1, i + 2, model);
			if (rayResult) //If we get a collision, push the vertices into those that need to be deformed
			{
				hitPoint = projectilePosition + hitDistance * glm::normalize(rayDirection);
				std::cout << "hit distance: " << hitDistance << "\nhitpoint: x: " << hitPoint.x << " y: " << hitPoint.y << " z: " << hitPoint.z << "\n";
				std::cout << "pushed " << i << " " << i + 1 << " " << i + 2 << " indices\n";
				collision = true;
			}
		}
	}
	//Mesh preprocessing, detects all intersections, bruteforce
	void ProcessTarget(Target& target, glm::mat4 model)
	{

		if (collision) //If (at least one) ray has intersected with the target
		{
			for (int i = 0; i < target.targetModel.meshes[0].vertices.size(); i++) //for each vertex of the target
			{
				std::pair<int, float> newVert;
				newVert.first = i; //take the index of the vertex, and the amount of fore we apply on it 
				glm::vec3 distance = (hitPoint - glm::vec3(model * glm::vec4(target.targetModel.meshes[0].vertices[i].Position, 1.0f)));
				newVert.second = target.falloffFunc(glm::length(distance)); //Calculate the force multiplier
				//std::cout << "vecLength: " << newVert.second << " x: " << vect.x << " y: " << vect.y << " z: " << vect.z << "\n";
				if (newVert.second > __EPSILON) //If the vertex is actually affected by the ray in any way, we push it back
					affectedVertices.push_back(newVert);
			}
		}
	}

	//Dents a single triangle on a given target, and slows down the projectile appropriately
	void DentTarget(Target& target, float time, glm::mat4 model)
	{
		//For each of the affected triangles, get the indices and translade according verts
		for (int i = 0; i < affectedVertices.size(); i++)
		{
			target.targetModel.TranslateVertex(0, affectedVertices[i].first, speed * affectedVertices[i].second);
			//Update the deformed vertices in the vertex buffer
			target.targetModel.meshes[0].UpdateBufferVertexDirect(affectedVertices[i].first);
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
	
	void Update(Target& target, float time, glm::mat4 model)
	{
		if (collision)
		{
			projectilePosition += speed; //Change projectile position according to current speed
			hitDistance = glm::length(projectilePosition - hitPoint);
			if (hitDistance < 0.05f)
			{
				isColliding = true;
				acceleration = -rayDirection; //reverse acceleration direction on hit (start slowing down)
				std::cout << "we hit the mesh\n";
			}
			if (isColliding)
			{
				if (!hasProcessed)
				{
					hasProcessed = true;
					ProcessTarget(target, model);
				}
				else
				{
					DentTarget(target, time, model);
				}
			}

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
	}
	
	glm::vec3 projectilePosition; //Position of projectile, also ray origin
	glm::vec3 acceleration; //Acceleration of body
	glm::vec3 rayDirection; //Direction of the actual ray
	
private:
	bool collision = false;
	bool isColliding = false;
	bool hasProcessed = false;
	float hitDistance;
	glm::vec3 speed; //Current speed of projectile
	glm::vec3 hitPoint;
	//Indices of verts affected by rays, along with the % of force acting upon them
	std::vector<std::pair<int,float>> affectedVertices; 
	Shader rayShader; //Shader of the ray itself
};




#endif