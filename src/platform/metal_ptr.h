#pragma once

#include <AppKit/AppKit.hpp>
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <utility>

template <class T>
using MetalPtr = NS::SharedPtr<T>;

using AutoreleasePoolPtr = MetalPtr<NS::AutoreleasePool>;
using DevicePtr = MetalPtr<MTL::Device>;
using CommandQueuePtr = MetalPtr<MTL::CommandQueue>;
using PipelineStatePtr = MetalPtr<MTL::RenderPipelineState>;
using DepthStencilStatePtr = MetalPtr<MTL::DepthStencilState>;
using BufferPtr = MetalPtr<MTL::Buffer>;
using WindowPtr = MetalPtr<NS::Window>;
using ViewPtr = MetalPtr<MTK::View>;

template <class T, class... Args>
MetalPtr<T> CreateMetalObject(Args&&... args) {
  return NS::TransferPtr(T::alloc()->init(std::forward<Args>(args)...));
}

[[nodiscard]]
inline NS::String* ToNsString(const char* text) {
  return NS::String::string(text, NS::StringEncoding::UTF8StringEncoding);
}
