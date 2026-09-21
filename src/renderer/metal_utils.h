#pragma once

#include <Metal/Metal.hpp>
#include <cstddef>

#include "platform/metal_ptr.h"

PipelineStatePtr CreateRenderPipelineState(MTL::Device* device,
                                           const char* vertex_function_name,
                                           const char* fragment_function_name);

BufferPtr CreateBuffer(MTL::Device* device, const void* data,
                       std::size_t length);

DepthStencilStatePtr CreateDepthStencilState(MTL::Device* device);
