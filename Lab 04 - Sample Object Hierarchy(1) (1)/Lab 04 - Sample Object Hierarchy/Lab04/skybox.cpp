#include "skybox.h"

// adapted from opengl

std::vector<std::string> faces =
{
		"skybox/posx.jpg",
		"skybox/negx.jpg",
		"skybox/posy.jpg",
		"skybox/negy.jpg",
		"skybox/posz.jpg",
		"skybox/negz.jpg"

};

Skybox::Skybox() {
	texture = loadCubemap(faces);
}

GLfloat skyboxVertices[] = {
	// positions
	-50.0f,  50.0f, -50.0f,
	-50.0f, -50.0f, -50.0f,
	 50.0f, -50.0f, -50.0f,
	 50.0f, -50.0f, -50.0f,
	 50.0f,  50.0f, -50.0f,
	-50.0f,  50.0f, -50.0f,

	-50.0f, -50.0f,  50.0f,
	-50.0f, -50.0f, -50.0f,
	-50.0f,  50.0f, -50.0f,
	-50.0f,  50.0f, -50.0f,
	-50.0f,  50.0f,  50.0f,
	-50.0f, -50.0f,  50.0f,

	 50.0f, -50.0f, -50.0f,
	 50.0f, -50.0f,  50.0f,
	 50.0f,  50.0f,  50.0f,
	 50.0f,  50.0f,  50.0f,
	 50.0f,  50.0f, -50.0f,
	 50.0f, -50.0f, -50.0f,

	-50.0f, -50.0f,  50.0f,
	-50.0f,  50.0f,  50.0f,
	 50.0f,  50.0f,  50.0f,
	 50.0f,  50.0f,  50.0f,
	 50.0f, -50.0f,  50.0f,
	-50.0f, -50.0f,  50.0f,

	-50.0f,  50.0f, -50.0f,
	 50.0f,  50.0f, -50.0f,
	 50.0f,  50.0f,  50.0f,
	 50.0f,  50.0f,  50.0f,
	-50.0f,  50.0f,  50.0f,
	-50.0f,  50.0f, -50.0f,

	-50.0f, -50.0f, -50.0f,
	-50.0f, -50.0f,  50.0f,
	 50.0f, -50.0f, -50.0f,
	 50.0f, -50.0f, -50.0f,
	-50.0f, -50.0f,  50.0f,
	 50.0f, -50.0f,  50.0f
};

void Skybox::draw() {
	glDepthFunc(GL_LEQUAL);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0 + texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}

void Skybox::GenObjectBuffer() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);
}

unsigned int Skybox::loadCubemap(std::vector<std::string> faces)
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
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
			std::cout << "cube " << i << " loaded\n";
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
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