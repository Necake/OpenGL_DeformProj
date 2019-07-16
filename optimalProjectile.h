#ifndef OPT_PROJECTILE_H
#define OPT_PROJECTILE_H
//-------------------------------------------------------------------------------------
// Same as projectile class, but utilizes octree (see projectile.h)
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
#include<set>
#include "optimalTarget.h"
#include "shader.h"
#include "model.h"
#include "rayUtil.h"
#include "triangleOctree.h"

class OctreeProjectile
{
public:
	OctreeProjectile(std::string meshPath, glm::vec3 accel) :
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
	bool CastRay(OctreeTarget& target, int indexv0, int indexv1, int indexv2, glm::vec3 rayOrigin, glm::mat4 model, float& hitDistance)
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

	void ProcessRays(OctreeTarget& target, glm::mat4 model)
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
	void ProcessTarget(OctreeTarget& target, glm::mat4 model)
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

	void DentVertexDirect(OctreeTarget& target, int index, glm::mat4 model)
	{
		target.targetModel.TranslateVertex(0, index, speed * target.vertInfo[index].hitIntensity);
		//Update the deformed vertices in the vertex buffer
		target.targetModel.meshes[0].UpdateBufferVertexDirect(index);
	}

	void Update(OctreeTarget& target, float time, glm::mat4 model)
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
				if (target.vertInfo[i].isColliding)
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
		for (int i = 0; i < optimizedVerts.size(); i++)
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
			if (!found)
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


class OctreePointProjectile
{
public:
	//Constructor, takes the position/origin of ray and the acceleration
	OctreePointProjectile(glm::vec3 pos, glm::vec3 acc) :
		projectilePosition(pos), acceleration(acc),
		rayShader("../OpenGL_DeformProj/ray.vert", "../OpenGL_DeformProj/ray.frag"), rayDirection(acceleration)
	{
		speed = acceleration;
		std::cout << "Successfuly constructed point projectile\n";
	}

