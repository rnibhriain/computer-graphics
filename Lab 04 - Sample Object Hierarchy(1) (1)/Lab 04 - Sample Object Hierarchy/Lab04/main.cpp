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
#include <sstream>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
using namespace std;

//typedef double DWORD;
vec3 PosData(0.0f, 0.0f, 3.0f);

Camera camera(PosData);

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_BODY "snowman.dae"
#define MESH_ARMS "snowmanArms.dae"
#define MESH_HAT "snowmanHat.dae"
#define PLANE_MESH "snow.dae"
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

#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;
GLuint shaderProgramID;

ModelData mesh_data;
ModelData plane_data;
ModelData arms_data;
ModelData hat_data;
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

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING


float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS

char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "C:/Users/User/Documents/computer-graphics/Lab 04 - Sample Object Hierarchy(1) (1)/Lab 04 - Sample Object Hierarchy/Shaders/simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "C:/Users/User/Documents/computer-graphics/Lab 04 - Sample Object Hierarchy(1) (1)/Lab 04 - Sample Object Hierarchy/Shaders/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

GLuint loc1, loc2, loc3;

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS

void generateObjectBufferMesh() {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	mesh_data = load_mesh(MESH_BODY);

	arms_data = load_mesh(MESH_ARMS);

	plane_data = load_mesh(PLANE_MESH);

	hat_data = load_mesh(MESH_HAT);

	plane_vp_vbo = 0;
	glGenBuffers(1, &plane_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, plane_data.mPointCount * sizeof(vec3), &plane_data.mVertices[0], GL_STATIC_DRAW);

	plane_vn_vbo = 0;
	glGenBuffers(1, &plane_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, plane_data.mPointCount * sizeof(vec3), &plane_data.mNormals[0], GL_STATIC_DRAW);

	/*
	plane_vt_vbo = 0;
	glGenBuffers(1, &plane_vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, plane_data.mPointCount * sizeof(vec2), &plane_data.mTextureCoords[0], GL_STATIC_DRAW);
	*/
	

	snowman_vp_vbo = 0;
	glGenBuffers(1, &snowman_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, snowman_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);

	

	snowman_vn_vbo = 0;
	glGenBuffers(1, &snowman_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, snowman_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);
	/*
	snowman_vt_vbo = 0;
	glGenBuffers(1, &snowman_vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, snowman_vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec2), &mesh_data.mTextureCoords[0], GL_STATIC_DRAW);
	*/



	arms_vp_vbo = 0;
	glGenBuffers(1, &arms_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, arms_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, arms_data.mPointCount * sizeof(vec3), &arms_data.mVertices[0], GL_STATIC_DRAW);



	arms_vn_vbo = 0;
	glGenBuffers(1, &arms_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, arms_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, arms_data.mPointCount * sizeof(vec3), &arms_data.mNormals[0], GL_STATIC_DRAW);
	/*
	arms_vt_vbo = 0;
	glGenBuffers(1, &arms_vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, arms_vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, arms_data.mPointCount * sizeof(vec2), &arms_data.mTextureCoords[0], GL_STATIC_DRAW);
	*/

	hat_vp_vbo = 0;
	glGenBuffers(1, &hat_vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, hat_vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, hat_data.mPointCount * sizeof(vec3), &hat_data.mVertices[0], GL_STATIC_DRAW);



	hat_vn_vbo = 0;
	glGenBuffers(1, &hat_vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, hat_vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, hat_data.mPointCount * sizeof(vec3), &hat_data.mNormals[0], GL_STATIC_DRAW);
	
}



void generateObjectBufferTeapot () {
	GLuint vp_vbo = 0;

	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normals");
	
	glGenBuffers (1, &vp_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof (float), teapot_vertex_points, GL_STATIC_DRAW);
	GLuint vn_vbo = 0;
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof (float), teapot_normals, GL_STATIC_DRAW);
  
	glGenVertexArrays (1, &teapot_vao);
	glBindVertexArray (teapot_vao);

	glEnableVertexAttribArray (loc1);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (loc2);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer (loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
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
float rotate_x = 0;

void display(){

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable (GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram (shaderProgramID);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation (shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation (shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation (shaderProgramID, "proj");

	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");
	

	mat4 view = camera.GetViewMatrix();
	mat4 persp_proj = perspective(camera.Zoom, (float)width / (float)height, 0.1f, 100.0f);
	mat4 model = identity_mat4();
	view = translate(view, vec3(0.0f, -10.0f, -60.0f));

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);


	mat4 Plane = identity_mat4();
	Plane = translate(Plane, vec3(0.0f, 0.0f, 0.0f));
	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vt_vbo);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glActiveTexture(GL_TEXTURE1);

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, Plane.m);
	// update uniforms & draw
	glDrawArrays(GL_TRIANGLES, 1, plane_data.mPointCount);

	mat4 snowman = identity_mat4();
	snowman = translate(snowman, vec3(-40.0f, 0.0f, 0.0f));
	snowman = translate(snowman, vec3(0.0f, 0.0f, forward_x));
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

	glActiveTexture(GL_TEXTURE2);
	glDrawArrays(GL_TRIANGLES, 3, mesh_data.mPointCount);
	
	mat4 arms = identity_mat4();
	arms = translate(arms, vec3(0.0f, forward_z, 0.0f));
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

	glActiveTexture(GL_TEXTURE2);
	glDrawArrays(GL_TRIANGLES, 3, arms_data.mPointCount);

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

	glActiveTexture(GL_TEXTURE2);
	glDrawArrays(GL_TRIANGLES, 3, hat_data.mPointCount);

    glutSwapBuffers();
}


bool start = false;
float x = 3.0f;
float y = 3.0f;

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
		forward_z += x * delta;
		if (forward_x > 40.0f || forward_x < -40.f) {
			y = -y;
		}
		forward_x += y * delta;
		angle += 1.0f;
	}
	
	// Draw the next framew
	glutPostRedisplay();
}


//keyboard event handler
void keyboard(unsigned char key, int x, int y)
{
	const float cameraSpeed = 0.05f;
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
}
float lastX = width / 2.0f;
float lastY = height / 2.0f;
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
}

void mouseCallback(int x, int y) {
	/**if (firstMouse) {
		lastX = x;
		lastY = y;
		firstMouse = false;
	}*/

	float xoffset = x - lastX;
	float yoffset = lastY - y; // reversed since y-coordinates go from bottom to top

	lastX = x;
	lastY = y;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void init()
{
	
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();

	generateObjectBufferMesh();
	
}

int main(int argc, char** argv){

	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Viewport Teapots");
	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouseCallback);
	glutMouseFunc(mouse);

	 // A call to glewInit() must be done after glut is initialized!
	glewExperimental = GL_TRUE; //for non-lab machines, this line gives better modern GL support
    GLenum res = glewInit();
	// Check for any errors
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
    return 0;
}











