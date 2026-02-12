// model.hpp (UPDATED)
// - keeps Assimp scene alive (Importer is a member)
// - exposes node/world transform lookup for FBX marker empties
// - does NOT change your Mesh/Texture pipeline

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
#include <iostream>
#include <vector>

using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

// Convert Assimp matrix to GLM (column-major)
static inline glm::mat4 AiToGlm(const aiMatrix4x4& m)
{
    glm::mat4 r;
    r[0][0] = m.a1; r[1][0] = m.a2; r[2][0] = m.a3; r[3][0] = m.a4;
    r[0][1] = m.b1; r[1][1] = m.b2; r[2][1] = m.b3; r[3][1] = m.b4;
    r[0][2] = m.c1; r[1][2] = m.c2; r[2][2] = m.c3; r[3][2] = m.c4;
    r[0][3] = m.d1; r[1][3] = m.d2; r[2][3] = m.d3; r[3][3] = m.d4;
    return r;
}

class Model
{
public:
    vector<Texture> textures_loaded;
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    Model() : gammaCorrection(false) {}
    void SetOverrideDiffuse(unsigned int texId)
    {
        for (auto& m : meshes) m.overrideDiffuse = texId;
    }
    Model(string const& path, bool gamma = false)
        : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    bool isLoaded() const { return m_scene != nullptr; }

    // For FBX marker empties in the SAME FBX:
    // Returns accumulated (root->...->node) transform in model space.
    bool GetNodeWorldTransform(const std::string& nodeName, glm::mat4& outWorld) const
    {
        if (!m_scene || !m_scene->mRootNode) return false;
        return GetNodeWorldTransformRec(m_scene->mRootNode, nodeName, aiMatrix4x4(), outWorld);
    }

    // For a marker FBX saved as a separate file (usually 1 object):
    // Returns transform of the first "meaningful" node below root.
    bool GetMarkerWorldTransform(glm::mat4& outWorld) const
    {
        if (!m_scene || !m_scene->mRootNode) return false;
        const aiNode* root = m_scene->mRootNode;

        if (root->mNumChildren == 0)
        {
            // fallback: root transform
            outWorld = AiToGlm(root->mTransformation);
            return true;
        }

        // Often: root->child[0] is the exported object (Empty/Null)
        // If exporter wraps it with an extra dummy, this still works in practice.
        const aiNode* n = root->mChildren[0];

        // Sometimes child[0] is a wrapper and real node is its child[0]
        if (n->mNumChildren > 0 && n->mNumMeshes == 0)
        {
            // prefer deeper node if it exists (common FBX structure)
            const aiNode* c = n->mChildren[0];
            // Use it only if it looks like an actual object (name not empty)
            if (c && c->mName.length > 0) n = c;
        }

        aiMatrix4x4 world = root->mTransformation * n->mTransformation;
        outWorld = AiToGlm(world);
        return true;
    }

    // Debug helper: prints node hierarchy (call once to see names in console)
    void DebugPrintNodes() const
    {
        if (!m_scene || !m_scene->mRootNode) return;
        DebugPrintNodesRec(m_scene->mRootNode, 0);
    }

private:
    // Keep importer as MEMBER so scene pointer stays valid
    Assimp::Importer m_importer;
    const aiScene* m_scene = nullptr;

    void loadModel(string const& path)
    {
        m_scene = m_importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace
        );

        if (!m_scene || (m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !m_scene->mRootNode)
        {
            cout << "ERROR::ASSIMP:: " << m_importer.GetErrorString() << endl;
            m_scene = nullptr;
            return;
        }

        directory = path.substr(0, path.find_last_of('/'));
        processNode(m_scene->mRootNode, m_scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
            processNode(node->mChildren[i], scene);
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 v;

            v.x = mesh->mVertices[i].x;
            v.y = mesh->mVertices[i].y;
            v.z = mesh->mVertices[i].z;
            vertex.Position = v;

            if (mesh->HasNormals())
            {
                v.x = mesh->mNormals[i].x;
                v.y = mesh->mNormals[i].y;
                v.z = mesh->mNormals[i].z;
                vertex.Normal = v;
            }

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 uv;
                uv.x = mesh->mTextureCoords[0][i].x;
                uv.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = uv;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f);
            }

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "uDiffMap");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "uSpecMap");
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

    bool GetNodeWorldTransformRec(const aiNode* node,
        const std::string& target,
        const aiMatrix4x4& parentWorld,
        glm::mat4& outWorld) const
    {
        aiMatrix4x4 world = parentWorld * node->mTransformation;

        if (target == std::string(node->mName.C_Str()))
        {
            outWorld = AiToGlm(world);
            return true;
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            if (GetNodeWorldTransformRec(node->mChildren[i], target, world, outWorld))
                return true;
        }

        return false;
    }

    void DebugPrintNodesRec(const aiNode* node, int depth) const
    {
        for (int i = 0; i < depth; i++) std::cout << "  ";
        std::cout << node->mName.C_Str() << "\n";
        for (unsigned int i = 0; i < node->mNumChildren; i++)
            DebugPrintNodesRec(node->mChildren[i], depth + 1);
    }
};

#endif
