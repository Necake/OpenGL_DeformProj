#ifndef OCTREE_H
#define OCTREE_H

#include<vector>
#include<iostream>


class vec3
{
public:
	vec3(float x, float y, float z) : x(x), y(y), z(z)
	{

	}
	vec3()
	{
		x = 0; y = 0; z = 0;
	}
	//is a right up front of b?
	bool isXpYpZp(vec3 b)
	{
		if ((x >= b.x) && (y >= b.y) && (z >= b.z))
			return true;
		return false;
	}
	//is a right up back of b?
	bool isXpYpZn(vec3 b)
	{
		if ((x >= b.x) && (y >= b.y) && (z < b.z))
			return true;
		return false;
	}
	//is a right down front of b?
	bool isXpYnZp(vec3 b)
	{
		if ((x >= b.x) && (y < b.y) && (z >= b.z))
			return true;
		return false;
	}
	//is a right down back of b?
	bool isXpYnZn(vec3 b)
	{
		if ((x >= b.x) && (y < b.y) && (z < b.z))
			return true;
		return false;
	}
	//is a left up front of b?
	bool isXnYpZp(vec3 b)
	{
		if ((x < b.x) && (y >= b.y) && (z >= b.z))
			return true;
		return false;
	}
	//is a left up back of b?
	bool isXnYpZn(vec3 b)
	{
		if ((x < b.x) && (y >= b.y) && (z < b.z))
			return true;
		return false;
	}
	//is a left down front of b?
	bool isXnYnZp(vec3 b)
	{
		if ((x < b.x) && (y < b.y) && (z >= b.z))
			return true;
		return false;
	}
	//is a left down back of b?
	bool isXnYnZn(vec3 b)
	{
		if ((x < b.x) && (y < b.y) && (z < b.z))
			return true;
		return false;
	}
	float x, y, z;

private:
};

struct Vertex
{
	vec3 position;
	vec3 normal;
};


bool operator==(const vec3 & lhs, const vec3 & rhs)
{
	if (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z)
		return true;
	return false;
}

struct OctreeNode
{

	OctreeNode(float size, vec3 position)
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

	std::vector<Vertex>* data = NULL;
	vec3 position;
	float size;
};

OctreeNode* newOctreeNode(float size, vec3 position)
{
	OctreeNode* newNode = new OctreeNode(size, position);
	newNode->data = nullptr;
	return newNode;
}

class Octree
{
public:
	Octree(float minSize, int maxVerts, int depth, float initSize, vec3 initPos)
	{
		this->minSize = minSize;
		this->maxVerts = maxVerts;
		this->depth = depth;
		this->size = initSize;
		root = newOctreeNode(size, initPos);//new OctreeNode(size, initPos); //init cvor
		InitSubdivide(root, this->depth);

	}
	~Octree()
	{
		DestroyTree();
	}

	void Insert(std::vector<vec3> dataArray)
	{
		if (root != NULL)
			Insert(dataArray, root);
		else
		{
			std::cout << "if i got here, you fucked something up??\n";
			//root = newOctreeNode(data);
		}
	}

	OctreeNode* Search(vec3 data)
	{
		return Search(data, root);
	}
	void DestroyTree()
	{
		DestroyTree(root);
	}

	OctreeNode* root;
	float minSize; //the most subdivisions allowed
	float size; //size of the space
	int maxVerts; //the most vertices allowed in a octant
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
		node->XpYpZp = newOctreeNode(newSize,
			vec3(node->position.x + newSize, node->position.y + newSize, node->position.z + newSize));
		node->XpYpZn = newOctreeNode(newSize,
			vec3(node->position.x + newSize, node->position.y + newSize, node->position.z - newSize));

		node->XpYnZp = newOctreeNode(newSize,
			vec3(node->position.x + newSize, node->position.y - newSize, node->position.z + newSize));
		node->XpYnZn = newOctreeNode(newSize,
			vec3(node->position.x + newSize, node->position.y - newSize, node->position.z - newSize));

		node->XnYpZp = newOctreeNode(newSize,
			vec3(node->position.x - newSize, node->position.y + newSize, node->position.z + newSize));
		node->XnYpZn = newOctreeNode(newSize,
			vec3(node->position.x - newSize, node->position.y + newSize, node->position.z - newSize));

		node->XnYnZp = newOctreeNode(newSize,
			vec3(node->position.x - newSize, node->position.y - newSize, node->position.z + newSize));
		node->XnYnZn = newOctreeNode(newSize,
			vec3(node->position.x - newSize, node->position.y - newSize, node->position.z - newSize));
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

	void Insert(std::vector<vec3> dataArray, OctreeNode * node)
	{
		if ((dataArray.size() < maxVerts) || (node->size / 2.0f < minSize))
		{
			//exit recursion tree lol
			//std::cout << "im at last recursion step";
			if (dataArray.size() > 0)
			{
				node->data = new std::vector<Vertex>();
				//std::cout << ", condition met\n";
				for (auto pos : dataArray)
				{
					Vertex vert;
					vert.position = pos;
					std::cout << "pushing back vertex... ";
					node->data->push_back(vert);
					std::cout << "done!\n";
				}
				std::cout << "pushed back data of " << dataArray.size() << " verts into node sized " << node->size << "\n";

			}
		}
		else
		{
			std::vector<vec3> splits[8];
			for (auto pos : dataArray)
			{
				if (pos.isXpYpZp(node->position))
					splits[0].push_back(pos);
				else if (pos.isXpYpZn(node->position))
					splits[1].push_back(pos);
				else if (pos.isXpYnZp(node->position))
					splits[2].push_back(pos);
				else if (pos.isXpYnZn(node->position))
					splits[3].push_back(pos);
				else if (pos.isXnYpZp(node->position))
					splits[4].push_back(pos);
				else if (pos.isXnYpZn(node->position))
					splits[5].push_back(pos);
				else if (pos.isXnYnZp(node->position))
					splits[6].push_back(pos);
				else if (pos.isXnYnZn(node->position))
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


	OctreeNode* Search(vec3 data, OctreeNode * leaf)
	{
		if (leaf != nullptr)
		{
			if (leaf->data != nullptr) //this automatically means that we are in a leaf, and have data
			{
				bool found = false;
				for (auto vert : (*leaf->data))
				{
					if (vert.position == data)
						return leaf;
				}
				if (!found)
					return nullptr;

			}
			if (data.isXpYpZp(leaf->position))
				return Search(data, leaf->XpYpZp);
			else if (data.isXpYpZn(leaf->position))
				return Search(data, leaf->XpYpZn);
			else if (data.isXpYnZp(leaf->position))
				return Search(data, leaf->XpYnZp);
			else if (data.isXpYnZn(leaf->position))
				return Search(data, leaf->XpYnZn);
			else if (data.isXnYpZp(leaf->position))
				return Search(data, leaf->XnYpZp);
			else if (data.isXnYpZn(leaf->position))
				return Search(data, leaf->XnYpZn);
			else if (data.isXnYnZp(leaf->position))
				return Search(data, leaf->XnYnZp);
			else if (data.isXnYnZn(leaf->position))
				return Search(data, leaf->XnYnZn);
		}
		else
			return nullptr;
	}
};

#endif