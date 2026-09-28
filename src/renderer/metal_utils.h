#ifndef METAL_CPP_SAMPLE_RENDERER_METAL_UTILS_H_
#define METAL_CPP_SAMPLE_RENDERER_METAL_UTILS_H_

#include "platform/metal_ptr.h"
#include <Metal/Metal.hpp>
#include <cstddef>

PipelineStatePtr CreateRenderPipelineState(MTL::Device* device,
                                           const char* vertex_function_name,
                                           const char* fragment_function_name);

BufferPtr CreateBuffer(MTL::Device* device, const void* data,
                       std::size_t length);

BufferPtr CreateSharedBuffer(MTL::Device* device, std::size_t length);

DepthStencilStatePtr CreateDepthStencilState(MTL::Device* device);

#endif  // METAL_CPP_SAMPLE_RENDERER_METAL_UTILS_H_
