//TODO: IZKOMENTARISI JEBENI KOD DEBILU

#include<GLAD\glad.h>
#include<GLFW\glfw3.h>
#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<glm\gtc\type_ptr.hpp>
#include<glm\gtc\constants.hpp>
#include<ft2build.h>
#include FT_FREETYPE_H

#include<iostream>
#include<string>
#include<fstream>
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "textRendering.h"
#include "rayUtil.h"
#include "optimalProjectile.h"
#include "optimalTarget.h"

//------------------------------------------------------------------------------------------------
//Function prototypes
//------------------------------------------------------------------------------------------------
GLFWwindow* setupWindow();
void setupCallbacks(GLFWwindow* window);
void loadTexture(unsigned int* texture, bool isRGBA, const char* path);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void setupStaticLights(Shader& targetShader, glm::vec3* lightPositions, glm::vec3 lightDiffuse);
unsigned int loadCubemap(vector<std::string> faces);

//------------------------------------------------------------------------------------------------
//Global variables
//------------------------------------------------------------------------------------------------
int windowWidth = 800, windowHeight = 600; //Window size
float deltaTime = 0.0f, lastFrame = 0.0f; //Time counter related stuff
float mousePitch = 0.0f, mouseYaw = 0.0f; //Mouse info
float linear = 0.045f, quadratic = 0.0075; //Light calculation constants

//------------------------------------------------------------------------------------------------
//Camera variables
//------------------------------------------------------------------------------------------------
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = windowWidth / 2.0f;
float lastY = windowHeight / 2.0f;
bool firstMouse = true;

//TODO: delete dis
float dentSpeed = 0.01f; bool started = false;

glm::vec3 rayPos = glm::vec3(0.01f, 2.0f, 0.01f);

