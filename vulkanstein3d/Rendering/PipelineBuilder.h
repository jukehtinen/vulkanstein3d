#pragma once

#include "Device.h"

#include <map>
#include <string>

struct SpvReflectShaderModule;

namespace Rendering
{
class Device;

struct Shader
{
    vk::ShaderModule shaderModule{};
    SpvReflectShaderModule* reflectInfo{nullptr};
};

class Pipeline
{
  public:
    vk::Pipeline pipeline{};
    vk::PipelineLayout pipelineLayout{};
    vk::DescriptorSetLayout descriptorSetLayout{};
};

class PipelineBuilder
{
  public:
    static PipelineBuilder Builder();

    PipelineBuilder& SetShaders(const std::string& vertFile, const std::string& fragFile);
    PipelineBuilder& SetDepthState(bool depthTest, bool depthWrite);
    PipelineBuilder& SetBlend(bool enable);
    PipelineBuilder& SetDynamicState(const std::vector<vk::DynamicState>& dynamicState);
    PipelineBuilder& SetRasterization(vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack, vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise);

    std::shared_ptr<Pipeline> Build(std::shared_ptr<Device> device);

  private:
    PipelineBuilder();

    Shader LoadShaderFile(const std::string& shaderFile);
    void ReflectVertexInput(Shader& shader);
    void ReflectLayout(Shader& shader);

  private:
    std::shared_ptr<Device> _device;    
    vk::PipelineLayout _pipelineLayout{};
    vk::DescriptorSetLayout _descriptorLayout{};

    std::string _vertFile;
    std::string _fragFile;

    std::vector<vk::VertexInputBindingDescription> _bindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> _vertexAttributes;
    std::map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> _setBindings;
    std::vector<vk::PushConstantRange> _pushConstants;

    bool _isDepthTest{true};
    bool _isDepthWrite{true};
    bool _isBlending{false};
    vk::CullModeFlags _cullMode{vk::CullModeFlagBits::eBack};
    vk::FrontFace _frontFace{vk::FrontFace::eCounterClockwise};
    std::vector<vk::DynamicState> _dynamicState{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
};
} // namespace Rendering