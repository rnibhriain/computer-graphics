//Some Windows Headers (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "camera.h"
#include "maths_funcs.h" //Anton's math class
#include "teapot.h" // teapot mesh
#include <string> 
#include <fstream>
#include <iostream>
#include <math.h>

#include "Snow.h"

// loading textures
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <sstream>

#include "model.h"
#include "shader.h"
#include "skybox.h"
#include "heightmap.h"


Snow snow;

Shader planeShader;
Shader skyboxShader;

Skybox skybox;

HeightMap heightmap;

using namespace std;

unsigned int snowTexture;
unsigned int snowManTexture;
unsigned int woodTexture;
unsigned int hatTexture;
unsigned int skyTexture;
unsigned int barkTexture;
unsigned int houseTexture;
unsigned int fireTexture;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

string directory;

vector<Texture> textures;

// camera
Camera camera(vec3(0.0f, 0.0f, 3.0f));
Camera skyCamera(vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

unsigned int loadTexture(char const* path) {
	unsigned int textureID = 0;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);


	if (data)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		std::cout << "Texture loaded\n";

	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
	}
	stbi_image_free(data);
	return textureID;
}



/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_BODY "snowman.dae"
#define MESH_ARMS "snowmanArms.dae"
#define MESH_HAT "snowmanHat.dae"
#define PLANE_MESH "snow.obj"
#define LEAF "leaf.obj"
#define BARK "bark.obj"
#define HOUSE "house.obj"
#define FIRE "fire.obj"

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
unsigned int plane_vp_vbo = 0;
unsigned int plane_vn_vbo = 0;
unsigned int plane_vt_vbo = 0;

unsigned int snowman_vp_vbo = 0;
unsigned int snowman_vn_vbo = 0;
unsigned int snowman_vt_vbo = 0;

unsigned int arms_vp_vbo = 0;
unsigned int arms_vn_vbo = 0;
unsigned int arms_vt_vbo = 0;

unsigned int hat_vp_vbo = 0;
unsigned int hat_vn_vbo = 0;
unsigned int hat_vt_vbo = 0;

unsigned int leaf_vp_vbo = 0;
unsigned int leaf_vn_vbo = 0;
unsigned int leaf_vt_vbo = 0;

unsigned int bark_vp_vbo = 0;
unsigned int bark_vn_vbo = 0;
unsigned int bark_vt_vbo = 0;

unsigned int house_vp_vbo = 0;
unsigned int house_vn_vbo = 0;
unsigned int house_vt_vbo = 0;

unsigned int fire_vp_vbo = 0;
unsigned int fire_vn_vbo = 0;
unsigned int fire_vt_vbo = 0;


using namespace std;
GLuint shaderProgramID;
GLuint snowShader;

ModelData mesh_data;
ModelData plane_data;
ModelData arms_data;
ModelData hat_data;
ModelData leaf_data;
ModelData bark_data;
ModelData house_data;
ModelData fire_data;

unsigned int mesh_vao = 0;


// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

unsigned int teapot_vao = 0;
int width = 800.0;
int height = 600.0;

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);


	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}
	aiMaterial* material = scene->mMaterials[scene->mMeshes[0]->mMaterialIndex];

	aiReleaseImport(scene);
	return modelData;
}



#pragma endregion MESH LOADING


float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame


GLuint loc1, loc2, loc3;

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS

