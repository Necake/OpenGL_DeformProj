#ifndef MESH_H
#define MESH_H
//-------------------------------------------------------------------------------------
// Mesh class, contains a single mesh, which is a building block for loaded models
// Also contains all the details such as vertices, indices etc.
//-------------------------------------------------------------------------------------

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

struct Vertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;
	//ray distance from the other object
	float hitDistance;
	//is the vertex calculated?
	bool isInitialized = false;
	//is the current vertex colliding?
	bool isColliding = false;
};

struct Texture {
	unsigned int id;
	string type;
	string path;
};

class Mesh {
public:
	/*  Mesh Data  */
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	/*  Functions  */
	//Constructor
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, bool isDynamic)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		this->isDynamic = isDynamic;
		//Now that we have all the required data, set the vertex buffers and its attribute pointers.
		setupMesh();
	}

	//Render the mesh
	void Draw(Shader shader)
	{
		//Bind appropriate textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
			// retrieve texture number (the N in diffuse_textureN)
			string number;
			string name = textures[i].type;
			if (name == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (name == "texture_specular")
				number = std::to_string(specularNr++); // transfer unsigned int to stream
			else if (name == "texture_normal")
				number = std::to_string(normalNr++); // transfer unsigned int to stream
			else if (name == "texture_height")
				number = std::to_string(heightNr++); // transfer unsigned int to stream

													 // now set the sampler to the correct texture unit
			glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
			// and finally bind the texture
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		//Draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//Set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
	}

	//Updates single triangle in array buffer, given the first index of triangle verts
	void UpdateBufferTriangle(int firstIndex)
	{
		//vertices of the triangle that need to be updated
		Vertex first = this->vertices[indices[firstIndex]];
		Vertex second = this->vertices[indices[firstIndex + 1]];
		Vertex third = this->vertices[indices[firstIndex + 2]];

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		//passing the first, second, and third vertex into buffer
		glBufferSubData(GL_ARRAY_BUFFER, indices[firstIndex] * sizeof(Vertex), sizeof(Vertex), &first);
		glBufferSubData(GL_ARRAY_BUFFER, indices[firstIndex + 1] * sizeof(Vertex), sizeof(Vertex), &second);
		glBufferSubData(GL_ARRAY_BUFFER, indices[firstIndex + 2] * sizeof(Vertex), sizeof(Vertex), &third);
	}
	void UpdateBufferVertex(int firstIndex)
	{
		//vertex to be updated
		Vertex first = this->vertices[indices[firstIndex]];

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		//passing the vert into the buffer
		glBufferSubData(GL_ARRAY_BUFFER, indices[firstIndex] * sizeof(Vertex), sizeof(Vertex), &first);
	}
	void UpdateBufferVertexDirect(int index)
	{
		//vertex to be updated
		Vertex first = this->vertices[index];

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		//passing the vert into the buffer
		glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(Vertex), sizeof(Vertex), &first);
	}

private:
	/*  Render data  */
	unsigned int VBO, EBO;
	bool isDynamic;
	/*  Functions    */
	// initializes all the buffer objects/arrays
	void setupMesh()
	{
		// create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.
		//If the mesh is dynamic, it gets set up for dynamic drawing
		if (isDynamic)
		{
			std::cout << "dynamic mesh setup\n"; //TODO delet dis
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);
		}
		else //Otherwise, it gets set up for static drawing
		{
			std::cout << "static mesh setup\n"; //TODO delet dis
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		}
		

		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		// vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
		// vertex bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

		glBindVertexArray(0);
	}
};
#endif