int main()
{
	//------------------------------------------------------------------------------------------------
	//Initialization
	//------------------------------------------------------------------------------------------------

	//Window init
	GLFWwindow* window = setupWindow();

	//Loading opengl func pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD!" << std::endl;
		return -1;
	}

	//Initialize text font
	Text text("../OpenGL_DeformProj/arial.ttf", 0, 24, glm::vec3(0, 0, 0));

	//Setup function callbacks for input etc.
	setupCallbacks(window);

	//Setting up opengl state constants
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE); //enable culling
	glCullFace(GL_BACK); //tell opengl to cull back faces
	glFrontFace(GL_CCW); //set the front faces to be counter-clockwise winded

	ofstream FPSOutput; //for logging fps onto a csv file
	FPSOutput.open("fps.csv");

	//------------------------------------------------------------------------------------------------
	//Geometry and shader setup
	//------------------------------------------------------------------------------------------------
	Shader objShader("../OpenGL_DeformProj/objVert.vert", "../OpenGL_DeformProj/objFrag_noTex.frag");
	Shader projShader("../OpenGL_DeformProj/objVert.vert", "../OpenGL_DeformProj/objFrag_noTex.frag");
	
	Shader lightShader("../OpenGL_DeformProj/lightVert.vert", "../OpenGL_DeformProj/lightFrag.frag");
	Shader textShader("../OpenGL_DeformProj/textShader.vert", "../OpenGL_DeformProj/textShader.frag");
	Shader skyboxShader("../OpenGL_DeformProj/skybox.vert", "../OpenGL_DeformProj/skybox.frag");
	Shader rayShader("../OpenGL_DeformProj/ray.vert", "../OpenGL_DeformProj/ray.frag");

	//Raw geometry for lights and the skybox
	float vertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		// Front face
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		// Left face
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		// Right face
		 0.5f,  0.5f,  0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 // Bottom face
		 -0.5f, -0.5f, -0.5f,
		  0.5f, -0.5f, -0.5f,
		  0.5f, -0.5f,  0.5f,
		  0.5f, -0.5f,  0.5f,
		 -0.5f, -0.5f,  0.5f,
		 -0.5f, -0.5f, -0.5f,
		 // Top face
		 -0.5f,  0.5f, -0.5f,
		  0.5f,  0.5f,  0.5f,
		  0.5f,  0.5f, -0.5f,
		  0.5f,  0.5f,  0.5f,
		 -0.5f,  0.5f, -0.5f,
		 -0.5f,  0.5f,  0.5f
	};
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	//Array of positions for lights in the scene
	glm::vec3 lightPositions[] = {
		glm::vec3(3.0f,  1.0f,  10.0f),
		glm::vec3(11.0f, -15.0f, -20.0f),
		glm::vec3(-20.0f,  10.0f, -60.0f),
		glm::vec3(0.0f,  0.0f, -15.0f)
	};

	//Light (cube) geometry setup
	unsigned int lampVBO, lampVAO;
	glGenBuffers(1, &lampVBO);
	glGenVertexArrays(1, &lampVAO);

	glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(lampVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	//------------------------------------------------------------------------------------------------
	//Skybox setup
	//------------------------------------------------------------------------------------------------
	std::vector<std::string> skyBoxFaces = {
		"../../OpenGLAssets/Skybox/right.jpg",
		"../../OpenGLAssets/Skybox/left.jpg",
		"../../OpenGLAssets/Skybox/top.jpg",
		"../../OpenGLAssets/Skybox/bottom.jpg",
		"../../OpenGLAssets/Skybox/front.jpg",
		"../../OpenGLAssets/Skybox/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(skyBoxFaces);
	unsigned int skyVAO, skyVBO;
	glGenVertexArrays(1, &skyVAO);
	glGenBuffers(1, &skyVBO);
	glBindVertexArray(skyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	skyboxShader.use();
	skyboxShader.setInt("skyBox", 0);

	//------------------------------------------------------------------------------------------------
	//Target, lights, and projectile setup
	//------------------------------------------------------------------------------------------------
	glm::vec3 lightDiffuse = glm::vec3(0.66f, 0.86f, 0.97f);
	setupStaticLights(objShader, lightPositions, lightDiffuse);
	setupStaticLights(projShader, lightPositions, lightDiffuse);
	//Loading the target
	OctreeTarget target("../../OpenGLAssets/testModels/testPlaneOverkill.obj", 1.0f, 3.0f, 1);
	Octree sceneOctree(target.targetModel, target.boundingBoxSize* 0.5f, 3, 3, 3, target.boundingBoxSize, target.boundingBoxCenter + glm::vec3(0, 0.001f, 0));
	target.SetupTree(sceneOctree);
	objShader.setVec3("material.diffuse", target.targetModel.material.diffuse);
	objShader.setVec3("material.specular", target.targetModel.material.specular);
	//Loading the projectile
	//PointProjectile projectile(glm::vec3(0.0f, 3.05f, -2.20f), glm::vec3(0.0f, -0.03f, 0.005f));
	OctreePointProjectile octreeTester(rayPos, glm::vec3(0.0f, -0.03f, 0.0f));
	OctreeProjectile legitOctreeTester("../../OpenGLAssets/testModels/projectileConcave.obj", glm::vec3(0.0f, -0.003f, 0.0f));
	projShader.setVec3("material.diffuse", legitOctreeTester.projectileMesh.material.diffuse);
	projShader.setVec3("material.specular", legitOctreeTester.projectileMesh.material.specular);
	//Fps counter constants
	double lastFPSCheck = glfwGetTime();
	int currentFPS = 0;
	bool firstPass = true; 

	//------------------------------------------------------------------------------------------------
	//Main loop
	//------------------------------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window))
	{
		//Fps and deltaTime updating
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		//Input handling
		processInput(window);

		//State setting
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		//Clearing leftover data
		glClearColor(.1f, .1f, .1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		//Setting up the matrices
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 projection;
		view = camera.GetViewMatrix();
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

		objShader.use();
		objShader.setMat4("view", view);
		objShader.setMat4("projection", projection);
		objShader.setVec3("viewPos", camera.Position);
		objShader.setFloat("material.shininess", 32.0f); //the only thing left in the material not managed by textures
		projShader.use();
		projShader.setMat4("view", view);
		projShader.setMat4("projection", projection);
		projShader.setVec3("viewPos", camera.Position);
		projShader.setFloat("material.shininess", 32.0f); //the only thing left in the material not managed by textures
		//Dynamic light setup
		//objShader.setSpotLight("flashLight", camera.Position, camera.Front, lightDiffuse, 12.5f, 17.5f);

		//Setting up skybox
		glDepthMask(GL_FALSE);
		// draw skybox as last
		skyboxShader.use();
		// remove translation from the view matrix
		skyboxShader.setMat4("view", glm::mat4(glm::mat3(camera.GetViewMatrix())));
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);

		//Rendering light geometry
		glBindVertexArray(lampVAO);
		for (int i = 0; i < 4; i++)
		{
			lightShader.use();
			model = glm::mat4(1.0f);
			model = glm::scale(model, glm::vec3(.5f, .5f, .5f));
			model = glm::translate(model, lightPositions[i]);
			lightShader.setMat4("view", view);
			lightShader.setMat4("model", model);
			lightShader.setMat4("projection", projection);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Rendering the target
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		model = glm::mat4(1.0f);
		//model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		//model = glm::rotate(model, (float)glm::radians(30.0f), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));
		//model = glm::translate(model, glm::vec3(0.5f, -3.2f, 0.2f));
		objShader.use();
		target.model = model;
		target.Draw(objShader);

		/* NON OCTREE IMPLEMENTATION
		if (started) //When simulation is started(keystroke), start denting the target
		{
			FPSOutput << currentFPS << "\n";
			if (firstPass) //First pass is for setting up the models, all done in 1 frame lol
			{
				legitProjectile.ProcessRays(target, model);
				//legitProjectile.ProcessTarget(target, model);
				std::cout << "Target has been set up.\n";
				firstPass = false;
			}
			if (!legitProjectile.isDone)
			{
				legitProjectile.Update(target, 0.0167f, target.model);
			}
			else
			{
				started = false;
				FPSOutput.close();
			}
		}
		*/
		//Reset the model matrix and render the ray itself
		model = glm::mat4(1.0f);
		//projectile.RenderInfiniteRay(view, model, projection);
		projShader.use();
		projShader.setMat4("model", model);
		legitOctreeTester.model = model;
		legitOctreeTester.Draw(projShader);
		legitOctreeTester.RenderInfiniteRays(view, projection);

		//octreeTester.projectilePosition = rayPos;
		octreeTester.RenderRay(view, model, projection);
		if (started)
		{
			FPSOutput << currentFPS << "\n";
			octreeTester.Update(sceneOctree, target, 0.0167f, model);
			if (octreeTester.isDone)
			{
				started = false;
				FPSOutput.close();
			}
		}

		glm::mat4 textCanvas = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
		//if (octreeTester.CastRay(sceneOctree))
		//{
		//	text.renderText(textShader, "hit ray!", 0.0f, windowHeight - 24.0f, 1.0f, textCanvas);
		//}

		//Rendering text
		//glm::mat4 textCanvas = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
		//text.renderText(textShader, "FPS:" + std::to_string(currentFPS), 0.0f, windowHeight - 24.0f, 1.0f, textCanvas);
		//if (glfwGetTime() - lastFPSCheck > 0.3)
		//{ //Updating the fps meter every 1/3 of a second
		currentFPS = 1 / deltaTime;
		//}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

//------------------------------------------------------------------------------------------------
//Function definitions
//------------------------------------------------------------------------------------------------

//Does basic GLFW window setup
GLFWwindow* setupWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "bottom text", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create window!" << std::endl;
		return NULL;
	}
	glfwMakeContextCurrent(window);
	return window;
}

//Sets up function callbacks for things input and window resizing
void setupCallbacks(GLFWwindow* window)
{
	glViewport(0, 0, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
}

//Loading textures from external source with assumed wrapping parameters
void loadTexture(unsigned int* texture, bool isRGBA, const char* path)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChans;
	unsigned char* data = stbi_load(path, &width, &height, &nrChans, 0);
	if (data)
	{
		if (isRGBA)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "ERROR: Failed to load image data!";
	}
	stbi_image_free(data);
}

//Callback for resizing the window
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//Handles all the input
void processInput(GLFWwindow * window)
{
	//Utility block
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{ //Quitting the window on escape press
		glfwSetWindowShouldClose(window, true);
	}
	else if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
	{ //Wireframe mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS)
	{ //Shaded mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//Camera movement
	float camSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	//debug ray movement
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		started = true;
	}

	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		rayPos.y -= deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
	{
		rayPos.y += deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		rayPos.x += deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		rayPos.x -= deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		rayPos.z -= deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		rayPos.z += deltaTime;
	}

}

//Mouse movement handling
void mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

//Scroll wheel handling
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

//Static light setup for object shader, these get calculated before the actual render
void setupStaticLights(Shader& targetShader, glm::vec3* lightPositions, glm::vec3 lightDiffuse)
{
	targetShader.use();
	for (int i = 0; i < 4; i++)
	{
		targetShader.setPointLightAt("pointLights", i, lightPositions[i], lightDiffuse, linear, quadratic);
	}
	glm::vec3 sunDiffuse = glm::vec3(1.0f, 0.7f, 0.3f);
	glm::vec3 sunDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
	targetShader.setDirectionalLight("dirLight", sunDirection, sunDiffuse);
}

//Loads cubemap for the skybox
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