void generateObjectBufferMesh() {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	Mesh planeLoader = Mesh();

	mesh_data = planeLoader.load_mesh(MESH_BODY);

	arms_data = load_mesh(MESH_ARMS);

	plane_data = load_mesh(PLANE_MESH);

	hat_data = load_mesh(MESH_HAT);

	leaf_data = load_mesh(LEAF);

	bark_data = load_mesh(BARK);

	house_data = load_mesh(HOUSE);

	fire_data = load_mesh(FIRE);

	plane_vp_vbo = 0;
	glGenBuffers(1, &plane_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, plane_data.mPointCount * sizeof(vec3), &plane_data.mVertices[0], GL_STATIC_DRAW);

	plane_vn_vbo = 0;
	glGenBuffers(1, &plane_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, plane_data.mPointCount * sizeof(vec3), &plane_data.mNormals[0], GL_STATIC_DRAW);

	
	plane_vt_vbo = 0;
	glGenBuffers(1, &plane_vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, plane_data.mPointCount * sizeof(vec2), &plane_data.mTextureCoords[0], GL_STATIC_DRAW);
	

	snowman_vp_vbo = 0;
	glGenBuffers(1, &snowman_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, snowman_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);

	

	snowman_vn_vbo = 0;
	glGenBuffers(1, &snowman_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, snowman_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);
	
	snowman_vt_vbo = 0;
	glGenBuffers(1, &snowman_vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, snowman_vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec2), &mesh_data.mTextureCoords[0], GL_STATIC_DRAW);
	

	arms_vp_vbo = 0;
	glGenBuffers(1, &arms_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, arms_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, arms_data.mPointCount * sizeof(vec3), &arms_data.mVertices[0], GL_STATIC_DRAW);



	arms_vn_vbo = 0;
	glGenBuffers(1, &arms_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, arms_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, arms_data.mPointCount * sizeof(vec3), &arms_data.mNormals[0], GL_STATIC_DRAW);
	
	arms_vt_vbo = 0;
	glGenBuffers(1, &arms_vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, arms_vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, arms_data.mPointCount * sizeof(vec2), &arms_data.mTextureCoords[0], GL_STATIC_DRAW);
	

	hat_vp_vbo = 0;
	glGenBuffers(1, &hat_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, hat_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, hat_data.mPointCount * sizeof(vec3), &hat_data.mVertices[0], GL_STATIC_DRAW);



	hat_vn_vbo = 0;
	glGenBuffers(1, &hat_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, hat_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, hat_data.mPointCount * sizeof(vec3), &hat_data.mNormals[0], GL_STATIC_DRAW);

	hat_vt_vbo = 0;
	glGenBuffers(1, &hat_vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, hat_vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, hat_data.mPointCount * sizeof(vec2), &hat_data.mTextureCoords[0], GL_STATIC_DRAW);
	
	leaf_vp_vbo = 0;
	glGenBuffers(1, &leaf_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, leaf_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, leaf_data.mPointCount * sizeof(vec3), &leaf_data.mVertices[0], GL_STATIC_DRAW);



	leaf_vn_vbo = 0;
	glGenBuffers(1, &leaf_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, leaf_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, leaf_data.mPointCount * sizeof(vec3), &leaf_data.mNormals[0], GL_STATIC_DRAW);

	leaf_vt_vbo = 0;
	glGenBuffers(1, &leaf_vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, leaf_vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, leaf_data.mPointCount * sizeof(vec2), &leaf_data.mTextureCoords[0], GL_STATIC_DRAW);


	bark_vp_vbo = 0;
	glGenBuffers(1, &bark_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, bark_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, bark_data.mPointCount * sizeof(vec3), &bark_data.mVertices[0], GL_STATIC_DRAW);



	bark_vn_vbo = 0;
	glGenBuffers(1, &bark_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, bark_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, bark_data.mPointCount * sizeof(vec3), &bark_data.mNormals[0], GL_STATIC_DRAW);

	bark_vt_vbo = 0;
	glGenBuffers(1, &bark_vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, bark_vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, bark_data.mPointCount * sizeof(vec2), &bark_data.mTextureCoords[0], GL_STATIC_DRAW);

	house_vp_vbo = 0;
	glGenBuffers(1, &house_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, house_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, house_data.mPointCount * sizeof(vec3), &house_data.mVertices[0], GL_STATIC_DRAW);



	house_vn_vbo = 0;
	glGenBuffers(1, &house_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, house_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, house_data.mPointCount * sizeof(vec3), &house_data.mNormals[0], GL_STATIC_DRAW);

	house_vt_vbo = 0;
	glGenBuffers(1, &house_vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, house_vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, house_data.mPointCount * sizeof(vec2), &house_data.mTextureCoords[0], GL_STATIC_DRAW);

	fire_vp_vbo = 0;
	glGenBuffers(1, &fire_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, fire_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, fire_data.mPointCount * sizeof(vec3), &fire_data.mVertices[0], GL_STATIC_DRAW);



	fire_vn_vbo = 0;
	glGenBuffers(1, &fire_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, fire_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, fire_data.mPointCount * sizeof(vec3), &fire_data.mNormals[0], GL_STATIC_DRAW);

	fire_vt_vbo = 0;
	glGenBuffers(1, &fire_vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, fire_vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, fire_data.mPointCount * sizeof(vec2), &fire_data.mTextureCoords[0], GL_STATIC_DRAW);


	skybox.GenObjectBuffer();
}


