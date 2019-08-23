/*
Title: Basic Ray Tracer
File Name: Main.cpp
Copyright � 2015, 2019
Original authors: Brockton Roth, Niko Procopi
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;

// These are your uniform variables.
GLuint eye_loc;		// Specifies camera location
// The rays are the four corner rays of the camera view. See: https://camo.githubusercontent.com/21a84a8b21d6a4bc98b9992e8eaeb7d7acb1185d/687474703a2f2f63646e2e6c776a676c2e6f72672f7475746f7269616c732f3134313230385f676c736c5f636f6d707574652f726179696e746572706f6c6174696f6e2e706e67
GLuint ray00;
GLuint ray01;
GLuint ray10;
GLuint ray11;

// A variable used to describe the position of the camera.
glm::vec3 cameraPos;

// A reference to our window.
GLFWwindow* window;

// Variables you will need to calculate FPS.
int tempFrame = 0;
int totalFrame = 0;
double dtime = 0.0;
double timebase = 0.0;
double totalTime = 0.0;
int fps = 0;

int width = 800;
int height = 600;

// This function takes in variables that define the perspective view of the camera, then outputs the four corner rays of the camera's view.
// It takes in a vec3 eye, which is the position of the camera.
// It also takes vec3 center, the position the camera's view is centered on.
// Then it will takes a vec3 up which is a vector that defines the upward direction. (So if you point it down, the camera view will be upside down.)
// Then it takes a float defining the verticle field of view angle. It also takes a float defining the ratio of the screen (in this case, 800/600 pixels).
// The last four parameters are actually just variables for this function to output data into. They should be pointers to pre-defined vec4 variables.
// For a visual reference, see this image: https://camo.githubusercontent.com/21a84a8b21d6a4bc98b9992e8eaeb7d7acb1185d/687474703a2f2f63646e2e6c776a676c2e6f72672f7475746f7269616c732f3134313230385f676c736c5f636f6d707574652f726179696e746572706f6c6174696f6e2e706e67
void calcCameraRays(glm::vec3 eye, glm::vec3 center, glm::vec3 up, float fov, float ratio)
{
	// Grab a ray from the camera position toward where the camera is to be centered on.
	glm::vec3 centerRay = center - eye;

	// w: Vector from center toward eye
	// u: Vector pointing directly right relative to the camera.
	// v: Vector pointing directly upward relative to the camera.

	// Create a w vector which is the opposite of that ray.
	glm::vec3 w = -centerRay;

	// Get the rightward (relative to camera) pointing vector by crossing up with w.
	glm::vec3 u = glm::cross(up, w);

	// Get the upward (relative to camera) pointing vector by crossing the rightward vector with your w vector.
	glm::vec3 v = glm::cross(w, u);

	// We create these two helper variables, as when we rotate the ray about it's relative Y axis (v), we will then need to rotate it about it's relative X axis (u).
	// This means that u has to be rotated by v too, otherwise the rotation will not be accurate. When the ray is rotated about v, so then are it's relative axes.
	glm::vec4 uRotateLeft = glm::vec4(u, 1.0f) * glm::rotate(glm::mat4(), glm::radians(-fov * ratio / 2.0f), v);
	glm::vec4 uRotateRight = glm::vec4(u, 1.0f) * glm::rotate(glm::mat4(), glm::radians(fov * ratio / 2.0f), v);

	// Now we simply take the ray and rotate it in each direction to create our four corner rays.
	glm::vec4 r00 = glm::vec4(centerRay, 1.0f) * glm::rotate(glm::mat4(), glm::radians(-fov * ratio / 2.0f), v) * glm::rotate(glm::mat4(), glm::radians(fov / 2.0f), glm::vec3(uRotateLeft));
	glm::vec4 r01 = glm::vec4(centerRay, 1.0f) * glm::rotate(glm::mat4(), glm::radians(-fov * ratio / 2.0f), v) * glm::rotate(glm::mat4(), glm::radians(-fov / 2.0f), glm::vec3(uRotateLeft));
	glm::vec4 r10 = glm::vec4(centerRay, 1.0f) * glm::rotate(glm::mat4(), glm::radians(fov * ratio / 2.0f), v) * glm::rotate(glm::mat4(), glm::radians(fov / 2.0f), glm::vec3(uRotateRight));
	glm::vec4 r11 = glm::vec4(centerRay, 1.0f) * glm::rotate(glm::mat4(), glm::radians(fov * ratio / 2.0f), v) * glm::rotate(glm::mat4(), glm::radians(-fov / 2.0f), glm::vec3(uRotateRight));

	// Now set the uniform variables in the shader to match our camera variables (cameraPos = eye, then four corner rays)
	glUniform3f(eye_loc, eye.x, eye.y, eye.z);
	glUniform3f(ray00, r00.x, r00.y, r00.z);
	glUniform3f(ray01, r01.x, r01.y, r01.z);
	glUniform3f(ray10, r10.x, r10.y, r10.z);
	glUniform3f(ray11, r11.x, r11.y, r11.z);
}

// This function runs every frame
void renderScene()
{
	// Used for FPS
	dtime = glfwGetTime();
	totalTime = dtime;

	// Every second, basically.
	if (dtime - timebase > 1)
	{
		// Calculate the FPS and set the window title to display it.
		fps = tempFrame / (dtime - timebase);
		timebase = dtime;
		tempFrame = 0;

		std::string s = "FPS: " + std::to_string(fps) + " Frame: " + std::to_string(totalFrame);

		glfwSetWindowTitle(window, s.c_str());
	}

	// To rotate the camera at the same speed
	// regardless of Frame Rate, use this code
	/*cameraPos = glm::vec3(
		8.0f * cos(totalTime),
		8.0f, 
		8.0f * sin(totalTime)
	);*/

	// To rotate the camera at a speed that depends
	// on FPS (high fps makes camera spin fast,
	// low fps makes camera spin slow), use this code
	cameraPos = glm::vec3(
		8.0f * cos((float)totalFrame / 60),
		8.0f,
		8.0f * sin((float)totalFrame / 60)
	);

	// Call the function we created to calculate the corner rays.
	// We use the camera position, the focus position, and the up direction (just like glm::lookAt)
	// We use Field of View, and aspect ratio (just like glm::perspective)
	calcCameraRays(cameraPos, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 45.0f, (float)width / height);

	// Notice, we do not use glClear or glClearColor
	// This is because, every pixel will be redrawn
	// by the fragment shader anyways, so clearing
	// it would only be an extra step

	// We procedurally generate a quad in the vertex shader.
	// No VAO or VBO needed, just like previous tutorials


	// this draws one quad that covers the whole screen
	// which allows the rasterizer to activate every
	// pixel in the fragment shader, just like deferred
	// rendering tutorials
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// help us keep track of FPS
	tempFrame++;
	totalFrame++;
}

