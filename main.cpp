#include <iostream>
#include <cstdlib>
#include <time.h>
//include glad before GLFW to avoid header conflict or define "#define GLFW_INCLUDE_NONE"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <map>

#include "camera.h"
#include "shader.h"
#include "object.h"
#include "utils.h"


const int width = 1000;
const int height = 1000;


void processInput(GLFWwindow* window);

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
double lastX = width / 2.0;
double lastY = height / 2.0;
double sensibilityMouse = 0.5;
bool firstMouse = true;

#ifndef NDEBUG
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}
#endif

Camera camera(glm::vec3(0.0, 0.0, 0.1));


struct Particle {
    glm::vec3 Position, Velocity;
    glm::vec4 Color;
    float     Life;
  
    Particle() 
      : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};


int main(int argc, char* argv[])
{
	std::cout << "Welcome to exercice 8: " << std::endl;
	std::cout << "Load a cubemap/skybox \n"
		"Load a cubemap and use it in your scene\n"
		"\n";


	//Boilerplate
	//Create the OpenGL context 
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialise GLFW \n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
	//create a debug context to help with Debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif


	//Create the window
	GLFWwindow* window = glfwCreateWindow(width, height, "Exercise 08", nullptr, nullptr);
	if (window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window\n");
	}

	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);//TODO change ot GLFW_CURSOR_DISABLED

	//load openGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Failed to initialize GLAD");
	}

	glEnable(GL_DEPTH_TEST);