#pragma endregion VBO_FUNCTIONS


glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

const float radius = 10.0f;

GLfloat forward_x = 0;
GLfloat forward_z = 0;
GLfloat angle = 0;
float rotate_z = 1;

float ambient = 0.5f;
float diffuse = 0.4f;
float spec = 1.0f;

unsigned int VAO;

int fog = 0;
bool startSnow = false;

vec3 lightPositions[] = {
	vec3(0.0f, 10.0f, -15.0f),
	vec3(15.0f, 6.0f, 10.0f),
	vec3(-10.0f, 3.0f, -15.0f),
	vec3(0.0f, 0.0f, 40.0f)
};

void display(){

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.4, 0.4, 0.4, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(planeShader.ID);

	planeShader.setVec3("viewPos", camera.Position);

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(planeShader.ID, "model");
	int view_mat_location = glGetUniformLocation (planeShader.ID, "view");
	int proj_mat_location = glGetUniformLocation (planeShader.ID, "proj");

	//mat4 projection = perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	loc1 = glGetAttribLocation(planeShader.ID, "vertex_position");
	loc2 = glGetAttribLocation(planeShader.ID, "vertex_normal");
	loc3 = glGetAttribLocation(planeShader.ID, "vertex_texture");
	
	glBindFragDataLocation(planeShader.ID, 1, "fragment_colour");

	mat4 view = camera.GetViewMatrix();
	mat4 persp_proj = perspective(camera.Zoom, (float)width / (float)height, 0.1f, 100.0f);
	mat4 model = identity_mat4();
	view = translate(view, vec3(0.0f, 0.0f, -60.0f));

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);

	planeShader.setVec3("dirLight.direction", vec3 (-0.2f, -1.0f, -0.3f));
	planeShader.setVec3("dirLight.ambient", vec3(0.005f, 0.005f, 0.005f));
	planeShader.setVec3("dirLight.diffuse", vec3(0.05f, 0.05f, 0.05f));
	planeShader.setVec3("dirLight.specular", vec3(0.05f, 0.05f, 0.05f));


	planeShader.setVec3("pointLights[0].position", lightPositions[0]);
	planeShader.setVec3("pointLights[0].ambient", vec3(ambient, ambient, ambient));
	planeShader.setVec3("pointLights[0].diffuse", vec3(diffuse, diffuse, diffuse));
	planeShader.setVec3("pointLights[0].specular", vec3(spec, spec, spec));
	planeShader.setFloat("pointLights[0].constant", 1.0f);
	planeShader.setFloat("pointLights[0].linear", 0.014);
	planeShader.setFloat("pointLights[0].quadratic", 0.02);


	planeShader.setVec3("pointLights[1].position", lightPositions[1]);
	planeShader.setVec3("pointLights[1].ambient", vec3(ambient, ambient, ambient));
	planeShader.setVec3("pointLights[1].diffuse", vec3(diffuse, diffuse, diffuse));
	planeShader.setVec3("pointLights[1].specular", vec3(spec, spec, spec));
	planeShader.setFloat("pointLights[1].constant", 1.0f);
	planeShader.setFloat("pointLights[1].linear", 0.014);
	planeShader.setFloat("pointLights[1].quadratic", 0.002);



	planeShader.setVec3("pointLights[2].position", lightPositions[2]);
	planeShader.setVec3("pointLights[2].ambient", vec3(ambient, ambient, ambient));
	planeShader.setVec3("pointLights[2].diffuse", vec3(diffuse, diffuse, diffuse));
	planeShader.setVec3("pointLights[2].specular", vec3(spec, spec, spec));
	planeShader.setFloat("pointLights[2].constant", 1.0f);
	planeShader.setFloat("pointLights[2].linear", 0.014);
	planeShader.setFloat("pointLights[2].quadratic", 0.0002);

	planeShader.setVec3("pointLights[3].position", lightPositions[3]);
	planeShader.setVec3("pointLights[3].ambient", vec3(ambient, ambient, ambient));
	planeShader.setVec3("pointLights[3].diffuse", vec3(diffuse, diffuse, diffuse));
	planeShader.setVec3("pointLights[3].specular", vec3(spec, spec, spec));
	planeShader.setFloat("pointLights[3].constant", 1.0f);
	planeShader.setFloat("pointLights[3].linear", 0.014);
	planeShader.setFloat("pointLights[3].quadratic", 0.0002);

	planeShader.setVec3("material.ambient", vec3(1.0f, 1.0f, 1.0f));
	planeShader.setVec3("material.specular", vec3(0.5f, 0.5f, 0.5f)); // specular lighting doesn't have full effect on this object's material
	planeShader.setFloat("material.shininess", 10.0f);
	planeShader.setFloat("material.diffuse", 0.8f);

	planeShader.setVec3("position_eye", camera.Position);
	planeShader.setInt("fog", fog);

	mat4 Plane = identity_mat4();
	Plane = translate(Plane, vec3(5.0f, 0.0f, 0.0f));
	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vt_vbo);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, Plane.m);

	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, snowTexture);

	glDrawArrays(GL_TRIANGLES, 3, plane_data.mPointCount);

	planeShader.setVec3("material.ambient", vec3(1.0f, 1.0f, 1.0f));
	planeShader.setVec3("material.specular", vec3(0.0f, 0.0f, 0.0f)); // specular lighting doesn't have full effect on this object's material
	planeShader.setFloat("material.shininess", 32.0f);

	mat4 House = identity_mat4();
	House = translate(House, vec3(50.0f, 0.0f, 50.0f));
	House = Plane * House;
	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, house_vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, house_vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, house_vt_vbo);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, House.m);

	glBindTexture(GL_TEXTURE_2D, woodTexture);

	glDrawArrays(GL_TRIANGLES, 3, house_data.mPointCount);


	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {

			mat4 bark = identity_mat4();
			bark = translate(bark, vec3(-75.0f + j * 20, 0.0f, -15.0f * (i + 1)));

			glEnableVertexAttribArray(loc1);
			glBindBuffer(GL_ARRAY_BUFFER, bark_vp_vbo);
			glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(loc2);
			glBindBuffer(GL_ARRAY_BUFFER, bark_vn_vbo);
			glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(loc3);
			glBindBuffer(GL_ARRAY_BUFFER, bark_vt_vbo);
			glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
			glUniformMatrix4fv(matrix_location, 1, GL_FALSE, bark.m);

			glActiveTexture(GL_TEXTURE0);

			glBindTexture(GL_TEXTURE_2D, woodTexture);

			glDrawArrays(GL_TRIANGLES, 3, bark_data.mPointCount);

			mat4 leaves = identity_mat4();
			leaves = bark * leaves;

			glEnableVertexAttribArray(loc1);
			glBindBuffer(GL_ARRAY_BUFFER, leaf_vp_vbo);
			glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(loc2);
			glBindBuffer(GL_ARRAY_BUFFER, leaf_vn_vbo);
			glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(loc3);
			glBindBuffer(GL_ARRAY_BUFFER, leaf_vt_vbo);
			glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

			glUniformMatrix4fv(matrix_location, 1, GL_FALSE, leaves.m);

			glBindTexture(GL_TEXTURE_2D, snowTexture);

			glDrawArrays(GL_TRIANGLES, 3, leaf_data.mPointCount);

		}
	}

	for (int i = 0; i < 9; i++) {
		planeShader.setVec3("material.ambient", vec3(1.0f, 1.0f, 1.0f));
		planeShader.setVec3("material.specular", vec3(0.1f, 0.1f, 0.1f)); // specular lighting doesn't have full effect on this object's material
		planeShader.setFloat("material.shininess", 32.0f);
		
		mat4 snowman = identity_mat4();
		snowman = translate(snowman, vec3(-75.0f+i*20, 0.0f, forward_x));
		glEnableVertexAttribArray(loc1);
		glBindBuffer(GL_ARRAY_BUFFER, snowman_vp_vbo);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, snowman_vn_vbo);
		glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(loc3);
		glBindBuffer(GL_ARRAY_BUFFER, snowman_vt_vbo);
		glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, snowman.m);

		glBindTexture(GL_TEXTURE_2D, snowManTexture);

		glDrawArrays(GL_TRIANGLES, 3, mesh_data.mPointCount);

		planeShader.setInt("material.diffuse", 0.3);
		mat4 arms = identity_mat4();
		arms = rotate_z_deg(arms, rotate_z);
		arms = snowman * arms;

		glEnableVertexAttribArray(loc1);
		glBindBuffer(GL_ARRAY_BUFFER, arms_vp_vbo);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, arms_vn_vbo);
		glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(loc3);
		glBindBuffer(GL_ARRAY_BUFFER, arms_vt_vbo);
		glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, arms.m);

		glBindTexture(GL_TEXTURE_2D, barkTexture);

		glDrawArrays(GL_TRIANGLES, 3, arms_data.mPointCount);

		planeShader.setVec3("material.ambient", vec3(1.0f, 1.0f, 1.0f));
		planeShader.setVec3("material.specular", vec3(1.0f, 1.0f, 1.0f)); 
		planeShader.setFloat("material.shininess", 100.0f);

		mat4 hat = identity_mat4();
		hat = rotate_y_deg(hat, angle);
		hat = snowman * hat;

		glEnableVertexAttribArray(loc1);
		glBindBuffer(GL_ARRAY_BUFFER, hat_vp_vbo);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, hat_vn_vbo);
		glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(loc3);
		glBindBuffer(GL_ARRAY_BUFFER, hat_vt_vbo);
		glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, hat.m);

		glBindTexture(GL_TEXTURE_2D, hatTexture);

		glDrawArrays(GL_TRIANGLES, 3, hat_data.mPointCount);
	}

	mat4 fire = identity_mat4();
	fire = translate(fire, vec3(30.0f, 2.5f, 30.0f));
	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, fire_vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, fire_vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, fire_vt_vbo);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, fire.m);

	glBindTexture(GL_TEXTURE_2D, fireTexture);

	glDrawArrays(GL_TRIANGLES, 3, fire_data.mPointCount);

	

	

	if (fog == 0) {
		skyboxShader.use();
		mat4 skyView = skyCamera.GetViewMatrix();
		matrix_location = glGetUniformLocation(skyboxShader.ID, "model");
		skyboxShader.setMat4("view", skyView);
		skyboxShader.setMat4("proj", persp_proj);
		skybox.draw();
	}
	
	if (startSnow) {
		glUseProgram(planeShader.ID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, snowManTexture);

		snow.drawRain();
	}
	

	glutSwapBuffers();
}