// This method reads the text from a file.
// Realistically, we wouldn't want plain text shaders hardcoded in, we'd rather read them in from a separate file so that the shader code is separated.
std::string readShader(std::string fileName)
{
	std::string shaderCode;
	std::string line;

	// We choose ifstream and std::ios::in because we are opening the file for input into our program.
	// If we were writing to the file, we would use ofstream and std::ios::out.
	std::ifstream file(fileName, std::ios::in);

	// This checks to make sure that we didn't encounter any errors when getting the file.
	if (!file.good())
	{
		std::cout << "Can't read file: " << fileName.data() << std::endl;

		// Return so we don't error out.
		return "";
	}

	// ifstream keeps an internal "get" position determining the location of the element to be read next
	// seekg allows you to modify this location, and tellg allows you to get this location
	// This location is stored as a streampos member type, and the parameters passed in must be of this type as well
	// seekg parameters are (offset, direction) or you can just use an absolute (position).
	// The offset parameter is of the type streamoff, and the direction is of the type seekdir (an enum which can be ios::beg, ios::cur, or ios::end referring to the beginning,
	// current position, or end of the stream).
	file.seekg(0, std::ios::end);					// Moves the "get" position to the end of the file.
	shaderCode.resize((unsigned int)file.tellg());	// Resizes the shaderCode string to the size of the file being read, given that tellg will give the current "get" which is at the end of the file.
	file.seekg(0, std::ios::beg);					// Moves the "get" position to the start of the file.

	// File streams contain two member functions for reading and writing binary data (read, write). The read function belongs to ifstream, and the write function belongs to ofstream.
	// The parameters are (memoryBlock, size) where memoryBlock is of type char* and represents the address of an array of bytes are to be read from/written to.
	// The size parameter is an integer that determines the number of characters to be read/written from/to the memory block.
	file.read(&shaderCode[0], shaderCode.size());	// Reads from the file (starting at the "get" position which is currently at the start of the file) and writes that data to the beginning
	// of the shaderCode variable, up until the full size of shaderCode. This is done with binary data, which is why we must ensure that the sizes are all correct.

	file.close(); // Now that we're done, close the file and return the shaderCode.

	return shaderCode;
}

