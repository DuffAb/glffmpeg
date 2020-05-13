#ifndef _VR_GLFW_EVENT_H_
#define _VR_GLFW_EVENT_H_

// ����GLEW�� ���徲̬����
#define GLEW_STATIC
#include "GL/glew.h"

// ����GLFW��
#include "GLFW/glfw3.h"

bool keys[1024];		// ���������¼
// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// ���û�����ESC��,��������window���ڵ�WindowShouldClose����Ϊtrue
	// �ر�Ӧ�ó���
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
		glfwSetWindowShouldClose(window, GL_TRUE); // �رմ���
	}
}
#endif // !_VR_GLFW_EVENT_H_

