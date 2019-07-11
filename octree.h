class vec3
{
public:
	vec3(float x, float y, float z): x(x), y(y), z(z)
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

bool operator==(const vec3& lhs, const vec3& rhs)
{
	if (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z)
		return true;
	return false;
}


struct Node
{
	int data;
	Node* left;
	Node* right;
};

struct OctreeNode
{
	OctreeNode()
	{
		data = vec3(0, 0, 0);
	}
	vec3 data;
	OctreeNode* XpYpZp;
	OctreeNode* XpYpZn;
	OctreeNode* XpYnZp;
	OctreeNode* XpYnZn;
	OctreeNode* XnYpZp;
	OctreeNode* XnYpZn;
	OctreeNode* XnYnZp;
	OctreeNode* XnYnZn;
};

class bTree
{
public:
	bTree()
	{
		root = nullptr;
	}
	~bTree()
	{
		DestroyTree();
	}

	void Insert(int data)
	{
		if (root != nullptr)
			Insert(data, root);
		else
		{
			root = new Node;
			root->data = data;
			root->left = nullptr;
			root->right = nullptr;
		}
	}
	Node* Search(int data)
	{
		return Search(data, root);
	}
	void DestroyTree()
	{
		DestroyTree(root);
	}

	Node* root;
private:
	void DestroyTree(Node* leaf)
	{
		if (leaf != nullptr)
		{
			DestroyTree(leaf->left);
			DestroyTree(leaf->right);
			delete leaf;
		}
	}

	void Insert(int data, Node* leaf)
	{
		if (data < leaf->data)
		{
			if(leaf->left != nullptr)
				Insert(data, leaf->left);
			else
			{
				leaf->left = new Node;
				leaf->left->data = data;
				leaf->left->left = nullptr;
				leaf->left->right = nullptr;
			}

		}
		else if (data >= leaf->data)
		{
			if (leaf->right != nullptr)
				Insert(data, leaf->left);
			else
			{
				leaf->right = new Node;
				leaf->right->data = data;
				leaf->right->left = nullptr;
				leaf->right->right = nullptr;
			}
		}
			
	}
	Node* Search(int data, Node* leaf)
	{
		if (leaf != nullptr)
		{
			if (data == leaf->data)
				return leaf;
			if (data < leaf->data)
				return Search(data, leaf->left);
			else
				return Search(data, leaf->right);
		}
		else
			return nullptr;
	}
};

OctreeNode* newOctreeNode(vec3 data)
{
	OctreeNode* newNode = new OctreeNode();
	newNode->data = data;
	newNode ->XpYpZp = nullptr;
	newNode ->XpYpZn = nullptr;
	newNode ->XpYnZp = nullptr;
	newNode ->XpYnZn = nullptr;
	newNode ->XnYpZp = nullptr;
	newNode ->XnYpZn = nullptr;
	newNode ->XnYnZp = nullptr;
	newNode ->XnYnZn = nullptr;
	return newNode;
}

class Octree 
{
public:
	Octree()
	{
		root = nullptr;
	}
	~Octree()
	{
		DestroyTree();
	}

	void Insert(vec3 data)
	{
		if (root != nullptr)
			Insert(data, root);
		else
		{
			root = newOctreeNode(data);
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
private:
	void DestroyTree(OctreeNode* leaf)
	{
		if (leaf != nullptr)
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

	void Insert(vec3 data, OctreeNode* leaf)
	{
		if (data.isXpYpZp(leaf->data))
		{
			if (leaf->XpYpZp != nullptr)
				Insert(data, leaf->XpYpZp);
			else
			{
				leaf->XpYpZp = newOctreeNode(data);
			}

		}
		else if (data.isXpYpZn(leaf->data))
		{
			if (leaf->XpYpZn != nullptr)
				Insert(data, leaf->XpYpZn);
			else
			{
				leaf->XpYpZn = newOctreeNode(data);
			}
		}
		else if (data.isXpYnZp(leaf->data))
		{
			if (leaf->XpYnZp != nullptr)
				Insert(data, leaf->XpYnZp);
			else
			{
				leaf->XpYnZp = newOctreeNode(data);
			}
		}
		else if (data.isXpYnZn(leaf->data))
		{
			if (leaf->XpYnZn != nullptr)
				Insert(data, leaf->XpYnZn);
			else
			{
				leaf->XpYnZn = newOctreeNode(data);
			}
		}
		else if (data.isXnYpZp(leaf->data))
		{
			if (leaf->XnYpZp != nullptr)
				Insert(data, leaf->XnYpZp);
			else
			{
				leaf->XnYpZp = newOctreeNode(data);
			}

		}
		else if (data.isXnYpZn(leaf->data))
		{
			if (leaf->XnYpZn != nullptr)
				Insert(data, leaf->XnYpZn);
			else
			{
				leaf->XnYpZn = newOctreeNode(data);
			}
		}
		else if (data.isXnYnZp(leaf->data))
		{
			if (leaf->XnYnZp != nullptr)
				Insert(data, leaf->XnYnZp);
			else
			{
				leaf->XnYnZp = newOctreeNode(data);
			}
		}
		else if (data.isXnYnZn(leaf->data))
		{
			if (leaf->XnYnZn != nullptr)
				Insert(data, leaf->XnYnZn);
			else
			{
				leaf->XnYnZn = newOctreeNode(data);
			}
		}
	}
	OctreeNode* Search(vec3 data, OctreeNode* leaf)
	{
		if (leaf != nullptr)
		{
			if (data == leaf->data)
				return leaf;
			if (data.isXpYpZp(leaf->data))
				return Search(data, leaf->XpYpZp);
			else if (data.isXpYpZn(leaf->data))
				return Search(data, leaf->XpYpZn);
			else if (data.isXpYnZp(leaf->data))
				return Search(data, leaf->XpYnZp);
			else if (data.isXpYnZn(leaf->data))
				return Search(data, leaf->XpYnZn);
			else if (data.isXnYpZp(leaf->data))
				return Search(data, leaf->XnYpZp);
			else if (data.isXnYpZn(leaf->data))
				return Search(data, leaf->XnYpZn);
			else if (data.isXnYnZp(leaf->data))
				return Search(data, leaf->XnYnZp);
			else if (data.isXnYnZn(leaf->data))
				return Search(data, leaf->XnYnZn);
		}
		else
			return nullptr;
	}
};
