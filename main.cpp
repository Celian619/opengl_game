#include <iostream>
//include glad before GLFW to avoid header conflict or define "#define GLFW_INCLUDE_NONE"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>

#include "camera.h"
#include "plane.h"
#include "shader.h"
#include "object.h"
#include "utils.h"
#include "particles.h"

Camera camera(glm::vec3(0.0, 2.0, 5.0));
Plane plane(glm::vec3(-400.0f, 12.0f, -982.0f));
const int WINDOWS_WIDTH = 1000;
const int WINDOWS_HEIGHT = 1000;
double lastX = WINDOWS_WIDTH / 2.0;
double lastY = WINDOWS_HEIGHT / 2.0;
double sensibilityMouse = 0.3;
bool firstMouse = true;
#include "callbacks.h" //those variables are used in callbacks


int main(int argc, char* argv[]){

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
	GLFWwindow* window = glfwCreateWindow(WINDOWS_WIDTH, WINDOWS_HEIGHT, "Lava planet", nullptr, nullptr);
	if (window == NULL){
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window\n");
	}

	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);//TODO change ot GLFW_CURSOR_DISABLED
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//load openGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Failed to initialize GLAD");
	
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
	Shader lightShader(PATH_TO_SHADERS"/LIGHT.vert", PATH_TO_SHADERS"/LIGHT.frag");
    std::cout << "Light shaders loaded" << std::endl;

	std::cout << "Loading cube map shader" << std::endl;
	Shader cubeMapShader(PATH_TO_SHADERS"/CUBE_MAP.vert", PATH_TO_SHADERS"/CUBE_MAP.frag");
    std::cout << "Cube map shaders loaded" << std::endl;

	std::cout << "Loading particle shader" << std::endl;
	Shader particleShader(PATH_TO_SHADERS"/PARTICLE.vert", PATH_TO_SHADERS"/PARTICLE.frag");
    std::cout << "Particle shaders loaded" << std::endl;
	
	char pathPlane[] = PATH_TO_OBJECTS "/futuristic_combat_jet.dae";
	char pathCube[] = PATH_TO_OBJECTS "/cube.obj";
	char pathGround[] = PATH_TO_OBJECTS "/ground2.fbx";
	char pathCity[] = PATH_TO_OBJECTS "/Sci-fi Tropical city.obj";

	Object particleObject(pathCube);
	particleObject.makeObject(particleShader);
	Particles particles(&particleShader, &particleObject);
	plane.particles = &particles;

	Object planeObj(pathPlane);
	planeObj.makeObject(lightShader);
	glm::mat4 modelPlane = glm::mat4(1.0f);
	modelPlane = glm::scale(modelPlane, glm::vec3(0.2f, 0.2f, 0.2f));
	modelPlane = glm::rotate(modelPlane, (float) glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelPlane = glm::rotate(modelPlane, (float) glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		
	
	Object city(pathCity);
	city.makeObject(lightShader);
	glm::mat4 modelCity = glm::mat4(1.0f);
	modelCity = glm::rotate(modelCity, (float) glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelCity = glm::translate(modelCity, glm::vec3(0.0f, -30.0f, 0.0f));
	glm::mat4 inverseModelCity = glm::transpose( glm::inverse(modelCity));
	
	glfwPollEvents();//Avoid window not responding during boot

	Object ground(pathGround);
	ground.makeObject(lightShader);

	glm::mat4 modelGround = glm::mat4(1.0f);
	modelGround = glm::scale(modelGround, glm::vec3(1500.0f, 3000.0f, 1500.0f));
	modelGround = glm::rotate(modelGround, (float) glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	modelGround = glm::translate(modelGround, glm::vec3(0.0f, 0.0f, -0.16f));//ground to zero
	glm::mat4 inverseModelGround = glm::transpose( glm::inverse(modelGround));

	glfwPollEvents();//Avoid window not responding during boot

	Object cubeMap(pathCube);
	cubeMap.makeObject(cubeMapShader);

	glm::vec3 light_pos = glm::vec3(0.0f, 0.1f, 0.0f);

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

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	float ambientDay = 0.5;
	float ambientNight = 0.2;
	float diffuseDay = 0.5;
	float diffuseNight = 0.2;
	float specularDay = 0.5;
	float specularNight = 0.5;

	//Rendering
	lightShader.use();
	lightShader.setFloat("light.ambient_strength", ambientDay);
	lightShader.setFloat("light.diffuse_strength", diffuseDay);
	lightShader.setFloat("light.specular_strength", specularDay);
	lightShader.setFloat("light.constant", 1.0);//no attenuation for sun light
	lightShader.setFloat("light.linear", 0);
	lightShader.setFloat("light.quadratic", 0);


	//Create the day sky cubemap texture
	GLuint dayCubeMapTexture;
	std::string pathToDayCubeMap = PATH_TO_TEXTURES "/cubemaps/cloudsv2/";
    genCubemapTexture(&dayCubeMapTexture, pathToDayCubeMap);

	GLuint nightCubeMapTexture;
	std::string pathToNightCubeMap = PATH_TO_TEXTURES "/cubemaps/yokohama3/";
    genCubemapTexture(&nightCubeMapTexture, pathToNightCubeMap);

	//start to record mouse movement when everything is loaded
	glfwPollEvents();
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, mouse_scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

    bool lastFrameDay = true;
    int timeSlowdown = 20;

    double lastTime = glfwGetTime();
	
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		processInput(window);
		plane.updateState();
		camera.updateCameraVectors(plane.yaw);
		camera.updatePosition(plane.position);
		view = camera.GetViewMatrix();
		
		double now = glfwGetTime();
		float dt = now - lastTime;
		lastTime = now;
        now =  (now + 3.14* timeSlowdown) / timeSlowdown;//add constant to start at night
        
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lightShader.use();
		lightShader.setMatrix4("V", view);
		lightShader.setMatrix4("P", perspective);
		lightShader.setVector3f("u_view_pos", camera.Position);

		
		float sinTime = std::sin(now);

		auto delta = light_pos +  glm::vec3(5000.0 * std::cos(now), 5000.0 * sinTime, 0.0);//Put the sun far away

		float ambient = ambientDay;
        float diffuse = diffuseDay;
		float specular = specularDay;
        if(sinTime < 0){//night
			delta = glm::vec3(5000.0* 0.75, 0.0, 5000.0* 1);//position of moon
			ambient = ambientNight;
			diffuse = diffuseNight;
			specular = specularNight;
		}else if(sinTime < 0.2f){//sunset or sunrise
			float sunProgress = sinTime / 0.2f;
            diffuse = diffuseNight +  sunProgress * (diffuseDay - diffuseNight);
			ambient = ambientNight +  sunProgress * (ambientDay - ambientNight);
			specular = specularNight +  sunProgress * (specularDay - specularNight);
		}
		
		lightShader.setVector3f("light.light_pos", delta);
		lightShader.setFloat("light.specular_strength", specular);
		lightShader.setFloat("light.ambient_strength",  ambient);
	    lightShader.setFloat("light.diffuse_strength", diffuse);

		//std::cout << plane.position.x << ":" << plane.position.y << ":" << plane.position.z << std::endl;
		
		
        lightShader.setMatrix4("M", modelCity);
		lightShader.setMatrix4("itM", inverseModelCity);
		city.draw();
		

        lightShader.setMatrix4("M", modelGround);
		lightShader.setMatrix4("itM", inverseModelGround);
		ground.draw();

		glm::mat4 planeModelMatrix = plane.getModelMatrix();
		planeModelMatrix =  planeModelMatrix * modelPlane;
		
		glm::mat4 inverseModelAvion = glm::transpose( glm::inverse(planeModelMatrix));

		lightShader.setMatrix4("M", planeModelMatrix);
		lightShader.setMatrix4("itM", inverseModelAvion);
        planeObj.draw();


		//Draw particles (laser)
		particleShader.use();
		particleShader.setMatrix4("V", view);
		particleShader.setMatrix4("P", perspective);
		particles.update(dt);
		particles.draw();
		
		//now, draw the cubemap
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

		glDepthFunc(GL_LESS);
        
		fps(now);
		glfwSwapBuffers(window);
	}

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
