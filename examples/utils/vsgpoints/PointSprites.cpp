#include "PointSprites.h"

#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/material.h>

#include <iostream>

using namespace vsg;

vsg::ref_ptr<vsg::Data> vsg::createParticleImage(uint32_t dim)
{
    auto data = vsg::ubvec4Array2D::create(dim, dim);
    data->getLayout().format = VK_FORMAT_R8G8B8A8_UNORM;
    float div = 2.0f / static_cast<float>(dim - 1);
    float distance_at_one = 0.5f;
    float distance_at_zero = 1.0f;

    vsg::vec2 v;
    for (uint32_t r = 0; r < dim; ++r)
    {
        v.y = static_cast<float>(r) * div - 1.0f;
        for (uint32_t c = 0; c < dim; ++c)
        {
            v.x = static_cast<float>(c) * div - 1.0f;
            float distance_from_center = vsg::length(v);
            float intensity = 1.0f - (distance_from_center - distance_at_one) / (distance_at_zero - distance_at_one);
            if (intensity > 1.0f) intensity = 1.0f;
            if (intensity < 0.0f) intensity = 0.0f;
            uint8_t alpha = static_cast<uint8_t>(intensity * 255);
            data->set(c, r, vsg::ubvec4(255, 255, 255, alpha));
        }
    }
    return data;
}

vsg::ref_ptr<vsg::StateGroup> vsg::createPointSpriteStateGroup(vsg::ref_ptr<const vsg::Options> options)
{
    bool lighting = true;
    VkVertexInputRate normalInputRate = VK_VERTEX_INPUT_RATE_VERTEX; // VK_VERTEX_INPUT_RATE_INSTANCE
    VkVertexInputRate colorInputRate = VK_VERTEX_INPUT_RATE_VERTEX;  // VK_VERTEX_INPUT_RATE_INSTANCE

    for (auto& path : options->paths)
    {
        std::cout << "path = " << path << std::endl;
    }

    // load shaders
    auto vertexShader = vsg::read_cast<vsg::ShaderStage>("shaders/pointsprites.vert", options);
    //if (!vertexShader) vertexShader = assimp_vert(); // fallback to shaders/assimp_vert.cppp

    vsg::ref_ptr<vsg::ShaderStage> fragmentShader;
    if (lighting)
    {
        fragmentShader = vsg::read_cast<vsg::ShaderStage>("shaders/assimp_phong.frag", options);
        //if (!fragmentShader) fragmentShader = assimp_phong_frag();
    }
    else
    {
        fragmentShader = vsg::read_cast<vsg::ShaderStage>("shaders/assimp_flat_shaded.frag", options);
        //if (!fragmentShader) fragmentShader = assimp_flat_shaded_frag();
    }

    if (!vertexShader || !fragmentShader)
    {
        std::cout << "Could not create shaders." << std::endl;
        std::cout << "vertexShader = " << vertexShader << std::endl;
        std::cout << "fragmentShader = " << fragmentShader << std::endl;

        return {};
    }

    auto shaderHints = vsg::ShaderCompileSettings::create();
    std::vector<std::string>& defines = shaderHints->defines;

    vertexShader->module->hints = shaderHints;
    vertexShader->module->code = {};

    fragmentShader->module->hints = shaderHints;
    fragmentShader->module->code = {};

    // set up graphics pipeline
    vsg::DescriptorSetLayoutBindings descriptorBindings;

    // enable the point sprite code paths
    defines.push_back("VSG_POINT_SPRITE");

    vsg::ref_ptr<vsg::Data> textureData;
    //textureData = vsg::read_cast<vsg::Data>("textures/lz.vsgb", options);
    textureData = createParticleImage(64);
    if (textureData)
    {
        std::cout << "textureData = " << textureData << std::endl;

        // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
        defines.push_back("VSG_DIFFUSE_MAP");
    }

    {
        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{10, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
    }

    auto descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);

    vsg::DescriptorSetLayouts descriptorSetLayouts{descriptorSetLayout};

    vsg::PushConstantRanges pushConstantRanges{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection view, and model matrices, actual push constant calls automatically provided by the VSG's DispatchTraversal
    };

    auto pipelineLayout = vsg::PipelineLayout::create(descriptorSetLayouts, pushConstantRanges);

    vsg::VertexInputState::Bindings vertexBindingsDescriptions{
        VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // vertex data
    };

    vsg::VertexInputState::Attributes vertexAttributeDescriptions{
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, // vertex data
    };

    vertexBindingsDescriptions.push_back(VkVertexInputBindingDescription{1, sizeof(vsg::vec3), normalInputRate});  // normal data
    vertexAttributeDescriptions.push_back(VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}); // normal data

    vertexBindingsDescriptions.push_back(VkVertexInputBindingDescription{2, 4, colorInputRate});                 // color data
    vertexAttributeDescriptions.push_back(VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R8G8B8A8_UNORM, 0}); // color data

    auto rasterState = vsg::RasterizationState::create();

    bool blending = true;
    auto colorBlendState = vsg::ColorBlendState::create();
    colorBlendState->attachments = vsg::ColorBlendState::ColorBlendAttachments{
        {blending, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_SUBTRACT, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}};

    auto inputAssemblyState = vsg::InputAssemblyState::create();
    inputAssemblyState->topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    vsg::GraphicsPipelineStates pipelineStates{
        vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
        inputAssemblyState,
        rasterState,
        vsg::MultisampleState::create(),
        colorBlendState,
        vsg::DepthStencilState::create()};

    auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{vertexShader, fragmentShader}, pipelineStates);

    auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

    // create texture image and associated DescriptorSets and binding

    vsg::Descriptors descriptors;
    if (textureData)
    {
        auto sampler = vsg::Sampler::create();
        sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        auto texture = vsg::DescriptorImage::create(sampler, textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        descriptors.push_back(texture);
    }

    auto mat = vsg::PhongMaterialValue::create();
    mat->value().alphaMask = 1.0f;
    mat->value().alphaMaskCutoff = 0.99f;

    auto material = vsg::DescriptorBuffer::create(mat, 10);
    descriptors.push_back(material);

    auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, descriptors);
    auto bindDescriptorSets = vsg::BindDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, vsg::DescriptorSets{descriptorSet});

    auto sg = vsg::StateGroup::create();
    sg->add(bindGraphicsPipeline);
    sg->add(bindDescriptorSets);

    return sg;
}

