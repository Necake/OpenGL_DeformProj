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

		float minX, minY, minZ, maxX, maxY, maxZ;
		minX = projectileMesh.meshes[0].vertices[0].Position.x; maxX = minX;
		minY = projectileMesh.meshes[0].vertices[0].Position.y; maxY = minY;
		minZ = projectileMesh.meshes[0].vertices[0].Position.z; maxZ = minZ;
		for (int i = 1; i < projectileMesh.meshes[0].vertices.size(); i++)
		{
			if (projectileMesh.meshes[0].vertices[i].Position.x <= minX)
				minX = projectileMesh.meshes[0].vertices[i].Position.x;
			if (projectileMesh.meshes[0].vertices[i].Position.x >= maxX)
				maxX = projectileMesh.meshes[0].vertices[i].Position.x;

			if (projectileMesh.meshes[0].vertices[i].Position.y <= minY)
				minY = projectileMesh.meshes[0].vertices[i].Position.y;
			if (projectileMesh.meshes[0].vertices[i].Position.y >= maxY)
				maxY = projectileMesh.meshes[0].vertices[i].Position.y;

			if (projectileMesh.meshes[0].vertices[i].Position.z <= minZ)
				minZ = projectileMesh.meshes[0].vertices[i].Position.z;
			if (projectileMesh.meshes[0].vertices[i].Position.z >= maxZ)
				maxZ = projectileMesh.meshes[0].vertices[i].Position.z;

		}
		std::cout << "min/max X: " << minX << " " << maxX << "\nmin/max Y: " << minY << " " << maxY <<
			"\nmin/max Z: " << minZ << " " << maxZ << "\n";
		boundingBoxSize = fmaxf(fmaxf(maxX - minX, maxY - minY), maxZ - minZ);
		boundingBoxCenter = glm::vec3((maxX + minX) / 2, (maxY + minY) / 2, (maxZ + minZ) / 2);
		std::cout << "bounding box size: " << boundingBoxSize << "\n";
		std::cout << "bounding box center: " << boundingBoxCenter.x << boundingBoxCenter.y << boundingBoxCenter.z << "\n";
		
		boundingBoxCenterOffset = boundingBoxCenter;
		/*
		for (int i = 0; i < projectileMesh.meshes[0].vertices.size(); i++)
		{
			projectileMesh.meshes[0].vertices[i].Position += glm::vec3(0, 3.0f, 0); //Change position of all vertices
			projectileMesh.meshes[0].UpdateBufferVertexDirect(i);
		}*/

		OptimizeVertices();
		std::cout << "optimized verts size: " << optimizedVerts.size() << "\n";

		//std::cout << projectileMesh.meshes[0].vertices.size();//.vertices.size();
	}

	void SetupTree(Octree& tree)
	{
		std::vector<Triangle> modelTris;
		for (int i = 0; i < projectileMesh.meshes[0].indices.size(); i += 3) //for each triangle, add to octree
		{
			modelTris.push_back(Triangle(projectileMesh.meshes[0].indices[i], projectileMesh.meshes[0].indices[i + 1], projectileMesh.meshes[0].indices[i + 2]));

		}

		tree.InsertTriangles(modelTris);
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

	void ProcessRays(Octree& tree, Octree& projectileTree, OctreeTarget& target /*glm::mat4 model*/)
	{
		//cast rays from projectile onto target
		for (auto vertexPos : optimizedVerts)
		{ //search for each ray on the projectile model
			OctreeNode* targetOctant = tree.FindOctant(vertexPos + speed);
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
							float hitDistance = FLT_MAX;
							bool rayResult = RayUtil::MTRayCheck(vert0, vert1, vert2, vertexPos, glm::normalize(rayDirection), hitDistance);
							if (rayResult && (hitDistance < glm::length(speed))) // there's gonna be a hit next frame
							{
								//odmah ovde dentuj da ne bi radio pretragu bezveze
								acceleration = -rayDirection;
								affectedVerts.insert((*targetOctant->tris)[i].index0);
								affectedVerts.insert((*targetOctant->tris)[i].index1);
								affectedVerts.insert((*targetOctant->tris)[i].index2);
								target.vertInfo[(*targetOctant->tris)[i].index0].hitIntensity = 1.0f;
								target.vertInfo[(*targetOctant->tris)[i].index1].hitIntensity = 1.0f;
								target.vertInfo[(*targetOctant->tris)[i].index2].hitIntensity = 1.0f;

								collision = true;
								glm::vec3 hitPoint = vertexPos + hitDistance * glm::normalize(rayDirection);

								CalcLocalFalloff(tree, target, targetOctant, hitPoint);
								//return true;
							}
						}
						//return false;
					}
				}
			}
			
		}
		//cast rays from target onto projectile (inverse)
		
		for (int i = 0; i < target.targetModel.meshes[0].vertices.size(); i++)
		{
			
			glm::vec3 vertexPos = target.targetModel.meshes[0].vertices[i].Position;
			OctreeNode* targetOctant = projectileTree.FindOctant(vertexPos - speed);
			int indexX = projectileTree.calcNodeIndexX(targetOctant);
			int indexY = projectileTree.calcNodeIndexY(targetOctant);
			int indexZ = projectileTree.calcNodeIndexZ(targetOctant);


			for (int x = max(indexX - 3, 0); x < min(indexX + 3, 7); x++)
			{
				for (int z = max(indexZ - 3, 0); z < min(indexZ + 3, 7); z++)
				{
					if (targetOctant != nullptr)
					{
						if (projectileTree.arrayRepresentation[x][indexY][z]->tris != nullptr && projectileTree.arrayRepresentation[x][indexY][z]->tris->size() > 0)
						{
							float hitDistance;
							for (int j = 0; j < projectileTree.arrayRepresentation[x][indexY][z]->tris->size(); j++)
							{
								glm::vec3 vert0 = projectileTree.model.meshes[0].vertices[(*projectileTree.arrayRepresentation[x][indexY][z]->tris)[j].index0].Position;
								glm::vec3 vert1 = projectileTree.model.meshes[0].vertices[(*projectileTree.arrayRepresentation[x][indexY][z]->tris)[j].index1].Position;
								glm::vec3 vert2 = projectileTree.model.meshes[0].vertices[(*projectileTree.arrayRepresentation[x][indexY][z]->tris)[j].index2].Position;
								bool rayResult = RayUtil::MTRayCheck(vert0, vert1, vert2, vertexPos, glm::normalize(-rayDirection), hitDistance);
								if (rayResult && hitDistance < glm::length(speed))
								{
									affectedVerts.insert(i);
									target.vertInfo[i].hitIntensity = 1.0f;
									glm::vec3 hitPoint = vertexPos + hitDistance * glm::normalize(rayDirection);
									CalcLocalFalloff(tree, target, projectileTree.arrayRepresentation[x][indexY][z], hitPoint);
								}

							}
						}
					}
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

	void AffectFalloff(OctreeNode * node, OctreeTarget & target, glm::vec3 hitPoint)
	{
		//if there are triangles
		if (node->tris != nullptr && node->tris->size() > 0)
		{
			for (int i = 0; i < node->tris->size(); i++)
			{
				float hitIntensity = 
					target.falloffFunc(glm::length(target.targetModel.meshes[0].vertices[(*node->tris)[i].index0].Position - hitPoint));
				if (hitIntensity > target.vertInfo[(*node->tris)[i].index0].hitIntensity)
				{
					target.vertInfo[(*node->tris)[i].index0].hitIntensity = hitIntensity;
					affectedVerts.insert((*node->tris)[i].index0);
				}

				hitIntensity =
					target.falloffFunc(glm::length(target.targetModel.meshes[0].vertices[(*node->tris)[i].index1].Position - hitPoint));
				if (hitIntensity > target.vertInfo[(*node->tris)[i].index1].hitIntensity)
				{
					target.vertInfo[(*node->tris)[i].index1].hitIntensity = hitIntensity;
					affectedVerts.insert((*node->tris)[i].index1);
				}

				hitIntensity =
					target.falloffFunc(glm::length(target.targetModel.meshes[0].vertices[(*node->tris)[i].index2].Position - hitPoint));
				if (hitIntensity > target.vertInfo[(*node->tris)[i].index2].hitIntensity)
				{
					target.vertInfo[(*node->tris)[i].index2].hitIntensity = hitIntensity;
					affectedVerts.insert((*node->tris)[i].index2);
				}
			}
		}
	}

	void AffectLeafFalloff(int indexX, int indexY, int indexZ, Octree & tree, OctreeTarget & target, glm::vec3 hitPoint)
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
					AffectFalloff(tree.arrayRepresentation[indexX + i][indexY + j][indexZ + k], target, hitPoint);
				}
			}
		}
	}

	void CalcLocalFalloff(Octree& tree, OctreeTarget& target, OctreeNode* targetOctant, glm::vec3 hitPoint)
	{
		OctreeNode* falloffCenter = tree.FindFalloffCenterNode(hitPoint, target.falloff);
		//FindAdjacentToData(tree, target, falloffCenter, hitPoint, target.falloff);
		int indexX = tree.calcNodeIndexX(targetOctant);
		int indexY = tree.calcNodeIndexY(targetOctant);
		int indexZ = tree.calcNodeIndexZ(targetOctant);
		//std::cout << indexX << indexY << indexZ << "\n";
		AffectLeafFalloff(indexX, indexY, indexZ, tree, target, hitPoint);
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

	void Update(Octree& tree, Octree& projectileTree, OctreeTarget& target, float time, glm::mat4 model)
	{
		if (collision)
		{
			//Update distances to impact on vertices
			for (auto vert : affectedVerts)
			{
				tree.model.meshes[0].vertices[vert].Position += speed * target.vertInfo[vert].hitIntensity;
				tree.model.meshes[0].UpdateBufferVertexDirect(vert);
			}

			//If the speed beomes the opposite direction of the ray, we hammer it at zero,
			//because we don't want backwards movement

		}
		//boundingBoxCenterOffset += speed;
		ProcessRays(tree, projectileTree, target);

		for (int i = 0; i < optimizedVerts.size(); i++)
		{
			optimizedVerts[i] += speed; //Change position of all ray origins according to current speed
		}
		for (int i = 0; i < projectileMesh.meshes[0].vertices.size(); i++)
		{
			projectileMesh.meshes[0].vertices[i].Position += speed; //Change position of all vertices according to current speed
			projectileMesh.meshes[0].UpdateBufferVertexDirect(i);
		}


		projectileTree.UpdatePosition(speed);

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

	float boundingBoxSize;
	glm::vec3 boundingBoxCenter;
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
	glm::vec3 boundingBoxCenterOffset;
	std::vector<glm::vec3> optimizedVerts;
	std::vector<std::pair<int, float>> affectedVertices;
	std::set<int> affectedVerts;
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
	float hitDistance;
	glm::vec3 speed; //Current speed of projectile
	glm::vec3 hitPoint;
	Shader rayShader; //Shader of the ray itself
};




#endif