#ifndef NDEBUG
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif

    std::cout << "Loading light shaders" << std::endl;
	Shader shader(PATH_TO_SHADERS"/LIGHT.vert", PATH_TO_SHADERS"/LIGHT.frag");

    std::cout << "Light shaders loaded" << std::endl;
	//Compile the shader for the cube map
	std::cout << "Loading cube map shader" << std::endl;
	Shader cubeMapShader(PATH_TO_SHADERS"/CUBE_MAP.vert", PATH_TO_SHADERS"/CUBE_MAP.frag");
    std::cout << "Cube map shaders loaded" << std::endl;

	char path[] = PATH_TO_OBJECTS "/jet.obj";
	Object plane(path);
	plane.makeObject(shader, false);
	
	//Load the cube model and make the model
	char pathCube[] = PATH_TO_OBJECTS "/cube.obj";


	//reference cube
	Object cube(pathCube);
	cube.makeObject(shader);

	Object cubeMap(pathCube);
	cubeMap.makeObject(cubeMapShader);

	double prev = 0;
	int deltaFrame = 0;
	//fps function
	auto fps = [&](double now) {
		double deltaTime = now - prev;
		deltaFrame++;
		if (deltaTime > 0.5) {
			prev = now;
			const double fpsCount = (double)deltaFrame / deltaTime;
			deltaFrame = 0;
			std::cout << "\r FPS: " << fpsCount;
			std::cout.flush();
		}
	};


	glm::vec3 light_pos = glm::vec3(0.0f, 0.1f, 0.0f);
	glm::mat4 modelAvion = glm::mat4(1.0f);
	modelAvion = glm::translate(modelAvion, glm::vec3(0.0f, -2.0f, 7.0f));
	modelAvion = glm::scale(modelAvion, glm::vec3(1.0f, 1.0f, 1.0f));

	glm::mat4 modelCube = glm::mat4(1.0f);
	modelCube = glm::translate(modelCube, glm::vec3(0.0f, 0.0f, 0.0f));
	modelCube = glm::scale(modelCube, glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 inverseModelCube = glm::transpose( glm::inverse(modelCube));

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	float ambient = 0.3;//TODO set to 0.1 to have realistic night
	float diffuse = 0.5;
	float specular = 0.8;

	glm::vec3 materialColor = glm::vec3(0.5f,0.5f,0.5f);

	//Rendering

	shader.use();
	shader.setVector3f("materialColor", materialColor);
	shader.setFloat("light.ambient_strength", ambient);
	shader.setFloat("light.diffuse_strength", diffuse);
	shader.setFloat("light.specular_strength", specular);
	shader.setFloat("light.constant", 1.0);
	shader.setFloat("light.linear", 0);
	shader.setFloat("light.quadratic", 0);


	
	//Create the day sky cubemap texture
	GLuint dayCubeMapTexture;
	std::string pathToDayCubeMap = PATH_TO_TEXTURE "/cubemaps/clouds/";
    genCubemapTexture(&dayCubeMapTexture, pathToDayCubeMap);

	GLuint nightCubeMapTexture;
	std::string pathToNightCubeMap = PATH_TO_TEXTURE "/cubemaps/yokohama3/";
    genCubemapTexture(&nightCubeMapTexture, pathToNightCubeMap);


    // initialize random seed:
    srand(time(NULL));

    bool lastFrameDay = true;
    int timeSlowdown = 2;

    double lastTime = glfwGetTime() / timeSlowdown;
	
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		view = camera.GetViewMatrix();
		glfwPollEvents();
		double now = glfwGetTime();
        now =  now / timeSlowdown;//TODO change to slow/speed time 0.5;//
        float dt = now - lastTime;
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		
		shader.setMatrix4("V", view);
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_view_pos", camera.Position);

		
		auto delta = light_pos +  glm::vec3(1000.0 * std::cos(now), 1000.0 * std::sin(now), 0.0);//Put the sun far away

		shader.setVector3f("light.light_pos", delta);

	    shader.setFloat("light.ambient_strength",  ambient);
	    shader.setFloat("light.specular_strength", delta.y > 0 ? specular * std::sin(now) : 0);

        auto diffuse_str = diffuse;
        if(std::sin(now) < 0.2)
            diffuse_str *= std::pow(1 + std::max(std::sin(now)-0.2, -1.0), 2);
	    shader.setFloat("light.diffuse_strength", diffuse_str);

		
		

        shader.setMatrix4("M", modelCube);
		shader.setMatrix4("itM", inverseModelCube);
		cube.draw();

		glm::vec3 posPlane = camera.Position + camera.Front * 5.0f;
		//glm::mat4 currModelAvion = glm::translate(modelAvion, posPlane);

		glm::mat4 currModelAvion  = glm::rotate(modelAvion, (float) std::sin(now), glm::vec3(0.0f, 0.0f, 1.0f));
		//currModelAvion = glm::rotate(currModelAvion, glm::radians(camera.Yaw - 90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		glm::mat4 inverseModelAvion = glm::transpose( glm::inverse(currModelAvion));
		//lastPlanePos = camera.Position + camera.Front;

		shader.setMatrix4("V", glm::lookAt(glm::vec3(0.0,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f), glm::vec3(0.0f,1.0f,0.0f)));
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_view_pos", glm::vec3(0.0f,0.0f,0.0f));

		 shader.setMatrix4("M", currModelAvion);
		shader.setMatrix4("itM", inverseModelAvion);
        plane.draw();

		glDepthFunc(GL_LEQUAL);
		//Use the shader for the cube map
		cubeMapShader.use();
		//Set the relevant uniform
        cubeMapShader.setVector3f("light_pos", delta);//To print the sun hallo
		cubeMapShader.setFloat("timeOfDay", std::sin(now));
		cubeMapShader.setMatrix4("V", view);
		cubeMapShader.setMatrix4("P", perspective);
		cubeMapShader.setInteger("cubemapTexture", 0);
		

        glActiveTexture(GL_TEXTURE0);
		// bind the texture for the cubemap
        if(std::sin(now) > 0){//day
            glBindTexture(GL_TEXTURE_CUBE_MAP, dayCubeMapTexture);
        }else{//night
            glBindTexture(GL_TEXTURE_CUBE_MAP, nightCubeMapTexture);

        }
		//Draw the cubemap
		cubeMap.draw();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthFunc(GL_LESS);
        
        
		fps(now);
		glfwSwapBuffers(window);
	}

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}




void processInput(GLFWwindow* window) {
	//3. Use the cameras class to change the parameters of the camera
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){//LEFT
		//camera.ProcessKeyboardMovement(LEFT, 0.1);

	}
		
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(RIGHT, 0.1);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(FORWARD, 0.1);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(BACKWARD, 0.1);

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(1, 0.0, 1);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(-1, 0.0, 1);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, 1.0, 1);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, -1.0, 1);


}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){
	double xpos = xposIn;
	double ypos = yposIn;

	//std::cout << xpos << ":" << "ypos" << std::endl;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement((float)(xoffset* sensibilityMouse), (float) (yoffset* sensibilityMouse));
}


