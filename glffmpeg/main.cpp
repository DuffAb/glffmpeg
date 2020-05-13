#include <vector>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include "vrglfwevent.h"
#include "vrshader.h"
#include "vrglrender.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vrmoviedecoder.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32sd.lib")
#pragma comment(lib, "avformat")
#pragma comment(lib, "avutil")
#pragma comment(lib, "avcodec")
#pragma comment(lib, "swscale")
#pragma comment(lib, "swresample")
#pragma comment(lib, "avdevice.lib")


std::string const vert_shader_source =
"#version 330\n"
"in vec3 position;\n"
"in vec2 uv;\n"
"uniform mat4 mvp;\n"
"out vec2 texCoord;\n"
"void main() {\n"
"	gl_Position = mvp * vec4(position, 1.0);\n"
"	texCoord = uv;\n"
"}\n";

std::string const frag_shader_source =
"#version 330\n"
"uniform sampler2D frameTex;\n"
"in vec2 texCoord;\n"
"out vec4 fragColor;\n"
"void main() {\n"
"	vec3 result = texture2D(frameTex, vec2(texCoord.x, 1.0 - texCoord.y)).rgb;\n"
"	fragColor = vec4(result, 1.0f);\n"
//"	fragColor = texture(frameTex, texCoord);\n"
"}\n";

std::string const vert_shader_source_rgb24 =
"#version 330\n"
"in vec3 position;\n"
"in vec2 uv;\n"
"uniform mat4 mvp;\n"
"out vec2 texCoord;\n"
"void main() {\n"
"	gl_Position = mvp * vec4(position, 1.0);\n"
"	texCoord = uv;\n"
"}\n";

std::string const frag_shader_source_rgb24 =
"#version 330\n"
"uniform sampler2D frameTex;\n"
"in vec2 texCoord;\n"
"out vec4 fragColor;\n"
"void main() {\n"
"	fragColor = texture(frameTex, texCoord);\n"
"}\n";

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
GLFWwindow* window;

int main(int argc, char* argv[]) 
{
	if (argc < 2) 
	{
		std::cout << "provide a filename" << std::endl;
		return -1;
	}

	vrmoviedecoder mdr;

	// open video
	int error = 0;
	//mdr.vr_movie_decoder_open_file(argv[1], &error);//播放视频
	mdr.vr_movie_decoder_open_screen(true);		//捕获屏幕
	mdr.vr_movie_decoder_init_avframe_rgb24();

	// open a window
	int width = mdr.vr_movie_decoder_width();
	int height = mdr.vr_movie_decoder_height();
	float aspect = (float)width / (float)height;
	int adj_width = aspect * 800;
	int adj_height = 800;
	// Init GLFW
	if (!glfwInit())	// 初始化glfw库
	{
		std::cout << "Error::GLFW could not initialize GLFW!" << std::endl;
		return -1;
	}
	
	// 开启OpenGL 3.3 core profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 创建窗口
	window = glfwCreateWindow(adj_width, adj_height, "Demo of gl ffmpeg", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// 创建的窗口的context指定为当前context
	glfwMakeContextCurrent(window);
	// 设置键盘回调函数
	glfwSetKeyCallback(window, key_callback);

	// 初始化GLEW 获取OpenGL函数
	glewExperimental = GL_TRUE;// 让glew获取所有拓展函数
	GLenum status = glewInit();
	if (status != GLEW_OK)
	{
		std::cout << "glew failed to init" << std::endl;
		glfwTerminate();
		return -1;
	}

	// initialize shaders
	vrshader shader(vert_shader_source_rgb24.c_str(), frag_shader_source_rgb24.c_str(), false);
	// initialize opengl
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_TEXTURE_2D);
	
	shader.vr_shader_bind();

	vrglrender render;
	render.vr_gl_render_new_quad();
	render.vr_gl_render_bind_shader(&shader);
	render.vr_gl_render_new_texture(width, height);
	// initialize renderable
	
	shader.vr_shader_update_uniform_1i("frameTex", 0);

	glm::mat4 mvp = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	shader.vr_shader_upload_matrix("mvp", glm::value_ptr(mvp));

	// run the application mainloop
	while (mdr.vr_movie_decoder_read_frame() && !glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();	// 处理例如鼠标 键盘等事件
		render.vr_gl_render_draw();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
}