#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"
#include "shader.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

// SAMO deklaracija (definicija ide u model.cpp)
unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model
{
public:
    // model data
    vector<Texture> textures_loaded;
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    // constructor
    Model(string const& path, bool gamma = false)
        : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // draw model
    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:

    void loadModel(string const& path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }

        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;

            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }

            // texture coordinates
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        // indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        vector<Texture> diffuseMaps =
            loadMaterialTextures(material, aiTextureType_DIFFUSE, "uDiffMap");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        vector<Texture> specularMaps =
            loadMaterialTextures(material, aiTextureType_SPECULAR, "uSpecMap");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        return Mesh(vertices, indices, textures);
    }

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;

        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }

            if (!skip)
            {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();

                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }

        return textures;
    }
};

#endif
