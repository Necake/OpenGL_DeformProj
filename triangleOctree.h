#ifndef TRI_OCTREE_H
#define TRI_OCTREE_H

#include<vector>
#include<iostream>
#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<glm\gtc\type_ptr.hpp>
#include<glm\gtc\constants.hpp>

#include"target.h"
#include"aabbtriCollision.h"

namespace vecUtil
{
	//is a right up front of b?
	bool isXpYpZp(glm::vec3 a, glm::vec3 b)
	{
		if ((a.x >= b.x) && (a.y >= b.y) && (a.z >= b.z))
			return true;
		return false;
	}
	//is a right up back of b?
	bool isXpYpZn(glm::vec3 a, glm::vec3 b)
	{
		if ((a.x >= b.x) && (a.y >= b.y) && (a.z < b.z))
			return true;
		return false;
	}
	//is a right down front of b?
	bool isXpYnZp(glm::vec3 a, glm::vec3 b)
	{
		if ((a.x >= b.x) && (a.y < b.y) && (a.z >= b.z))
			return true;
		return false;
	}
	//is a right down back of b?
	bool isXpYnZn(glm::vec3 a, glm::vec3 b)
	{
		if ((a.x >= b.x) && (a.y < b.y) && (a.z < b.z))
			return true;
		return false;
	}
	//is a left up front of b?
	bool isXnYpZp(glm::vec3 a, glm::vec3 b)
	{
		if ((a.x < b.x) && (a.y >= b.y) && (a.z >= b.z))
			return true;
		return false;
	}
	//is a left up back of b?
	bool isXnYpZn(glm::vec3 a, glm::vec3 b)
	{
		if ((a.x < b.x) && (a.y >= b.y) && (a.z < b.z))
			return true;
		return false;
	}
	//is a left down front of b?
	bool isXnYnZp(glm::vec3 a, glm::vec3 b)
	{
		if ((a.x < b.x) && (a.y < b.y) && (a.z >= b.z))
			return true;
		return false;
	}
	//is a left down back of b?
	bool isXnYnZn(glm::vec3 a, glm::vec3 b)
	{
		if ((a.x < b.x) && (a.y < b.y) && (a.z < b.z))
			return true;
		return false;
	}
	float x, y, z;
};


struct Triangle
{
	/*
	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;
	Triangle()
	{
		v0 = glm::vec3(); v1 = glm::vec3(); v2 = glm::vec3();
	}
	Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2)
	{
		this->v0 = v0;
		this->v1 = v1;
		this->v2 = v2;
	}*/
	//indices from the model indices
	int index0;
	int index1;
	int index2;

	glm::vec3 positions[3];

	Triangle(int i0, int i1, int i2)
	{
		index0 = i0;
		index1 = i1;
		index2 = i2;
	}
	Triangle()
	{
		index0 = -1;
		index1 = -1;
		index2 = -1;
	}
};

struct OctreeNode
{

	OctreeNode(float size, glm::vec3 position)
	{
		this->position = position;
		this->size = size;
	}

	OctreeNode* XpYpZp = NULL;
	OctreeNode* XpYpZn = NULL;
	OctreeNode* XpYnZp = NULL;
	OctreeNode* XpYnZn = NULL;
	OctreeNode* XnYpZp = NULL;
	OctreeNode* XnYpZn = NULL;
	OctreeNode* XnYnZp = NULL;
	OctreeNode* XnYnZn = NULL;

	std::vector<Vertex>* vertices = NULL; //todo change to indices
	std::vector<Triangle>* tris = NULL;
	glm::vec3 position;
	float size;
};


OctreeNode* newOctreeNode(float size, glm::vec3 position)
{
	OctreeNode* newNode = new OctreeNode(size, position);
	newNode->vertices = nullptr;
	newNode->tris = nullptr;
	return newNode;
}

bool satTest(glm::vec3 pos0, glm::vec3 pos1, glm::vec3 pos2, glm::vec3 axis, float e, glm::vec3 norm1, glm::vec3 norm2, glm::vec3 norm3)
{
	//project triangle/aabb onto the axis
	float p0 = glm::dot(pos0, axis);
	float p1 = glm::dot(pos1, axis);
	float p2 = glm::dot(pos2, axis);
	float r = e * fabs(glm::dot(norm1, axis)) + e * fabs(glm::dot(norm2, axis)) + e * fabs(glm::dot(norm3, axis));

	if (fmaxf(-fmaxf(fmaxf(p0, p1), p2), fminf(fminf(p0, p1), p2)) > r)
	{
		return false;
	}
	return true;
}