bool start = false;
float x = 3.0f;
float y = 3.0f;
float z = 10.0f;
float  delta = 0;

void updateScene() {	

		// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	float  delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;
	if (start)
	{	
		
		if (forward_z > 0.5f || forward_z < -0.5f) {
			x = -x;
		}
		if (rotate_z > 4 || rotate_z < -4) {
			z = -z;
		}
		forward_z += x * delta;
		rotate_z += z * delta;
		if (forward_x > 40.0f || forward_x < 0.f) {
			y = -y;
		}
		forward_x += y * delta;
		angle += 1.0f;
	}
	
	// Draw the next frame
	glutPostRedisplay();
}

//keyboard event handler
void keyboard(unsigned char key, int x, int y)
{
	if (key == 'w') {
		camera.ProcessKeyboard(FORWARD, 0.5);
	}
	if (key == 'a') {
		camera.ProcessKeyboard(LEFT, 0.5);
	}
	if (key == 'd') {
		camera.ProcessKeyboard(RIGHT, 0.5);
	}
	if (key == 's') {
		camera.ProcessKeyboard(BACKWARD, 0.5);
	}
	if (key == 't') {
		start = !start;
	}
	if (key == 'f') {
		if (fog == 0) {
			fog = 1;
		}
		else {
			fog = 0;
		}
	}
	if (key == 'h') {
		startSnow = !startSnow;
	}

	if (key == 'r') {
		ambient = 0.5f;
		diffuse = 0.4f;
		spec = 1.0f;
	}
	//Ambient
	if (key == 'i') {
		ambient = 0.5f;
		diffuse = 0.0f;
		spec = 0.0f;
	}
	//Diffuse
	if (key == 'y') {
		ambient = 0.0f;
		diffuse = 0.4f;
		spec = 0.0f;
	}
	//Spec
	if (key == 'u') {
		spec = 1.0f;
		ambient = 0.0f;
		diffuse = 0.0f;
	}
}
int startX, startY, tracking = 0;

