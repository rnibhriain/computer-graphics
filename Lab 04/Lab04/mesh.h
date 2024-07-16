#ifndef MESH_H
#include "shader.h"


#pragma once


// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;


#define MAX_BONE_INFLUENCE 4

#pragma region SimpleTypes
typedef struct
{
    size_t mPointCount = 0;
    std::vector<vec3> mVertices;
    std::vector<vec3> mNormals;
    std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    unsigned int VAO;

 

    Mesh() {};
    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

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
                //if (mesh->HasFaces()) {
                //	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                //		aiFace face = mesh->mFaces[i];
                //		for (unsigned int j = 0; j < face.mNumIndices; j++) {
                //			modelData.mIndices.push_back(face.mIndices[j]);
                //		}
                //	}
                //}
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
    
    // render the mesh
    void Draw(Shader& shader)
    {
        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            string number;
            string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++); // transfer unsigned int to string
            else if (name == "texture_normal")
                number = std::to_string(normalNr++); // transfer unsigned int to string
            else if (name == "texture_height")
                number = std::to_string(heightNr++); // transfer unsigned int to string

            // now set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        // ids
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
};
#endif