bool TriangleOctantIntersection(const Model& model, float octantSize, glm::vec3 octantPos, Triangle tri)
{
	float e = octantSize / 2;
	//translate all verts so the cube pos is actually the origin when testing
	glm::vec3 v0 = model.meshes[0].vertices[model.meshes[0].indices[tri.index0]].Position - octantPos;
	glm::vec3 v1 = model.meshes[0].vertices[model.meshes[0].indices[tri.index1]].Position - octantPos;
	glm::vec3 v2 = model.meshes[0].vertices[model.meshes[0].indices[tri.index2]].Position - octantPos;

	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v1;
	glm::vec3 edge3 = v0 - v2;
	glm::vec3 triNorm = glm::cross((v2 - v0), (v1 - v0));

	glm::vec3 norm1 = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 norm2 = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 norm3 = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec3 axisn1e1 = glm::cross(norm1, edge1);
	glm::vec3 axisn1e2 = glm::cross(norm1, edge2);
	glm::vec3 axisn1e3 = glm::cross(norm1, edge3);

	glm::vec3 axisn2e1 = glm::cross(norm2, edge1);
	glm::vec3 axisn2e2 = glm::cross(norm2, edge2);
	glm::vec3 axisn2e3 = glm::cross(norm2, edge3);

	glm::vec3 axisn3e1 = glm::cross(norm3, edge1);
	glm::vec3 axisn3e2 = glm::cross(norm3, edge2);
	glm::vec3 axisn3e3 = glm::cross(norm3, edge3);


	if (!satTest(v0, v1, v2, axisn1e1, e, norm1, norm2, norm3)) //1
		return false;
	if (!satTest(v0, v1, v2, axisn1e2, e, norm1, norm2, norm3)) //2
		return false;
	if (!satTest(v0, v1, v2, axisn1e3, e, norm1, norm2, norm3)) //3
		return false;
	if (!satTest(v0, v1, v2, axisn2e1, e, norm1, norm2, norm3)) //4
		return false;
	if (!satTest(v0, v1, v2, axisn2e2, e, norm1, norm2, norm3)) //5
		return false;
	if (!satTest(v0, v1, v2, axisn2e3, e, norm1, norm2, norm3)) //6
		return false;
	if (!satTest(v0, v1, v2, axisn3e1, e, norm1, norm2, norm3)) //7
		return false;
	if (!satTest(v0, v1, v2, axisn3e2, e, norm1, norm2, norm3)) //8
		return false;
	if (!satTest(v0, v1, v2, axisn3e3, e, norm1, norm2, norm3)) //9
		return false;
	if (!satTest(v0, v1, v2, norm1, e, norm1, norm2, norm3)) //10
		return false;
	if (!satTest(v0, v1, v2, norm2, e, norm1, norm2, norm3)) //11
		return false;
	if (!satTest(v0, v1, v2, norm3, e, norm1, norm2, norm3)) //12
		return false;
	if (!satTest(v0, v1, v2, triNorm, e, norm1, norm2, norm3)) //13
		return false;

	return true;
}


class Octree
{
public:
	Octree(Model& model, float minSize, int maxVerts, int maxTris, int depth, float initSize, glm::vec3 initPos): model(model)
	{
		this->minSize = minSize;
		this->maxVerts = maxVerts;
		this->maxTris = maxTris;
		this->depth = depth;
		this->size = initSize;
		root = newOctreeNode(size, initPos);//new OctreeNode(size, initPos); //init cvor
		InitSubdivide(root, this->depth);
	}
	~Octree()
	{
		DestroyTree();
	}

	void Insert(std::vector<glm::vec3> dataArray)
	{
		if (root != NULL)
			Insert(dataArray, root);
		else
		{
			std::cout << "if i got here, you fucked something up??\n";
			//root = newOctreeNode(data);
		}
	}

