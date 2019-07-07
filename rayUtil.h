#ifndef RAYUTIL_H
#define RAYUTIL_H

#define __EPSILON 0.00001f

#include<GLAD\glad.h>
#include<GLFW\glfw3.h>
#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<glm\gtc\type_ptr.hpp>
#include<glm\gtc\constants.hpp>

#include<iostream>
#include<string>
#include "shader.h"

namespace RayUtil
{
	void renderRay(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::mat4 view, glm::mat4 model, glm::mat4 projection, Shader& shader)
	{
		//Render the ray (debug purposes)
		glm::vec3 vertices[] = { rayOrigin, rayOrigin + rayDir };
		unsigned int VAO, VBO;
		glGenBuffers(1, &VBO);
		glGenVertexArrays(1, &VAO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //position vertex attribute
		glEnableVertexAttribArray(0);

		shader.use();
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		glDrawArrays(GL_LINES, 0, 2);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//Simple unoptimized ray checking algorithm
	bool basicRayCheck(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 rayOrigin, glm::vec3 rayDir)
	{
		float u, v, w; //barycentric coordinates

		//find triangle plane's normal
		glm::vec3 normal = glm::cross((v1 - v0), (v2 - v0));

		float denom = glm::dot(normal, normal);
		// finding point of intersection with triangular plane

		//check if parallel
		float NdotRayDirection = glm::dot(normal, rayDir);
		if (fabs(NdotRayDirection) < __EPSILON) // almost 0 
			return false;

		if (glm::dot(rayDir, normal) > 0)
			return false; //we've hit a backface

		// compute d parameter using equation 2
		float planeDistance = glm::dot(normal, v0);

		//find intersection distance
		float t = -(glm::dot(normal, rayOrigin) + planeDistance) / NdotRayDirection;
		// check if the triangle is in behind the ray
		if (t < 0) return false;

		//find intersection
		glm::vec3 P = rayOrigin + t * rayDir;

		//Testing if ray is inside triangle
		glm::vec3 C; // vector perpendicular to triangle's plane 

		//edge 0
		C = cross((v1 - v0), (P - v0));
		if (glm::dot(normal, C) < 0) return false;

		//edge 1
		C = cross((v2 - v1), (P - v1));
		if ((u = glm::dot(normal, C)) < 0) return false;

		// edge 2
		C = glm::cross((v0 - v2), (P - v2));
		if ((v = glm::dot(normal, C)) < 0) return false;

		u /= denom;
		v /= denom;
		w = 1 - u - v;

		return true; // this ray hits the triangle 
	}

	//Möller-Trumbore ray intersection algorithm
	bool MTRayCheck(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 rayOrigin, glm::vec3 rayDir, float& t)
	{
		glm::vec3 v0v1 = v1 - v0;
		glm::vec3 v0v2 = v2 - v0;
		glm::vec3 pvec = glm::cross(rayDir, v0v2);
		float det = glm::dot(v0v1, pvec); //calculating determinant

		//if its negative, backface, if its near 0 parallel
		if (det < __EPSILON)
			return false;

		float invDet = 1 / det;

		glm::vec3 tvec = rayOrigin - v0;
		float u = glm::dot(tvec, pvec) * invDet;
		if (u < 0 || u > 1)
			return false; //barycentric coords, if these conditions are true, point is outside of tri

		glm::vec3 qvec = glm::cross(tvec, v0v1);
		float v = glm::dot(rayDir, qvec) * invDet;
		if (v < 0 || u + v > 1)
			return false; //same as above, different coord

		t = glm::dot(v0v2, qvec) * invDet; //will use later
		if (t < 0) //intersection is "behind" ray
			return false;

		return true;
	}
}


#endif