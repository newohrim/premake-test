#include "MeshLoader.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <iostream>
#include <vector>
#include <cassert>
#include <functional>

#include "GeometryData.h"
#include "Renderer.h"
#include "Mesh.h"

#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


bool MeshLoader::LoadMesh(const std::string& path, Renderer* renderer, Mesh::PTR& outMesh)
{
    // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_ConvertToLeftHanded/* | aiProcess_GenBoundingBoxes*/);

    // If the import failed, report it
    if (nullptr == scene) {
        //DoTheErrorLogging(importer.GetErrorString());
        std::cout << importer.GetErrorString() << '\n';
        return false;
    }

    // Now we can access the file's contents.
    //DoTheSceneProcessing(scene);
    outMesh = std::make_shared<Mesh>();
    CopyNodesWithMeshes(scene, scene->mRootNode, outMesh->root, renderer, aiMatrix4x4{});

    // We're done. Everything will be cleaned up by the importer destructor
    return true;
}

void MeshLoader::CopyNodesWithMeshes(const aiScene* scene, aiNode* node, Mesh::MeshNode& targetParent, Renderer* renderer, const aiMatrix4x4& accTransform) {
    aiMatrix4x4 transform;

    // if node has meshes, create a new scene object for it
    if (node->mNumMeshes > 0) {
        // copy the meshes
        for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
            AddMesh(scene, i, renderer, targetParent);
        }

        aiVector3D pos;
        aiVector3D rotAxis;
        ai_real angle;
        aiVector3D scale;
        // TODO: replace transform with node's transoform or accTransform???
        transform.Decompose(scale, rotAxis, angle, pos);
        targetParent.pos = *reinterpret_cast<Math::Vector3*>(&pos);
        if (rotAxis.SquareLength() > 0.99f) {
            targetParent.rot = Math::Quaternion::CreateFromAxisAngle(
                *reinterpret_cast<Math::Vector3*>(&rotAxis), angle);
        }
        targetParent.scale = *reinterpret_cast<Math::Vector3*>(&scale);
    }
    else {
        // if no meshes, skip the node, but keep its transformation
        transform = node->mTransformation * accTransform;
        for (uint32_t i = 0; i < node->mNumChildren; ++i) {
            aiNode* child = node->mChildren[i];
            CopyNodesWithMeshes(scene, child, targetParent, renderer, transform);
        }
        return;
    }

    // continue for all child nodes
    targetParent.children.resize(node->mNumChildren);
    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        aiNode* child = node->mChildren[i];
        CopyNodesWithMeshes(scene, child, targetParent.children[i], renderer, transform);
    }
}

struct MeshData {
    std::vector<float> vertFloats;
    std::vector<uint32_t> indices;
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputs;
    uint32_t stride = 0;
    float boundingSphereRadius = 0.0f;
};

MeshData GatherMeshData(const aiScene* scene, int meshIdx) 
{
    MeshData res;
    std::vector<std::function<void(int)>> pipeline;
    auto addVector = [&res](const aiVector3D& v) {
        res.vertFloats.push_back(v.x);
        if (res.vertFloats.back() > 905 && res.vertFloats.back() < 906) {
        }
        res.vertFloats.push_back(v.y);
        if (res.vertFloats.back() > 905 && res.vertFloats.back() < 906) {
        }
        res.vertFloats.push_back(v.z);
        if (res.vertFloats.back() > 905 && res.vertFloats.back() < 906) {
        }
    };
    auto addInput3 = [&res](const char* channel) {
        res.stride += sizeof(float) * 3;
        res.inputs.push_back({
            channel,
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        });
    };

    const aiMesh* mesh = scene->mMeshes[meshIdx];
    if (!mesh->HasPositions()) {
        return res;
    }
    addInput3("POSITION");

    uint32_t floatsNum = mesh->mNumVertices * 3;
    if (floatsNum == 0) {
        return res;
    }
    if (mesh->HasNormals()) {
        floatsNum += mesh->mNumVertices * 3;
        pipeline.push_back([&](int idx) { addVector(mesh->mNormals[idx]); });
        addInput3("NORMAL");
    }
    static constexpr int TEXCOORD_IDX = 0;  // TODO: support several uv channels?
    if (mesh->GetNumUVChannels() > 0 && mesh->HasTextureCoords(TEXCOORD_IDX)) {
        const uint32_t uvCompNum = mesh->mNumUVComponents[TEXCOORD_IDX];
        floatsNum += mesh->mNumVertices * uvCompNum;
        pipeline.push_back([&](int idx) {
            for (uint32_t i = 0; i < uvCompNum; ++i) {
                res.vertFloats.push_back(mesh->mTextureCoords[TEXCOORD_IDX][idx][i]);
                if (res.vertFloats.back() > 905 && res.vertFloats.back() < 906) {
                }
            }
        });
        DXGI_FORMAT uvFormat;
        switch (uvCompNum) {
        case 1:
            uvFormat = DXGI_FORMAT_R32_FLOAT;
            break;
        case 2:
            uvFormat = DXGI_FORMAT_R32G32_FLOAT;
            break;
        case 3:
            uvFormat = DXGI_FORMAT_R32G32B32_FLOAT;
            break;
        default:
            assert(false);
        }
        res.stride += sizeof(float) * uvCompNum;
        res.inputs.push_back({
            "TEXCOORD",
            0,
            uvFormat,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        });
    }
    res.vertFloats.reserve(floatsNum);

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        const aiVector3D& vert = mesh->mVertices[i];
        res.boundingSphereRadius = max(res.boundingSphereRadius, vert.SquareLength());
        addVector(vert);
        for (auto stage : pipeline) {
            stage(i);
        }
    }
    assert(res.vertFloats.size() == floatsNum);

    static constexpr int INDEX_PER_FACE = 3;
    const int indicesNum = mesh->mNumFaces * INDEX_PER_FACE;
    res.indices.reserve(indicesNum);
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        const uint32_t indicesNum = face.mNumIndices;
        assert(indicesNum == INDEX_PER_FACE);
        for (uint32_t j = 0; j < indicesNum; ++j) {
            res.indices.push_back(face.mIndices[j]);
        }
    }
    assert(res.indices.size() == indicesNum);

    return res;
}

void MeshLoader::AddMesh(const aiScene* scene, int meshIdx, Renderer* renderer, Mesh::MeshNode& target)
{
    MeshData mesh = GatherMeshData(scene, meshIdx);
    mesh.boundingSphereRadius = std::sqrt(mesh.boundingSphereRadius);
    // TODO: replace make_shared with emplace_back here if possible?
    target.geoms.push_back(std::make_shared<GeometryData>(
        renderer->GetDevice(),
        static_cast<void*>(mesh.vertFloats.data()), (int)(sizeof(float) * mesh.vertFloats.size()),
        mesh.indices.data(), (int)(sizeof(uint32_t) * mesh.indices.size()),
        std::vector<uint32_t>{mesh.stride}, std::vector<uint32_t>{0}));
    // TODO: somehow use input layout from MeshData?
}
