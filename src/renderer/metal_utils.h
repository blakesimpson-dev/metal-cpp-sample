#pragma once

#include <Metal/Metal.hpp>
#include <cstddef>

MTL::RenderPipelineState* CreateRenderPipelineState(
    MTL::Device* device, const char* vertex_function_name,
    const char* fragment_function_name);

MTL::Buffer* CreateBuffer(MTL::Device* device, const void* data,
                          std::size_t length);

MTL::DepthStencilState* CreateDepthStencilState(MTL::Device* device);