	OctreeNode* FindFalloffCenterNode(glm::vec3 hitPoint, float falloff)
	{
		return FindFalloffCenterNode(hitPoint, root, falloff);
	}

	void InsertTriangles(std::vector<Triangle> dataArray)
	{
		if (root != NULL)
			InsertTriangles(dataArray, root);
		else
		{
			std::cout << "if i got here, you fucked something up??\n";
			//root = newOctreeNode(data);
		}
	}

	OctreeNode* FindOctant(glm::vec3 data)
	{
		if (fabs(data.x) > size || fabs(data.y) > size || fabs(data.z) > size)
		{
			//the vector is outside of the tree
			return nullptr;
		}
		return FindOctant(data, root);
	}

	OctreeNode* Search(glm::vec3 data)
	{
		return Search(data, root);
	}
	void DestroyTree()
	{
		DestroyTree(root);
	}

	OctreeNode* root;
	Model& model;
	float minSize; //the most subdivisions allowed
	float size; //size of the space
	int maxVerts; //the most vertices allowed in an octant
	int maxTris; //the most tris allowed in an octant
	int depth; //initial depth
private:
	void DestroyTree(OctreeNode* leaf)
	{
		if (leaf->XpYpZp != nullptr) //checking the first child is enough, since all will exist, or none will exist
		{
			DestroyTree(leaf->XpYpZp);
			DestroyTree(leaf->XpYpZn);
			DestroyTree(leaf->XpYnZp);
			DestroyTree(leaf->XpYnZn);
			DestroyTree(leaf->XnYpZp);
			DestroyTree(leaf->XnYpZn);
			DestroyTree(leaf->XnYnZp);
			DestroyTree(leaf->XnYnZn);
			delete leaf;
		}
	}

	void InitEmptyChildren(OctreeNode* node)
	{

		float newSize = node->size / 2.0f;
		float offset = newSize / 2.0f;
		node->XpYpZp = newOctreeNode(newSize,
			glm::vec3(node->position.x + offset, node->position.y + offset, node->position.z + offset));
		node->XpYpZn = newOctreeNode(newSize,
			glm::vec3(node->position.x + offset, node->position.y + offset, node->position.z - offset));

		node->XpYnZp = newOctreeNode(newSize,
			glm::vec3(node->position.x + offset, node->position.y - offset, node->position.z + offset));
		node->XpYnZn = newOctreeNode(newSize,
			glm::vec3(node->position.x + offset, node->position.y - offset, node->position.z - offset));

		node->XnYpZp = newOctreeNode(newSize,
			glm::vec3(node->position.x - offset, node->position.y + offset, node->position.z + offset));
		node->XnYpZn = newOctreeNode(newSize,
			glm::vec3(node->position.x - offset, node->position.y + offset, node->position.z - offset));

		node->XnYnZp = newOctreeNode(newSize,
			glm::vec3(node->position.x - offset, node->position.y - offset, node->position.z + offset));
		node->XnYnZn = newOctreeNode(newSize,
			glm::vec3(node->position.x - offset, node->position.y - offset, node->position.z - offset));
	}

	void InitSubdivide(OctreeNode * node, int depth) //granam svaki cvor do dubine n
	{
		//dati cvor je inicijalizovan, ja mu pravim decu
		if (depth > 0) //create n depth layers of the tree initially
		{
			//init children
			InitEmptyChildren(node);
			InitSubdivide(node->XpYpZp, depth - 1);
			InitSubdivide(node->XpYpZn, depth - 1);
			InitSubdivide(node->XpYnZp, depth - 1);
			InitSubdivide(node->XpYnZn, depth - 1);
			InitSubdivide(node->XnYpZp, depth - 1);
			InitSubdivide(node->XnYpZn, depth - 1);
			InitSubdivide(node->XnYnZp, depth - 1);
			InitSubdivide(node->XnYnZn, depth - 1);
		}
		else
		{
			std::cout << "leef\n";
		}

	}

