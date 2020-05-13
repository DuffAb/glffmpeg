#ifndef _VR_GL_RENDER_H_
#define _VR_GL_RENDER_H_
//包含C++标准头文件
#include <list>
using namespace std;

// 引入GLEW库 定义静态链接
#define GLEW_STATIC
#include "GL/glew.h"

struct vr3dattributebuffer {
	const GLchar* name;
	GLuint vbo;
	size_t element_size;
	int vector_length;
};

class vrshader;
class vrglrender
{
public:
	vrglrender();
	~vrglrender();

public:
	void vr_gl_render_bind_shader(vrshader* shader);
	void vr_gl_render_new_quad();

	void vr_gl_render_new_texture(GLuint width, GLuint height);
	void vr_gl_render_draw();
	GLuint vao;
private:
	void vr_gl_render_init_buffers();
	void vr_gl_render_append_attribute_buffer(const char* name, size_t element_size, int vector_length, GLfloat* vertices);
	void vr_gl_render_upload_quad();

	
private:
	list<vr3dattributebuffer*> attribute_buffers;
	//GLuint vao;
	GLuint ebo;

	GLuint index_size;		//索引大小
	GLuint vertex_count;	//顶点个数

	GLenum draw_mode;		//绘制模式

	GLuint frame_tex;
};

#endif // !_VR_GL_RENDER_H_

