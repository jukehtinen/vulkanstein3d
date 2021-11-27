#include "PipelineBuilder.h"

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

PipelineBuilder& PipelineBuilder::SetVertexInput(const std::vector<vk::VertexInputBindingDescription>& bindingDescriptions, const std::vector<vk::VertexInputAttributeDescription>& attributes)
{
    _bindingDescriptions = bindingDescriptions;
    _vertexAttributes = attributes;
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

PipelineBuilder& PipelineBuilder::SetPipelineLayout(vk::PipelineLayout layout)
{
    _pipelineLayout = layout;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetPushConstants(uint32_t size)
{
    _pushConstantSize = size;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetDynamicState(const std::vector<vk::DynamicState>& dynamicState)
{
    _dynamicState = dynamicState;
    return *this;
}

Pipeline PipelineBuilder::Build(std::shared_ptr<Device> device)
{
    _device = device;

    std::vector bindings{vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr}};

    std::vector<vk::DescriptorBindingFlags> descriptorBindingFlags = {vk::DescriptorBindingFlagBits::eVariableDescriptorCount};

    const vk::DescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo{descriptorBindingFlags};

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{{}, bindings};
    descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;

    auto descriptorSetLayout = device->Get().createDescriptorSetLayout(descriptorSetLayoutCreateInfo).value;

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{descriptorSetLayout};
    std::vector<vk::PushConstantRange> pushConstantRanges;

    if (_pushConstantSize > 0)
    {
        pushConstantRanges.push_back(vk::PushConstantRange{vk::ShaderStageFlagBits::eAllGraphics, 0, _pushConstantSize});
    }

    _pipelineLayout = _device->Get().createPipelineLayout({{}, descriptorSetLayouts, pushConstantRanges}).value;

    const vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{{}, _bindingDescriptions, _vertexAttributes};

    auto vertShaderModule = LoadShaderFile(_vertFile);
    auto fragShaderModule = LoadShaderFile(_fragFile);

    std::vector stages = {
        vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main"},
        vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main"},
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

    _device->Get().destroyShaderModule(vertShaderModule);
    _device->Get().destroyShaderModule(fragShaderModule);

    return Pipeline{pipeline, _pipelineLayout, descriptorSetLayout};
}

vk::ShaderModule PipelineBuilder::LoadShaderFile(const std::string& shaderFile)
{
    std::ifstream file(shaderFile, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        return {};

    size_t fileSize = (size_t)file.tellg();

    std::vector<uint32_t> codeBuffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read((char*)codeBuffer.data(), fileSize);

    auto [result, shaderModule] = _device->Get().createShaderModule({{}, codeBuffer});

    return shaderModule;
}

} // namespace Rendering