void processSelection(int xx, int yy) {
	unsigned char res[4];
	GLint viewport[4];

	updateScene();

	glGetIntegerv(GL_VIEWPORT, viewport);
	glReadPixels(xx, viewport[3] - yy, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);

}

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		startX = x;
		startY = y;
		processSelection(x, y);
	}
	if (button == 4) {
		camera.ProcessMouseScroll(1.0f);
	}
	if (button == 3) {
		camera.ProcessMouseScroll(-1.0f);
	}
}


void mouseCallback(int xposIn, int yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
	skyCamera.ProcessMouseMovement(xoffset, yoffset);
}

void init()
{
	skybox = Skybox();

	
	planeShader = Shader("Shaders/snowVertexShader.txt", "Shaders/snowFragmentShader.txt");
	skyboxShader = Shader("Shaders/skyVS.txt", "Shaders/skyFS.txt");

	//skyboxShader.use();
	//skyboxShader.setInt("skybox", 1);

	snowTexture = loadTexture("snowyground.jpg");
	snowManTexture = loadTexture("snow.jpg");
	woodTexture = loadTexture("wood.jpg");
	hatTexture = loadTexture("black.jpg");
	barkTexture = loadTexture("bark.jpg");
	houseTexture = loadTexture("farmhouse.jpg");
	fireTexture = loadTexture("campfire.png");

	//heightmap = HeightMap();
	snow = Snow();
	generateObjectBufferMesh();
	
}

int main(int argc, char** argv){
	srand(1);
	// Set up the window

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);

	glutCreateWindow("Winter Wonderland");
	// Tell glut where the display function is
	
	glEnable(GL_BLEND); //Enable blending.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set up your objects and shaders
	
	// A call to glewInit() must be done after glut is initialized!
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouseCallback);
	glutMouseFunc(mouse);
	glewExperimental = GL_TRUE; //for non-lab machines, this line gives better modern GL support
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	init();

	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