	void Insert(std::vector<glm::vec3> dataArray, OctreeNode * node)
	{
		if ((dataArray.size() < maxVerts) || (node->size / 2.0f < minSize))
		{
			//exit recursion tree lol
			//std::cout << "im at last recursion step";
			if (dataArray.size() > 0)
			{
				node->vertices = new std::vector<Vertex>();
				//std::cout << ", condition met\n";
				for (auto pos : dataArray)
				{
					Vertex vert;
					vert.Position = pos;
					std::cout << "pushing back vertex... ";
					node->vertices->push_back(vert);
					std::cout << "done!\n";
				}
				std::cout << "pushed back data of " << dataArray.size() << " verts into node sized " << node->size << "\n";

			}
		}
		else
		{
			std::vector<glm::vec3> splits[8];
			for (auto pos : dataArray)
			{
				if (vecUtil::isXpYpZp(pos, node->position))
					splits[0].push_back(pos);
				else if (vecUtil::isXpYpZn(pos, node->position))
					splits[1].push_back(pos);
				else if (vecUtil::isXpYnZp(pos, node->position))
					splits[2].push_back(pos);
				else if (vecUtil::isXpYnZn(pos, node->position))
					splits[3].push_back(pos);
				else if (vecUtil::isXnYpZp(pos, node->position))
					splits[4].push_back(pos);
				else if (vecUtil::isXnYpZn(pos, node->position))
					splits[5].push_back(pos);
				else if (vecUtil::isXnYnZp(pos, node->position))
					splits[6].push_back(pos);
				else if (vecUtil::isXnYnZn(pos, node->position))
					splits[7].push_back(pos);
			}

			if (node->XpYpZp == NULL) //if it's a leaf, but none of the conditions have been met
				InitEmptyChildren(node); //wasteful, todo fix

			Insert(splits[0], node->XpYpZp);
			Insert(splits[1], node->XpYpZn);
			Insert(splits[2], node->XpYnZp);
			Insert(splits[3], node->XpYnZn);
			Insert(splits[4], node->XnYpZp);
			Insert(splits[5], node->XnYpZn);
			Insert(splits[6], node->XnYnZp);
			Insert(splits[7], node->XnYnZn);
		}
	}

