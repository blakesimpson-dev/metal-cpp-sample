#pragma once

#include <Metal/Metal.hpp>
#include <cstddef>

// NOTE: Caller owns the returned pipeline state and must release() it
MTL::RenderPipelineState* CreateRenderPipelineState(
    MTL::Device* device, const char* vertex_function_name,
    const char* fragment_function_name);

// NOTE: Copies into new buffer, caller owns the result and must release() it
MTL::Buffer* CreateBuffer(MTL::Device* device, const void* data,
                          std::size_t length);
