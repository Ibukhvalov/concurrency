#pragma once

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION

#include "glm-wrapper.hpp"
#include "hittable_list.h"
#include "triangle_mesh.h"
#include "tiny_gltf.h"

#include <string>
#include <vector>


//namespace oka
//{

//  valid range of coordinates [-1; 1]
uint32_t packNormal(const glm::float3& normal)
{
    uint32_t packed = (uint32_t)((normal.x + 1.0f) / 2.0f * 511.99999f);
    packed += (uint32_t)((normal.y + 1.0f) / 2.0f * 511.99999f) << 10;
    packed += (uint32_t)((normal.z + 1.0f) / 2.0f * 511.99999f) << 20;
    return packed;
}

glm::float2 unpackUV(uint32_t val)
{
    glm::float2 uv;
    uv.y = ((val & 0xffff0000) >> 16) / 16383.99999f * 10.0f - 5.0f;
    uv.x = (val & 0x0000ffff) / 16383.99999f * 10.0f - 5.0f;

    return uv;
}

//  valid range of coordinates [-10; 10]
uint32_t packTangent(const vec3& tangent)
{
    uint32_t packed = (uint32_t)((tangent.x() + 10.0f) / 20.0f * 511.99999f);
    packed += (uint32_t)((tangent.y() + 10.0f) / 20.0f * 511.99999f) << 10;
    packed += (uint32_t)((tangent.z() + 10.0f) / 20.0f * 511.99999f) << 20;
    return packed;
}

void computeTangent(std::vector<Vertex>& vertices,
                    const std::vector<uint32_t>& indices)
{
    const size_t lastIndex = indices.size();
    Vertex& v0 = vertices[indices[lastIndex - 3]];
    Vertex& v1 = vertices[indices[lastIndex - 2]];
    Vertex& v2 = vertices[indices[lastIndex - 1]];

    glm::float2 uv0 = unpackUV(v0.uv);
    glm::float2 uv1 = unpackUV(v1.uv);
    glm::float2 uv2 = unpackUV(v2.uv);

    vec3 deltaPos1 = v1.pos - v0.pos;
    vec3 deltaPos2 = v2.pos - v0.pos;
    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;

    vec3 tangent{ 0.0f, 0.0f, 1.0f };
    const float d = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
    if (abs(d) > 1e-6)
    {
        float r = 1.0f / d;
        tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
    }

    glm::uint32_t packedTangent = packTangent(tangent);

    v0.tangent = packedTangent;
    v1.tangent = packedTangent;
    v2.tangent = packedTangent;
}

glm::float4x4 getTransform(const tinygltf::Node& node, const float globalScale)
{
    if (node.matrix.empty())
    {
        glm::float3 scale{ 1.0f };
        if (!node.scale.empty())
        {
            scale = glm::float3((float)node.scale[0], (float)node.scale[1], (float)node.scale[2]);
            // check that scale is uniform, otherwise we have to support it in shader
            // assert(scale.x == scale.y && scale.y == scale.z);
        }

        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        if (!node.rotation.empty())
        {
            const float floatRotation[4] = {
                    (float)node.rotation[0],
                    (float)node.rotation[1],
                    (float)node.rotation[2],
                    (float)node.rotation[3],
            };
            rotation = glm::make_quat(floatRotation);
        }

        glm::float3 translation{ 0.0f };
        if (!node.translation.empty())
        {
            translation = glm::float3((float)node.translation[0], (float)node.translation[1], (float)node.translation[2]);
            translation *= globalScale;
        }

        const glm::float4x4 translationMatrix = glm::translate(glm::float4x4(1.0f), translation);
        const glm::float4x4 rotationMatrix{ rotation };
        const glm::float4x4 scaleMatrix = glm::scale(glm::float4x4(1.0f), scale);

        const glm::float4x4 localTransform = translationMatrix * rotationMatrix * scaleMatrix;

        return localTransform;
    }
    else
    {
        glm::float4x4 localTransform = glm::make_mat4(node.matrix.data());
        return localTransform;
    }
}

