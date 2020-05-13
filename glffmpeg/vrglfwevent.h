#ifndef _VR_GLFW_EVENT_H_
#define _VR_GLFW_EVENT_H_

// 引入GLEW库 定义静态链接
#define GLEW_STATIC
#include "GL/glew.h"

// 引入GLFW库
#include "GLFW/glfw3.h"

bool keys[1024];		// 按键情况记录
// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// 当用户按下ESC键,我们设置window窗口的WindowShouldClose属性为true
	// 关闭应用程序
	// std::cout << key << std::endl;
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE); // 关闭窗口
	}
}
#endif // !_VR_GLFW_EVENT_H_