// This method will consolidate some of the shader code we've written to return a GLuint to the compiled shader.
// It only requires the shader source code and the shader type.
GLuint createShader(std::string sourceCode, GLenum shaderType)
{
	// glCreateShader, creates a shader given a type (such as GL_VERTEX_SHADER) and returns a GLuint reference to that shader.
	GLuint shader = glCreateShader(shaderType);
	const char *shader_code_ptr = sourceCode.c_str(); // We establish a pointer to our shader code string
	const int shader_code_size = sourceCode.size();   // And we get the size of that string.

	// glShaderSource replaces the source code in a shader object
	// It takes the reference to the shader (a GLuint), a count of the number of elements in the string array (in case you're passing in multiple strings), a pointer to the string array
	// that contains your source code, and a size variable determining the length of the array.
	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader); // This just compiles the shader, given the source code.

	GLint isCompiled = 0;

	// Check the compile status to see if the shader compiled correctly.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		char infolog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, infolog);

		// Print the compile error.
		std::cout << "The shader failed to compile with the error:" << std::endl << infolog << std::endl;

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.

		// NOTE: I almost always put a break point here, so that instead of the program continuing with a deleted/failed shader, it stops and gives me a chance to look at what may
		// have gone wrong. You can check the console output to see what the error was, and usually that will point you in the right direction.
	}

	return shader;
}

// Initialization code
void init()
{
	glewExperimental = GL_TRUE;
	// Initializes the glew library
	glewInit();

	// Read in the shader code from a file.
	std::string vertShader = readShader("../Assets/VertexShader.glsl");
	std::string fragShader = readShader("../Assets/FragmentShader.glsl");

	// createShader consolidates all of the shader compilation code
	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	// A shader is a program that runs on your GPU instead of your CPU. In this sense, OpenGL refers to your groups of shaders as "programs".
	// Using glCreateProgram creates a shader program and returns a GLuint reference to it.
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);		// This attaches our vertex shader to our program.
	glAttachShader(program, fragment_shader);	// This attaches our fragment shader to our program.

	// This links the program, using the vertex and fragment shaders to create executables to run on the GPU.
	glLinkProgram(program);
	// End of shader and program creation

	// Tell our code to use the program
	glUseProgram(program);

	// This gets us a reference to the uniform variables in the vertex shader, which are called by the same name here as in the shader.
	// We're using these variables to define the camera. The eye is the camera position, and teh rays are the four corner rays of what the camera sees.
	// Only 2 parameters required: A reference to the shader program and the name of the uniform variable within the shader code.
	eye_loc = glGetUniformLocation(program, "eye");
	ray00 = glGetUniformLocation(program, "ray00");
	ray01 = glGetUniformLocation(program, "ray01");
	ray10 = glGetUniformLocation(program, "ray10");
	ray11 = glGetUniformLocation(program, "ray11");
}

void window_size_callback(GLFWwindow* window, int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, width, height);
}

int main(int argc, char **argv)
{
	// Initializes the GLFW library
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(width, height, "", nullptr, nullptr);

	// This allows us to resize the window when we want to
	glfwSetWindowSizeCallback(window, window_size_callback);

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	glfwSwapInterval(1);

	// Initializes most things needed before the main loop
	init();

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		// Remember, you're rendering to the back buffer, then once rendering is complete, you're moving the back buffer to the front so it can be displayed.
		glfwSwapBuffers(window);

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// After the program is over, cleanup your data!
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);

	// Frees up GLFW memory
	glfwTerminate();

	return 0;
}