	/*
	void InsertTriangles(std::vector<Triangle> dataArray, OctreeNode* node)
	{
		if (((dataArray.size() < maxTris) || (node->size / 2.0f < minSize)) && node->XpYpZp == nullptr)
		{
			//exit recursion and piss off
			if (dataArray.size() > 0)
			{
				node->tris = new std::vector<Triangle>();
				//std::cout << ", condition met\n";
				for (auto tri : dataArray)
				{
					if (TriangleOctantIntersection(model, node->size, node->position, tri)) //if they intersect, add it here
					{
						std::cout << "pushing back triangle... ";
						node->tris->push_back(tri);
						std::cout << "done!\n";
					}
				}
				std::cout << "pushed back data of " << dataArray.size() << " tris into node sized " << node->size << "addr: " << node << "\n";
			}
		}
		else
		{
			std::vector<Triangle> splits[8];
			float halfSize = node->size / 4;
			for (int i =0; i < dataArray.size(); i++)
			{
				Triangle newTri = dataArray[i];
				if (TriangleOctantIntersection(model, halfSize * 2, node->position + glm::vec3(halfSize, halfSize, halfSize), newTri))
				{
					splits[0].push_back(newTri);

				}
				if (TriangleOctantIntersection(model, halfSize * 2, node->position + glm::vec3(halfSize, halfSize, -halfSize), newTri))
				{
					splits[1].push_back(newTri);

				}
				if (TriangleOctantIntersection(model, halfSize * 2, node->position + glm::vec3(halfSize, -halfSize, halfSize), newTri))
				{
					splits[2].push_back(newTri);

				}
				if (TriangleOctantIntersection(model, halfSize * 2, node->position + glm::vec3(halfSize, -halfSize, -halfSize), newTri))
				{
					splits[3].push_back(newTri);

				}
				if (TriangleOctantIntersection(model, halfSize * 2, node->position + glm::vec3(-halfSize, halfSize, halfSize), newTri))
				{
					splits[4].push_back(newTri);

				}
				if (TriangleOctantIntersection(model, halfSize * 2, node->position + glm::vec3(-halfSize, halfSize, -halfSize), newTri))
				{
					splits[5].push_back(newTri);

				}
				if (TriangleOctantIntersection(model, halfSize * 2, node->position + glm::vec3(-halfSize, -halfSize, halfSize), newTri))
				{
					splits[6].push_back(newTri);

				}
				if (TriangleOctantIntersection(model, halfSize * 2, node->position + glm::vec3(-halfSize, -halfSize, -halfSize), newTri))
				{
					splits[7].push_back(newTri);

				}
			}

			if (node->XpYpZp == NULL) //if it's a leaf, but none of the conditions have been met
				InitEmptyChildren(node); //wasteful, todo fix

			InsertTriangles(splits[0], node->XpYpZp);
			InsertTriangles(splits[1], node->XpYpZn);
			InsertTriangles(splits[2], node->XpYnZp);
			InsertTriangles(splits[3], node->XpYnZn);
			InsertTriangles(splits[4], node->XnYpZp);
			InsertTriangles(splits[5], node->XnYpZn);
			InsertTriangles(splits[6], node->XnYnZp);
			InsertTriangles(splits[7], node->XnYnZn);
		}
	}*/
	void InsertTriangles(std::vector<Triangle> dataArray, OctreeNode* node)
	{
		if (node->XpYpZp == nullptr)
		{
			//onda je list, odjebi i zavrsi

			node->tris = new std::vector<Triangle>();
			for (int i = 0; i < dataArray.size(); i++)
			{
				glm::vec3 triangleVerts[3] = { model.meshes[0].vertices[dataArray[i].index0].Position,
					model.meshes[0].vertices[dataArray[i].index1].Position, 
					model.meshes[0].vertices[dataArray[i].index2].Position };
				if (triBoxOverlap(node->position, glm::vec3(node->size / 2, node->size / 2, node->size / 2), triangleVerts)) //(model, node->size, node->position, dataArray[i]))
				{
					dataArray[i].positions[0] = model.meshes[0].vertices[dataArray[i].index0].Position;
					dataArray[i].positions[1] = model.meshes[0].vertices[dataArray[i].index1].Position;
					dataArray[i].positions[2] = model.meshes[0].vertices[dataArray[i].index2].Position;
					node->tris->push_back(dataArray[i]);
					//std::cout << "pushed triangle into addr: " << node << "\n";
				}
			}
		}
		else
		{
			InsertTriangles(dataArray, node->XpYpZp);
			InsertTriangles(dataArray, node->XpYpZn);
			InsertTriangles(dataArray, node->XpYnZp);
			InsertTriangles(dataArray, node->XpYnZn);
			InsertTriangles(dataArray, node->XnYpZp);
			InsertTriangles(dataArray, node->XnYpZn);
			InsertTriangles(dataArray, node->XnYnZp);
			InsertTriangles(dataArray, node->XnYnZn);
		}
	}

	OctreeNode* FindFalloffCenterNode(glm::vec3 hitPoint, OctreeNode* node, float falloff)
	{
		if ((node->size / 2 < falloff) || (node->XpYpZp == nullptr)) //if it's small enough, or a leaf if the falloff is smaller than the leaves
		{
			//recursion exit
			return node;
		}
		else if (vecUtil::isXpYpZp(hitPoint, node->position)) //condition not met, search further
			FindFalloffCenterNode(hitPoint, node->XpYpZp, falloff);
		else if (vecUtil::isXpYpZn(hitPoint, node->position))
			FindFalloffCenterNode(hitPoint, node->XpYpZn, falloff);
		else if (vecUtil::isXpYnZp(hitPoint, node->position))
			FindFalloffCenterNode(hitPoint, node->XpYnZp, falloff);
		else if (vecUtil::isXpYnZn(hitPoint, node->position))
			FindFalloffCenterNode(hitPoint, node->XpYnZn, falloff);
		else if (vecUtil::isXnYpZp(hitPoint, node->position))
			FindFalloffCenterNode(hitPoint, node->XnYpZp, falloff);
		else if (vecUtil::isXnYpZn(hitPoint, node->position))
			FindFalloffCenterNode(hitPoint, node->XnYpZn, falloff);
		else if (vecUtil::isXnYnZp(hitPoint, node->position))
			FindFalloffCenterNode(hitPoint, node->XnYnZp, falloff);
		else if (vecUtil::isXnYnZn(hitPoint, node->position))
			FindFalloffCenterNode(hitPoint, node->XnYnZn, falloff);
	}