void processPrimitive(const tinygltf::Model& model, hittable_list& world, const tinygltf::Primitive& primitive, const glm::float4x4& transform/*, const float globalScale*/)
{
    using namespace std;
    assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

    const tinygltf::Accessor& positionAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
    const tinygltf::BufferView& positionView = model.bufferViews[positionAccessor.bufferView];
    const float* positionData = reinterpret_cast<const float*>(&model.buffers[positionView.buffer].data[positionAccessor.byteOffset + positionView.byteOffset]);
    assert(positionData != nullptr);
    const uint32_t vertexCount = static_cast<uint32_t>(positionAccessor.count);
    assert(vertexCount != 0);
    const int byteStride = positionAccessor.ByteStride(positionView);
    assert(byteStride > 0); // -1 means invalid glTF
    int posStride = byteStride / sizeof(float);

    // Normals
    const float* normalsData = nullptr;
    int normalStride = 0;
    if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
    {
        const tinygltf::Accessor& normalAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
        const tinygltf::BufferView& normView = model.bufferViews[normalAccessor.bufferView];
        normalsData = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normalAccessor.byteOffset + normView.byteOffset]));
        assert(normalsData != nullptr);
        normalStride = normalAccessor.ByteStride(normView) / sizeof(float);
        assert(normalStride > 0);
    }

    int matId = primitive.material;
    if (matId == -1)
    {
        matId = 0; // TODO: should be index of default material
    }

    vec3 sum = vec3(0.0, 0.0, 0.0);
    std::vector<Vertex> vertices;
    vertices.reserve(vertexCount);
    for (uint32_t v = 0; v < vertexCount; ++v)
    {
        Vertex vertex{};
        vertex.pos = glm::make_vec3(&positionData[v * posStride])/* * globalScale*/;
        vertex.normal = packNormal(glm::normalize(glm::vec3(normalsData ? glm::make_vec3(&normalsData[v * normalStride]) : glm::vec3(0.0f))));
        //vertex.uv = packUV(texCoord0Data ? glm::make_vec2(&texCoord0Data[v * texCoord0Stride]) : glm::vec3(0.0f));
        vertices.push_back(vertex);
        sum += vertex.pos;
    }
    const vec3 massCenter = sum / (float)vertexCount;

    uint32_t indexCount = 0;
    std::vector<uint32_t> indices;
    const bool hasIndices = (primitive.indices != -1);
    assert(hasIndices); // currently support only this mode
    if (hasIndices)
    {
        const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

        indexCount = static_cast<uint32_t>(accessor.count);
        assert(indexCount != 0 && (indexCount % 3 == 0));
        const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

        indices.reserve(indexCount);
        switch (accessor.componentType)
        {
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
                for (size_t index = 0; index < indexCount; index++)
                {
                    indices.push_back(buf[index]);
                }
                computeTangent(vertices, indices);
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
                for (size_t index = 0; index < indexCount; index++)
                {
                    indices.push_back(buf[index]);
                }
                computeTangent(vertices, indices);
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
                for (size_t index = 0; index < indexCount; index++)
                {
                    indices.push_back(buf[index]);
                }
                computeTangent(vertices, indices);
                break;
            }
            default:
                std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                return;
        }
    }

    world.addPrimitive(vertices, indices, transform);

    //uint32_t instId = world.createInstance(Instance::Type::eMesh, meshId, matId, transform);
    //assert(instId != -1);
}


void processMesh(const tinygltf::Model& model, hittable_list& world, const tinygltf::Mesh& mesh, const glm::float4x4& transform/*, const float globalScale*/)
{
    using namespace std;
    clog << "Mesh name: " << mesh.name << endl;
    clog << "Primitive count: " << mesh.primitives.size() << endl;
    for (size_t i = 0; i < mesh.primitives.size(); ++i)
    {
        processPrimitive(model, world, mesh.primitives[i], transform/*, globalScale*/);
    }
}

void processNode(const tinygltf::Model& model, hittable_list& world, const tinygltf::Node& node, const uint32_t currentNodeId/*, const glm::float4x4& baseTransform, const float globalScale*/)
{
    using namespace std;
    clog << "Node name: " << node.name << endl;

    const glm::float4x4 localTransform = getTransform(node, 1);
    const glm::float4x4 globalTransform = glm::float4x4(1.0f) * localTransform;

    if (node.mesh != -1) // mesh exist
    {
        const tinygltf::Mesh& mesh = model.meshes[node.mesh];
        processMesh(model, world, mesh, globalTransform/*, globalScale*/);
    }
    /*else if (node.camera != -1) // camera node
    {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(globalTransform, scale, rotation, translation, skew, perspective);

        rotation = glm::conjugate(rotation);

        scene.getCamera(node.camera).node = currentNodeId;
        scene.getCamera(node.camera).position = translation * scale;
        scene.getCamera(node.camera).mOrientation = rotation;
        scene.getCamera(node.camera).updateViewMatrix();
    }*/

    for (int i = 0; i < node.children.size(); ++i)
    {
        //scene.mNodes[node.children[i]].parent = currentNodeId;
        processNode(model, world, model.nodes[node.children[i]], node.children[i]/*, globalTransform, globalScale*/);
    }
}
    class GltfLoader
    {
    private:

    public:
        explicit GltfLoader(){}

        static bool loadGltf(const std::string& modelPath, hittable_list& world) {
            if (modelPath.empty())
            {
                return false;
            }

            using namespace std;
            tinygltf::Model model;
            tinygltf::TinyGLTF gltf_ctx;
            std::string err;
            std::string warn;
            bool res = gltf_ctx.LoadASCIIFromFile(&model, &err, &warn, modelPath);
            if (!res)
            {
                std::cerr << "Unable to load file: " << modelPath;
                return res;
            }

            int sceneId = model.defaultScene;

            /*loadMaterials(model, scene);
            if (loadLightsFromJson(modelPath, scene) == false)
            {
                STRELKA_WARNING("No light is scene, adding default distant light");
                oka::Scene::UniformLightDesc lightDesc {};
                // lightDesc.xform = glm::mat4(1.0f);
                // lightDesc.useXform = true;
                lightDesc.useXform = false;
                lightDesc.position = glm::float3(0.0f, 0.0f, 0.0f);
                lightDesc.orientation = glm::float3(-45.0f, 15.0f, 0.0f);
                lightDesc.type = 3; // distant light
                lightDesc.halfAngle = 10.0f * 0.5f * (M_PI / 180.0f);
                lightDesc.intensity = 100000;
                lightDesc.color = glm::float3(1.0);
                scene.createLight(lightDesc);
            }

            loadCameras(model, scene);

            const float globalScale = 1.0f;
            loadNodes(model, scene, globalScale);*/

            for (int i = 0; i < model.scenes[sceneId].nodes.size(); ++i)
            {
                const int rootNodeIdx = model.scenes[sceneId].nodes[i];
                processNode(model, world, model.nodes[rootNodeIdx], rootNodeIdx/*, glm::float4x4(1.0f), globalScale*/);
            }

            //loadAnimation(model, scene);

            return res;
        }

        /*void computeTangent(std::vector<Scene::Vertex>& _vertices,
                            const std::vector<uint32_t>& _indices) const;*/
    };
//} // namespace oka