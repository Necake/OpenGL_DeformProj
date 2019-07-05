#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include<GLAD\glad.h>
#include<GLFW\glfw3.h>
#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<glm\gtc\type_ptr.hpp>
#include<ft2build.h>
#include FT_FREETYPE_H

#include<iostream>
#include<string>
#include<map>
#include "shader.h"

struct Character
{
	GLuint TextureID; //id of glyph texture
	glm::ivec2 Size;
	glm::ivec2 Bearing; //offset from baseline
	GLuint Advance;
};

class Text
{
public:
	Text(const char* fontPath, int index, int size, glm::vec3 color)
	{
		//Loading the text library, returns non-0 if there's an error
		if (FT_Init_FreeType(&ft))
			std::cout << "ERROR: Failed to initialize FreeType Library" << std::endl;
		//Loading the typeface
		if (FT_New_Face(ft, fontPath, 0, &face))
			std::cout << "ERROR: Failed to load font" << std::endl;
		//Setting font size
		FT_Set_Pixel_Sizes(face, index, size);
		
		this->color = color;

		//Text geometry setup
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		generateTypeFace();
	}

	void renderText(Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::mat4 projection)
	{
		s.use();
		s.setVec3("textColor", color);
		s.setMat4("projection", projection);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VAO);

		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = Characters[*c];

			GLfloat xpos = x + ch.Bearing.x * scale;
			GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			GLfloat w = ch.Size.x * scale;
			GLfloat h = ch.Size.y * scale;

			GLfloat vertices[6][4] = {
				{xpos, ypos + h,		0.0, 0.0},
				{xpos, ypos,			0.0, 1.0},
				{xpos + w, ypos,		1.0, 1.0},

				{xpos, ypos + h,		0.0, 0.0},
				{xpos + w, ypos,		1.0, 1.0},
				{xpos + w, ypos + h,	1.0, 0.0}
			};
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);
			x += (ch.Advance >> 6) * scale; //because advance 1/64 of a pixel
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

private:
	void generateTypeFace()
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		for (GLubyte c = 0; c < 128; c++)
		{
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR: Failed to load glyph " << c << std::endl;
				continue;
			}
			//Texture generation based on loaded glyph
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};
			Characters.insert(std::pair<GLchar, Character>(c, character));
		}
		FT_Done_Face(face);
		FT_Done_FreeType(ft);
	}
	FT_Library ft;
	FT_Face face;
	std::map<GLchar, Character> Characters;

	unsigned int VAO, VBO;
	glm::vec3 color;
};


#endif