	//To be used for falloff functions
	OctreeNode* FindAdjacentToData(OctreeNode * node, glm::vec3 data)
	{
		int answer = 0b000000;
		if (data.x + minSize > node->position.x + node->size) //if we get out 
			answer |= 0b000001; //need to check x+
		else if (data.x - minSize < node->position.x - node->size)
			answer |= 0b001000; //need to check x-
		if (data.y + minSize > node->position.y + node->size)
			answer |= 0b000010; //need to check y+
		else if (data.y - minSize < node->position.y - node->size)
			answer |= 0b010000; //need to check y-
		if (data.z + minSize > node->position.z + node->size)
			answer |= 0b000100; //need to check z+
		else if (data.z + minSize > node->position.z + node->size)
			answer |= 0b100000; //need to check z-

		if (answer & 0b000001 == 0b000001)
		{
			//x+
			glm::vec3 temp = glm::vec3(data.x + minSize, data.y, data.z);
			FindOctant(temp);
			if (answer & 0b000010 == 0b000010)
			{
				//y+
				temp = glm::vec3(data.x, data.y + minSize, data.z);
				FindOctant(temp);
				temp.x += minSize; //x+y+
				FindOctant(temp);
				if (answer & 0b000100 == 0b000100)
				{
					//z+
					temp = glm::vec3(data.x, data.y, data.z + minSize);
					FindOctant(temp);
					temp.x += minSize;//x+z+
					FindOctant(temp);
					temp = glm::vec3(data.x, data.y + minSize, data.z + minSize);//y+z+
					FindOctant(temp);
					temp.x += minSize; //x+y+z+
					FindOctant(temp);
				}
				else if (answer & 0b100000 == 0b100000)
				{
					//z-
					temp = glm::vec3(data.x, data.y, data.z - minSize);
					FindOctant(temp);
					temp.x += minSize;//x+z-
					FindOctant(temp);
					temp = glm::vec3(data.x, data.y + minSize, data.z - minSize);//y+z-
					FindOctant(temp);
					temp.x += minSize; //x+y+z-
					FindOctant(temp);
				}
			}
			else if (answer & 0b010000 == 0b010000)
			{
				//y-
				glm::vec3 temp = glm::vec3(data.x, data.y - minSize, data.z);
				FindOctant(temp);
				temp.x += minSize; //x+y-
				FindOctant(temp);
				if (answer & 0b000100 == 0b000100)
				{
					//z+
					temp = glm::vec3(data.x, data.y, data.z + minSize);
					FindOctant(temp);
					temp.x += minSize;//x+z+
					FindOctant(temp);
					temp = glm::vec3(data.x, data.y - minSize, data.z + minSize);//y-z+
					FindOctant(temp);
					temp.x += minSize; //x+y-z+
					FindOctant(temp);
				}
				else if (answer & 0b100000 == 0b100000)
				{
					//z-
					temp = glm::vec3(data.x, data.y, data.z - minSize);
					FindOctant(temp);
					temp.x += minSize;//x+z-
					FindOctant(temp);
					temp = glm::vec3(data.x, data.y - minSize, data.z - minSize);//y-z-
					FindOctant(temp);
					temp.x += minSize; //x+y-z-
					FindOctant(temp);
				}
			}
		}
		else if (answer & 0b001000 == 0b001000)
		{
			//x-
			glm::vec3 temp = glm::vec3(data.x - minSize, data.y, data.z);
			FindOctant(temp, root);
			if (answer & 0b000010 == 0b000010)
			{
				//y+
				temp = glm::vec3(data.x, data.y + minSize, data.z);
				FindOctant(temp);
				temp.x -= minSize; //x-y+
				FindOctant(temp);
				if (answer & 0b000100 == 0b000100)
				{
					//z+
					temp = glm::vec3(data.x, data.y, data.z + minSize);
					FindOctant(temp);
					temp.x -= minSize;//x-z+
					FindOctant(temp);
					temp = glm::vec3(data.x, data.y + minSize, data.z + minSize);//y+z+
					FindOctant(temp);
					temp.x -= minSize; //x-y+z+
					FindOctant(temp);
				}
				else if (answer & 0b100000 == 0b100000)
				{
					//z-
					temp = glm::vec3(data.x, data.y, data.z - minSize);
					FindOctant(temp);
					temp.x -= minSize;//x-z-
					FindOctant(temp);
					temp = glm::vec3(data.x, data.y + minSize, data.z - minSize);//y+z-
					FindOctant(temp);
					temp.x -= minSize; //x-y+z-
					FindOctant(temp);
				}
			}
			else if (answer & 0b010000 == 0b010000)
			{
				//y-
				glm::vec3 temp = glm::vec3(data.x, data.y - minSize, data.z);
				FindOctant(temp);
				temp.x -= minSize; //x-y-
				FindOctant(temp);
				if (answer & 0b000100 == 0b000100)
				{
					//z+
					temp = glm::vec3(data.x, data.y, data.z + minSize);
					FindOctant(temp);
					temp.x -= minSize;//x-z+
					FindOctant(temp);
					temp = glm::vec3(data.x, data.y - minSize, data.z + minSize);//y-z+
					FindOctant(temp);
					temp.x -= minSize; //x-y-z+
					FindOctant(temp);
				}
				else if (answer & 0b100000 == 0b100000)
				{
					//z-
					temp = glm::vec3(data.x, data.y, data.z - minSize);
					FindOctant(temp);
					temp.x += minSize;//x-z-
					FindOctant(temp);
					temp = glm::vec3(data.x, data.y - minSize, data.z - minSize);//y-z-
					FindOctant(temp);
					temp.x += minSize; //x-y-z-
					FindOctant(temp);
				}
			}
		}

		return nullptr;
	}