	//Casts a single ray on a given triangle of a target (transformed using a model matrix)
	bool CastRay(Octree& tree, OctreeTarget& target)
	{
		//preform search in given octant
		OctreeNode* targetOctant = tree.FindOctant(projectilePosition + speed);
		if (targetOctant != nullptr)
		{
			if (targetOctant->tris != nullptr)
			{
				if (targetOctant->tris->size() > 0)
				{
					for (int i = 0; i < targetOctant->tris->size(); i++)
					{
						glm::vec3 vert0 = tree.model.meshes[0].vertices[(*targetOctant->tris)[i].index0].Position;
						glm::vec3 vert1 = tree.model.meshes[0].vertices[(*targetOctant->tris)[i].index1].Position;
						glm::vec3 vert2 = tree.model.meshes[0].vertices[(*targetOctant->tris)[i].index2].Position;
						bool rayResult = RayUtil::MTRayCheck(vert0, vert1, vert2, projectilePosition, glm::normalize(rayDirection), hitDistance);
						if (rayResult && (hitDistance < glm::length(speed))) // there's gonna be a hit next frame
						{
							//odmah ovde dentuj da ne bi radio pretragu bezveze
							acceleration = -rayDirection;
							affectedVerts.insert((*targetOctant->tris)[i].index0);
							affectedVerts.insert((*targetOctant->tris)[i].index1);
							affectedVerts.insert((*targetOctant->tris)[i].index2);

							collision = true;
							hitPoint = projectilePosition + hitDistance * glm::normalize(rayDirection);
							CalcLocalFalloff(tree, target, targetOctant);
							return true;
						}
					}
					return false;
				}
			}
		}
		return false;
		
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
	void DentTarget(Target & target, float time, glm::mat4 model)
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

	void CalcLocalFalloff(Octree& tree, OctreeTarget& target, OctreeNode* targetOctant)
	{
		OctreeNode* falloffCenter = tree.FindFalloffCenterNode(hitPoint, target.falloff);
		//FindAdjacentToData(tree, target, falloffCenter, hitPoint, target.falloff);
		int indexX = tree.calcNodeIndexX(targetOctant);
		int indexY = tree.calcNodeIndexY(targetOctant);
		int indexZ = tree.calcNodeIndexZ(targetOctant);
		//std::cout << indexX << indexY << indexZ << "\n";
		AffectLeafFalloff(indexX, indexY, indexZ, tree, target);
		//AffectFalloff(tree.arrayRepresentation[indexX][indexY][indexZ], target);
		//AffectFalloff(tree.arrayRepresentation[indexX - 1][indexY][indexZ - 1], target);
		//AffectFalloff(tree.arrayRepresentation[indexX - 1][indexY][indexZ], target);
		//AffectFalloff(tree.arrayRepresentation[indexX][indexY][indexZ - 1], target);
	}

	void AffectFalloffRecursive(OctreeNode* node, OctreeTarget& target)
	{
		//affect all subnodes
		if (node->XpYpZp == nullptr)
		{
			//it's a leaf
			if (node->tris != nullptr)
			{
				for (int i = 0; i < node->tris->size(); i++)
				{
					target.vertInfo[(*node->tris)[i].index0].hitIntensity =
						target.falloffFunc(glm::length(target.targetModel.meshes[0].vertices[(*node->tris)[i].index0].Position - hitPoint));
					if (target.vertInfo[(*node->tris)[i].index0].hitIntensity > 0.0f)
						affectedVerts.insert((*node->tris)[i].index0);

					target.vertInfo[(*node->tris)[i].index1].hitIntensity =
						target.falloffFunc(glm::length(target.targetModel.meshes[0].vertices[(*node->tris)[i].index1].Position - hitPoint));
					if (target.vertInfo[(*node->tris)[i].index1].hitIntensity > 0.0f)
						affectedVerts.insert((*node->tris)[i].index1);

					target.vertInfo[(*node->tris)[i].index2].hitIntensity =
						target.falloffFunc(glm::length(target.targetModel.meshes[0].vertices[(*node->tris)[i].index2].Position - hitPoint));
					if (target.vertInfo[(*node->tris)[i].index2].hitIntensity > 0.0f)
						affectedVerts.insert((*node->tris)[i].index2);
				}
			}
		}
		else
		{
			AffectFalloffRecursive(node->XpYpZp, target);
			AffectFalloffRecursive(node->XpYpZn, target);
			AffectFalloffRecursive(node->XpYnZp, target);
			AffectFalloffRecursive(node->XpYnZn, target);
			AffectFalloffRecursive(node->XnYpZp, target);
			AffectFalloffRecursive(node->XnYpZn, target);
			AffectFalloffRecursive(node->XnYnZp, target);
			AffectFalloffRecursive(node->XnYnZn, target);
		}
	}

	void AffectFalloff(OctreeNode* node, OctreeTarget& target)
	{
		//if there are triangles
		if (node->tris != nullptr && node->tris->size() > 0)
		{
			for (int i = 0; i < node->tris->size(); i++)
			{
				target.vertInfo[(*node->tris)[i].index0].hitIntensity =
					target.falloffFunc(glm::length(target.targetModel.meshes[0].vertices[(*node->tris)[i].index0].Position - hitPoint));
				if (target.vertInfo[(*node->tris)[i].index0].hitIntensity > 0.0f)
					affectedVerts.insert((*node->tris)[i].index0);

				target.vertInfo[(*node->tris)[i].index1].hitIntensity =
					target.falloffFunc(glm::length(target.targetModel.meshes[0].vertices[(*node->tris)[i].index1].Position - hitPoint));
				if (target.vertInfo[(*node->tris)[i].index1].hitIntensity > 0.0f)
					affectedVerts.insert((*node->tris)[i].index1);

				target.vertInfo[(*node->tris)[i].index2].hitIntensity =
					target.falloffFunc(glm::length(target.targetModel.meshes[0].vertices[(*node->tris)[i].index2].Position - hitPoint));
				if (target.vertInfo[(*node->tris)[i].index2].hitIntensity > 0.0f)
					affectedVerts.insert((*node->tris)[i].index2);
			}
		}
	}

	void AffectLeafFalloff(int indexX, int indexY, int indexZ, Octree& tree, OctreeTarget& target)
	{
		//in a leaf
		float leafSize = tree.size / 8; //todo change to dynamic
		int radiusXP = ceil((tree.arrayRepresentation[indexX][indexY][indexZ]->position.x - hitPoint.x + target.falloff) / leafSize); //= target.falloff / (tree.size / 8);
		int radiusXN = ceil(fabs((tree.arrayRepresentation[indexX][indexY][indexZ]->position.x - hitPoint.x - target.falloff) / leafSize));
		int radiusYP = ceil((tree.arrayRepresentation[indexX][indexY][indexZ]->position.y - hitPoint.y + target.falloff) / leafSize); //= target.falloff / (tree.size / 8);
		int radiusYN = ceil(fabs((tree.arrayRepresentation[indexX][indexY][indexZ]->position.y - hitPoint.y - target.falloff) / leafSize));
		int radiusZP = ceil((tree.arrayRepresentation[indexX][indexY][indexZ]->position.z - hitPoint.z + target.falloff) / leafSize); //= target.falloff / (tree.size / 8);
		int radiusZN = ceil(fabs((tree.arrayRepresentation[indexX][indexY][indexZ]->position.z - hitPoint.z - target.falloff) / leafSize));
		radiusXP = min(radiusXP, 7 - indexX); radiusYP = min(radiusYP, 7 - indexY); radiusZP = min(radiusZP, 7 - indexZ);
		radiusXN = min(radiusXN, indexX); radiusYN = min(radiusYN, indexY); radiusZN = min(radiusXN, indexZ);
		//std::cout << radiusXP << radiusXN << radiusYP << radiusYN << radiusZP << radiusZN << "\n";

		for (int i = -radiusXN; i <= radiusXP; i++)
		{
			for (int j = -radiusYN; j <= radiusYP; j++)
			{
				for (int k = -radiusZN; k <= radiusZP; k++)
				{
					AffectFalloff(tree.arrayRepresentation[indexX + i][indexY + j][indexZ + k], target);
				}
			}
		}
	}

	void FindAdjacentToData(Octree& tree, OctreeTarget& target, OctreeNode* node, glm::vec3 data, float minSize)
	{
		int answer = 0b000000;
		if (data.x + minSize > node->position.x + node->size / 2) //if we get out 
			answer |= 0b000001; //need to check x+
		else if (data.x - minSize < node->position.x - node->size / 2)
			answer |= 0b001000; //need to check x-
		if (data.y + minSize > node->position.y + node->size / 2)
			answer |= 0b000010; //need to check y+
		else if (data.y - minSize < node->position.y - node->size / 2)
			answer |= 0b010000; //need to check y-
		if (data.z + minSize > node->position.z + node->size / 2)
			answer |= 0b000100; //need to check z+
		else if (data.z + minSize > node->position.z + node->size / 2)
			answer |= 0b100000; //need to check z-

		if ((answer & 0b000001) == 0b000001)
		{
			//x+
			glm::vec3 temp = glm::vec3(data.x + minSize, data.y, data.z);
			AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
			if ((answer & 0b000010) == 0b000010)
			{
				//y+
				temp = glm::vec3(data.x, data.y + minSize, data.z);
				AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				temp.x += minSize; //x+y+
				AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				if ((answer & 0b000100) == 0b000100)
				{
					//z+
					temp = glm::vec3(data.x, data.y, data.z + minSize);
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x += minSize;//x+z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp = glm::vec3(data.x, data.y + minSize, data.z + minSize);//y+z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x += minSize; //x+y+z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				}
				else if ((answer & 0b100000) == 0b100000)
				{
					//z-
					temp = glm::vec3(data.x, data.y, data.z - minSize);
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x += minSize;//x+z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp = glm::vec3(data.x, data.y + minSize, data.z - minSize);//y+z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x += minSize; //x+y+z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				}
			}
			else if ((answer & 0b010000) == 0b010000)
			{
				//y-
				glm::vec3 temp = glm::vec3(data.x, data.y - minSize, data.z);
				AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				temp.x += minSize; //x+y-
				AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				if ((answer & 0b000100) == 0b000100)
				{
					//z+
					temp = glm::vec3(data.x, data.y, data.z + minSize);
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x += minSize;//x+z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp = glm::vec3(data.x, data.y - minSize, data.z + minSize);//y-z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x += minSize; //x+y-z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				}
				else if ((answer & 0b100000) == 0b100000)
				{
					//z-
					temp = glm::vec3(data.x, data.y, data.z - minSize);
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x += minSize;//x+z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp = glm::vec3(data.x, data.y - minSize, data.z - minSize);//y-z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x += minSize; //x+y-z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				}
			}
		}
		else if ((answer & 0b001000) == 0b001000)
		{
			//x-
			glm::vec3 temp = glm::vec3(data.x - minSize, data.y, data.z); 
			AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
			if ((answer & 0b000010) == 0b000010)
			{
				//y+
				temp = glm::vec3(data.x, data.y + minSize, data.z);
				AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				temp.x -= minSize; //x-y+
				AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				if ((answer & 0b000100) == 0b000100)
				{
					//z+
					temp = glm::vec3(data.x, data.y, data.z + minSize);
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x -= minSize;//x-z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp = glm::vec3(data.x, data.y + minSize, data.z + minSize);//y+z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x -= minSize; //x-y+z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				}
				else if ((answer & 0b100000) == 0b100000)
				{
					//z-
					temp = glm::vec3(data.x, data.y, data.z - minSize);
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x -= minSize;//x-z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp = glm::vec3(data.x, data.y + minSize, data.z - minSize);//y+z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x -= minSize; //x-y+z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				}
			}
			else if ((answer & 0b010000) == 0b010000)
			{
				//y-
				glm::vec3 temp = glm::vec3(data.x, data.y - minSize, data.z);
				AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				temp.x -= minSize; //x-y-
				AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				if ((answer & 0b000100) == 0b000100)
				{
					//z+
					temp = glm::vec3(data.x, data.y, data.z + minSize);
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x -= minSize;//x-z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp = glm::vec3(data.x, data.y - minSize, data.z + minSize);//y-z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x -= minSize; //x-y-z+
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				}
				else if ((answer & 0b100000) == 0b100000)
				{
					//z-
					temp = glm::vec3(data.x, data.y, data.z - minSize);
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x += minSize;//x-z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp = glm::vec3(data.x, data.y - minSize, data.z - minSize);//y-z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
					temp.x += minSize; //x-y-z-
					AffectFalloff(tree.FindFalloffCenterNode(temp, minSize), target);
				}
			}
		}
	}

	void Update(Octree& tree, OctreeTarget& target, float time, glm::mat4 model)
	{
		if (collision)
		{
			for (auto vert : affectedVerts)
			{
				tree.model.meshes[0].vertices[vert].Position += speed * target.vertInfo[vert].hitIntensity;
				tree.model.meshes[0].UpdateBufferVertexDirect(vert);
			}
		}
		else
		{
			CastRay(tree, target);
		}
		projectilePosition += speed; //Change projectile position according to current speed

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

	glm::vec3 projectilePosition; //Position of projectile, also ray origin
	glm::vec3 acceleration; //Acceleration of body
	glm::vec3 rayDirection; //Direction of the actual ray
	std::set<int> affectedVerts;
	bool isDone = false;
private:
	bool collision = false;
	//bool isColliding = false;
	bool hasProcessed = false;
	float hitDistance;
	glm::vec3 speed; //Current speed of projectile
	glm::vec3 hitPoint;
	//Indices of verts affected by rays, along with the % of force acting upon them
	std::vector<std::pair<int, float>> affectedVertices;
	Shader rayShader; //Shader of the ray itself
};




#endif