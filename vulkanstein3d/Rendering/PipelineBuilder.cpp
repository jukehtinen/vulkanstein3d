#include "../Common.h"

#include "PipelineBuilder.h"
#include "SPIRV-Reflect/spirv_reflect.h"

#include <fstream>

namespace Rendering
{
PipelineBuilder PipelineBuilder::Builder()
{
    PipelineBuilder builder{};
    return builder;
}

PipelineBuilder::PipelineBuilder()
{
}

PipelineBuilder& PipelineBuilder::SetShaders(const std::string& vertFile, const std::string& fragFile)
{
    _vertFile = vertFile;
    _fragFile = fragFile;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetRasterization(vk::CullModeFlags cullMode, vk::FrontFace frontFace)
{
    _cullMode = cullMode;
    _frontFace = frontFace;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetDepthState(bool depthTest, bool depthWrite)
{
    _isDepthTest = depthTest;
    _isDepthWrite = depthWrite;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetBlend(bool enable)
{
    _isBlending = enable;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetRenderpass(vk::RenderPass renderPass)
{
    _renderPass = renderPass;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetDynamicState(const std::vector<vk::DynamicState>& dynamicState)
{
    _dynamicState = dynamicState;
    return *this;
}

std::shared_ptr<Pipeline> PipelineBuilder::Build(std::shared_ptr<Device> device)
{
    _device = device;

    auto vertShader = LoadShaderFile(_vertFile);
    auto fragShader = LoadShaderFile(_fragFile);

    ReflectVertexInput(vertShader);
    const vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{{}, _bindingDescriptions, _vertexAttributes};

    ReflectLayout(vertShader);
    ReflectLayout(fragShader);

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    for (const auto& [set, bindings] : _setBindings)
    {
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{{}, bindings};
        descriptorSetLayouts.push_back(device->Get().createDescriptorSetLayout(descriptorSetLayoutCreateInfo).value);
        _descriptorLayout = descriptorSetLayouts[0];
    }

    _pipelineLayout = device->Get().createPipelineLayout({{}, descriptorSetLayouts, _pushConstants}).value;

    std::vector stages = {
        vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eVertex, vertShader.shaderModule, "main"},
        vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eFragment, fragShader.shaderModule, "main"},
    };

    const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{{}, vk::PrimitiveTopology::eTriangleList};

    const vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{{}, 1, nullptr, 1, nullptr};

    const vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{{}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, _cullMode, _frontFace, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f};

    const vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{{}, vk::SampleCountFlagBits::e1};

    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
    if (_isBlending)
    {
        vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachments.push_back(colorBlendAttachment);
    }
    else
    {
        vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachments.push_back(colorBlendAttachment);
    }

    const vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{{}, VK_FALSE, vk::LogicOp::eCopy, colorBlendAttachments, {}};

    const vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{{}, _isDepthTest, _isDepthWrite, vk::CompareOp::eLess, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1.0f};

    const vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{{}, _dynamicState};

    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.setStages(stages);
    graphicsPipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo);
    graphicsPipelineCreateInfo.setPInputAssemblyState(&inputAssemblyStateCreateInfo);
    graphicsPipelineCreateInfo.setPViewportState(&viewportStateCreateInfo);
    graphicsPipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo);
    graphicsPipelineCreateInfo.setPMultisampleState(&multisampleStateCreateInfo);
    graphicsPipelineCreateInfo.setPDepthStencilState(&depthStencilStateCreateInfo);
    graphicsPipelineCreateInfo.setPColorBlendState(&colorBlendStateCreateInfo);
    graphicsPipelineCreateInfo.setPDynamicState(&dynamicStateCreateInfo);
    graphicsPipelineCreateInfo.setLayout(_pipelineLayout);
    graphicsPipelineCreateInfo.setRenderPass(_renderPass);
    graphicsPipelineCreateInfo.setSubpass(0);

    auto [result, pipeline] = _device->Get().createGraphicsPipeline({}, graphicsPipelineCreateInfo);

    _device->Get().destroyShaderModule(vertShader.shaderModule);
    _device->Get().destroyShaderModule(fragShader.shaderModule);
    spvReflectDestroyShaderModule(vertShader.reflectInfo);
    spvReflectDestroyShaderModule(fragShader.reflectInfo);
    delete vertShader.reflectInfo;
    delete fragShader.reflectInfo;

    return std::make_shared<Pipeline>(pipeline, _pipelineLayout, _descriptorLayout);
}

void PipelineBuilder::ReflectVertexInput(Shader& shader)
{
    if (shader.reflectInfo->shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
    {
        std::vector<SpvReflectInterfaceVariable*> inputVars(shader.reflectInfo->input_variable_count);
        spvReflectEnumerateInputVariables(shader.reflectInfo, &shader.reflectInfo->input_variable_count, inputVars.data());

        // Remove built-ins (gl_VertexIndex etc.)
        inputVars.erase(std::remove_if(inputVars.begin(), inputVars.end(), [](SpvReflectInterfaceVariable* inputVar) { return inputVar->built_in != -1; }), inputVars.end());

        vk::VertexInputBindingDescription bindingDescription{};

        _vertexAttributes.resize(inputVars.size());
        for (size_t i = 0; i < inputVars.size(); ++i)
        {
            _vertexAttributes[i].location = inputVars[i]->location;
            _vertexAttributes[i].binding = bindingDescription.binding;
            _vertexAttributes[i].format = static_cast<vk::Format>(inputVars[i]->format);
            _vertexAttributes[i].offset = 0; // final offset computed below after sorting.
        }
        // Sort attributes by location
        std::sort(std::begin(_vertexAttributes), std::end(_vertexAttributes),
                  [](const vk::VertexInputAttributeDescription& a, const vk::VertexInputAttributeDescription& b) { return a.location < b.location; });

        // Compute final offsets of each attribute, and total vertex stride.
        for (auto& attribute : _vertexAttributes)
        {
            // Check sizes from https://github.com/KhronosGroup/SPIRV-Reflect/blob/4689b3360cb38283b67926a065a0f2cc285928b5/examples/main_io_variables.cpp#L14
            uint32_t formatSize = 0;
            switch (attribute.format)
            {
            case vk::Format::eR32G32Sfloat:
                formatSize = 8;
                break;
            case vk::Format::eR32G32B32Sfloat:
                formatSize = 12;
                break;
            case vk::Format::eR32G32B32A32Sfloat:
                formatSize = 16;
                break;
            default:
                spdlog::error("Unknown vk::Format size: {}", vk::to_string(attribute.format));
                break;
            }

            attribute.offset = bindingDescription.stride;
            bindingDescription.stride += formatSize;
        }

        _bindingDescriptions.push_back(bindingDescription);
    }
}

void PipelineBuilder::ReflectLayout(Shader& shader)
{
    std::vector<SpvReflectDescriptorSet*> sets(shader.reflectInfo->descriptor_set_count);
    spvReflectEnumerateDescriptorSets(shader.reflectInfo, &shader.reflectInfo->descriptor_set_count, sets.data());

    for (size_t i = 0; i < sets.size(); i++)
    {
        const SpvReflectDescriptorSet& refl_set = *(sets[i]);

        std::vector<vk::DescriptorSetLayoutBinding> bindings;

        auto iter = _setBindings.find(refl_set.set);
        if (iter != _setBindings.end())
            bindings = iter->second;

        for (uint32_t b = 0; b < refl_set.binding_count; b++)
        {
            const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[b]);

            vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = refl_binding.binding;
            layoutBinding.descriptorType = static_cast<vk::DescriptorType>(refl_binding.descriptor_type);
            layoutBinding.descriptorCount = 1;
            for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim)
            {
                layoutBinding.descriptorCount *= refl_binding.array.dims[i_dim];
            }
            layoutBinding.stageFlags = static_cast<vk::ShaderStageFlagBits>(shader.reflectInfo->shader_stage);

            bindings.push_back(layoutBinding);
        }

        _setBindings[refl_set.set] = bindings;
    }

    std::vector<SpvReflectBlockVariable*> pushConstants(shader.reflectInfo->push_constant_block_count);
    spvReflectEnumeratePushConstantBlocks(shader.reflectInfo, &shader.reflectInfo->push_constant_block_count, pushConstants.data());
    for (uint32_t i = 0; i < pushConstants.size(); i++)
    {
        _pushConstants.push_back(vk::PushConstantRange{static_cast<vk::ShaderStageFlagBits>(shader.reflectInfo->shader_stage),
                                                       pushConstants[i]->offset, pushConstants[i]->size});
    }
}

Shader PipelineBuilder::LoadShaderFile(const std::string& shaderFile)
{
    std::ifstream file(shaderFile, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        return {};

    size_t fileSize = (size_t)file.tellg();

    std::vector<uint32_t> codeBuffer(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read((char*)codeBuffer.data(), fileSize);

    auto [result, shaderModule] = _device->Get().createShaderModule({{}, codeBuffer});

    if (result != vk::Result::eSuccess)
    {
        spdlog::error("[Vulkan] LoadShaderFile: {}", result);
        return {};
    }

    auto reflection = new SpvReflectShaderModule;
    if (spvReflectCreateShaderModule(fileSize, codeBuffer.data(), reflection) != SPV_REFLECT_RESULT_SUCCESS)
    {
        spdlog::error("[Vulkan] spvReflectCreateShaderModule failed");
        return {};
    }

    return {shaderModule, reflection};
}

} // namespace Rendering