	//Find octant of given data, will use for falloff origin
	OctreeNode* FindOctant(glm::vec3 data, OctreeNode * node)
	{
		if (node->XpYpZp == nullptr)
		{
			//if it's a leaf
			return node;
		}
		else if (vecUtil::isXpYpZp(data, node->position)) //not a leaf
			FindOctant(data, node->XpYpZp);
		else if (vecUtil::isXpYpZn(data, node->position))
			FindOctant(data, node->XpYpZn);
		else if (vecUtil::isXpYnZp(data, node->position))
			FindOctant(data, node->XpYnZp);
		else if (vecUtil::isXpYnZn(data, node->position))
			FindOctant(data, node->XpYnZn);
		else if (vecUtil::isXnYpZp(data, node->position))
			FindOctant(data, node->XnYpZp);
		else if (vecUtil::isXnYpZn(data, node->position))
			FindOctant(data, node->XnYpZn);
		else if (vecUtil::isXnYnZp(data, node->position))
			FindOctant(data, node->XnYnZp);
		else if (vecUtil::isXnYnZn(data, node->position))
			FindOctant(data, node->XnYnZn);
	}

	OctreeNode* Search(glm::vec3 data, OctreeNode * leaf)
	{
		if (leaf != nullptr)
		{
			if (leaf->vertices != nullptr) //this automatically means that we are in a leaf, and have data
			{
				bool found = false;
				for (auto vert : (*leaf->vertices))
				{
					if (vert.Position == data)
						return leaf;
				}
				if (!found)
					return nullptr;

			}
			if (vecUtil::isXpYpZp(data, leaf->position))
				return Search(data, leaf->XpYpZp);
			else if (vecUtil::isXpYpZn(data, leaf->position))
				return Search(data, leaf->XpYpZn);
			else if (vecUtil::isXpYnZp(data, leaf->position))
				return Search(data, leaf->XpYnZp);
			else if (vecUtil::isXpYnZn(data, leaf->position))
				return Search(data, leaf->XpYnZn);
			else if (vecUtil::isXnYpZp(data, leaf->position))
				return Search(data, leaf->XnYpZp);
			else if (vecUtil::isXnYpZn(data, leaf->position))
				return Search(data, leaf->XnYpZn);
			else if (vecUtil::isXnYnZp(data, leaf->position))
				return Search(data, leaf->XnYnZp);
			else if (vecUtil::isXnYnZn(data, leaf->position))
				return Search(data, leaf->XnYnZn);
		}
		else
			return nullptr;
	}
};


#endif