PointSprites::PointSprites(uint32_t maxNumParticles, vsg::ref_ptr<const vsg::Options> options)
{
    vertices = vsg::vec3Array::create(maxNumParticles);
    normals = vsg::vec3Array::create(maxNumParticles);
    colors = vsg::ubvec4Array::create(maxNumParticles);

    bindVertexBuffers = vsg::BindVertexBuffers::create();
    bindVertexBuffers->arrays = { vertices, normals, colors };

    draw = vsg::Draw::create(0, 1, 0, 0);

    stateGroup = createPointSpriteStateGroup(options);
    stateGroup->addChild(bindVertexBuffers);
    stateGroup->addChild(draw);

    addChild(stateGroup);
}

PointSprites::PointSprites(const vsg::DataList& arrays, vsg::ref_ptr<const vsg::Options> options)
{
    if (arrays.size()>=1) vertices = arrays[0].cast<vsg::vec3Array>();
    if (arrays.size()>=2) normals = arrays[1].cast<vsg::vec3Array>();
    if (arrays.size()>=3) colors = arrays[2].cast<vsg::ubvec4Array>();

    bindVertexBuffers = vsg::BindVertexBuffers::create();
    bindVertexBuffers->arrays = arrays;

    draw = vsg::Draw::create(vertices->size(), 1, 0, 0);

    stateGroup = createPointSpriteStateGroup(options);
    stateGroup->addChild(bindVertexBuffers);
    stateGroup->addChild(draw);

    addChild(stateGroup);
}

void PointSprites::set(size_t i, const vsg::vec3& vertex, const vsg::vec3& normal, const vsg::vec4& color)
{
    vertices->set(i, vertex);
    normals->set(i, normal);
    colors->set(i, vsg::ubvec4(static_cast<uint8_t>(color.r * 255.0), static_cast<uint8_t>(color.g * 255.0), static_cast<uint8_t>(color.b * 255.0), static_cast<uint8_t>(color.a * 255.0)));
}

void PointSprites::set(size_t i, const vsg::vec3& vertex, const vsg::vec3& normal, const vsg::ubvec4& color)
{
    vertices->set(i, vertex);
    normals->set(i, normal);
    colors->set(i, color);
}
