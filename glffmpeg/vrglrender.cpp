#include <string.h>
#include <stdlib.h>
#include "vrshader.h"
#include "vrglrender.h"

vrglrender::vrglrender()
{
}

vrglrender::~vrglrender()
{
	glDeleteVertexArrays(1, &vao);

	glDeleteBuffers(1, &ebo);
	glDeleteTextures(1, &frame_tex);
}

void vrglrender::vr_gl_render_bind_shader(vrshader* shader)
{
	shader->vr_shader_bind();
	list<vr3dattributebuffer*>::iterator it;
	for (it = this->attribute_buffers.begin(); it != this->attribute_buffers.end(); ++it)
	{
		struct vr3dattributebuffer* buf = *it;
		// ��VBO����
		glBindBuffer(GL_ARRAY_BUFFER, buf->vbo);

		GLint attrib_location = shader->vr_get_attribute_location(buf->name);

		if (attrib_location != -1) {
			// �趨��������ָ�� ����OpenGL����ν�����������
			glVertexAttribPointer(attrib_location, //index: ��ɫ����������ɫ���е�����λ��ֵ
				buf->vector_length, //size: ������ÿ��������Ҫ���µķ�����Ŀ
				GL_FLOAT,           //type: ���������ݵ����� ��GL_FLOAT��GL_BYTE��GL_INT ��
				GL_FALSE,           //normalized: ���ö��������ڴ洢ǰ�Ƿ���Ҫ��һ��   GL_FALSE:����Ҫ��һ��
				0, 0);              //stride:����Ԫ��֮��Ĵ�Сƫ��ֵ(byte)      pointer: ����Ӧ�ôӻ�������ĸ���ַ��ʼ��ȡ
			glEnableVertexAttribArray(attrib_location);//����������index����������Ķ�������
		}
		else {
			printf("could not find attribute %s in shader.", buf->name);
		}
	}
	glBindVertexArray(0);
}

void vrglrender::vr_gl_render_new_quad()
{
	vr_gl_render_init_buffers();
	vr_gl_render_upload_quad();
}

void vrglrender::vr_gl_render_new_texture(GLuint width, GLuint height)
{
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &frame_tex);
	glBindTexture(GL_TEXTURE_2D, frame_tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

void vrglrender::vr_gl_render_draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, frame_tex);
	glBindVertexArray(vao);
	//glDrawElements(draw_mode, index_size, GL_UNSIGNED_SHORT, 0);  //ע���������ݵ����ͣ�GLushort
	glDrawElements(draw_mode, index_size, GL_UNSIGNED_BYTE, 0); //ע���������ݵ����ͣ�GLubyte
	glBindVertexArray(0);
}

void vrglrender::vr_gl_render_init_buffers()
{
	// ��������VAO����
	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);
	// ����EBO����
	glGenBuffers(1, &this->ebo);
}

void vrglrender::vr_gl_render_append_attribute_buffer(const char* name, size_t element_size, int vector_length, GLfloat* vertices)
{
	struct vr3dattributebuffer* attrib_buffer = (struct vr3dattributebuffer*)malloc(sizeof(struct vr3dattributebuffer));

	attrib_buffer->name = name;
	attrib_buffer->element_size = element_size;
	attrib_buffer->vector_length = vector_length;
	// ����VBO����
	glGenBuffers(1, &attrib_buffer->vbo);
	// ��VBO����
	glBindBuffer(GL_ARRAY_BUFFER, attrib_buffer->vbo);
	// ����ռ䣬���Ͷ�������
	glBufferData(GL_ARRAY_BUFFER, this->vertex_count * attrib_buffer->vector_length * attrib_buffer->element_size, vertices, GL_STATIC_DRAW);

	this->attribute_buffers.push_back(attrib_buffer);//�� attrib_buffer ��ӵ� attribute_buffers(List)��
}

void vrglrender::vr_gl_render_upload_quad()
{
	float vertices[] = {
		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

	//��������
	GLfloat uvs[] = {
	   0.0f, 0.0f,	// 0
	   0.0f, 1.0f,	// 1
	   1.0f, 1.0f,	// 2
	   1.0f, 0.0f	// 3
	};

	// ��������
	const GLubyte indices[] = {
		0, 1, 2,  // ��һ��������
		0, 2, 3   // �ڶ���������
	};
	
	this->vertex_count = 4;	//����ĸ���
	this->draw_mode = GL_TRIANGLES;

	// ���������� ���������ύGPU
	vr_gl_render_append_attribute_buffer("position", sizeof(GLfloat), 3, vertices);
	vr_gl_render_append_attribute_buffer("uv", sizeof(GLfloat), 2, uvs);

	this->index_size = sizeof(indices);
	// ��EBO����
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	// ����ռ䣬������������
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->index_size, indices, GL_STATIC_DRAW);
}

