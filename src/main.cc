#include <simd/simd.h>

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <cassert>
#include <cstring>
#include <iostream>

constexpr const char* kShaderLibraryPath = "build/basic.metallib";

#pragma region Renderer {
class Renderer {
 public:
  explicit Renderer(MTL::Device* device);
  ~Renderer();
  void Draw(MTK::View* view);

 private:
  MTL::Device* device_;
  MTL::CommandQueue* command_queue_;
  MTL::RenderPipelineState* pipeline_state_;
  MTL::Buffer* positions_buffer_;
  MTL::Buffer* colors_buffer_;
  size_t vertex_count_;
};

Renderer::Renderer(MTL::Device* device) : device_(device->retain()) {
  using NS::StringEncoding::UTF8StringEncoding;

  command_queue_ = device_->newCommandQueue();

  assert(command_queue_ != nullptr && "Failed to create command queue.");
  std::cout << "Command queue created.\n";
  const char* device_name = device_->name()->utf8String();
  std::cout << "GPU device name: '" << device_name << "'.\n";

  NS::Error* shader_library_error = nullptr;
  NS::URL* shader_library_url = NS::URL::fileURLWithPath(
      NS::String::string(kShaderLibraryPath, UTF8StringEncoding));
  MTL::Library* shader_library =
      device_->newLibrary(shader_library_url, &shader_library_error);

  assert(shader_library != nullptr && "Failed to create shader library.");
  std::cout << "Shader library created.\n";

  MTL::Function* vertex_main = shader_library->newFunction(
      NS::String::string("VertexMain", UTF8StringEncoding));

  assert(vertex_main != nullptr && "Failed to create shader vertex function.");
  std::cout << "Shader vertex function created.\n";

  MTL::Function* fragment_main = shader_library->newFunction(
      NS::String::string("FragmentMain", UTF8StringEncoding));

  assert(fragment_main != nullptr &&
         "Failed to create shader fragment function.");
  std::cout << "Shader fragment function created.\n";

  MTL::RenderPipelineDescriptor* pipeline_descriptor =
      MTL::RenderPipelineDescriptor::alloc()->init();

  pipeline_descriptor->setVertexFunction(vertex_main);
  pipeline_descriptor->setFragmentFunction(fragment_main);
  pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

  NS::Error* pipeline_state_error = nullptr;
  pipeline_state_ = device_->newRenderPipelineState(pipeline_descriptor,
                                                    &pipeline_state_error);
  assert(pipeline_state_ != nullptr &&
         "Failed to create render pipeline state.");
  std::cout << "Render pipeline state created.\n";

  std::array positions{simd::float3{-0.675F, 0.675F, 0.0F},
                       simd::float3{0.0F, -0.675F, 0.0F},
                       simd::float3{+0.675F, 0.675F, 0.0F}};

  vertex_count_ = std::size(positions);

  std::array colors{simd::float3{1.0F, 0.0F, 0.0F},
                    simd::float3{0.0F, 1.0F, 0.0F},
                    simd::float3{0.0F, 0.0F, 1.0F}};

  MTL::ResourceOptions storage_mode = device->hasUnifiedMemory()
                                          ? MTL::ResourceStorageModeShared
                                          : MTL::ResourceStorageModeManaged;

  positions_buffer_ = device_->newBuffer(sizeof(positions), storage_mode);
  assert(positions_buffer_ != nullptr && "Failed to create positions buffer.");
  memcpy(positions_buffer_->contents(), positions.data(), sizeof(positions));
  std::cout << "Positions buffer created.\n";

  colors_buffer_ = device_->newBuffer(sizeof(colors), storage_mode);
  assert(colors_buffer_ != nullptr && "Failed to create colors buffer.");
  memcpy(colors_buffer_->contents(), colors.data(), sizeof(colors));
  std::cout << "Colors buffer created.\n";

  if (!device_->hasUnifiedMemory()) {
    positions_buffer_->didModifyRange(
        NS::Range::Make(0, positions_buffer_->length()));
    colors_buffer_->didModifyRange(
        NS::Range::Make(0, colors_buffer_->length()));
  }

  pipeline_descriptor->release();
  fragment_main->release();
  vertex_main->release();
  shader_library->release();
}

Renderer::~Renderer() {
  colors_buffer_->release();
  positions_buffer_->release();
  pipeline_state_->release();
  command_queue_->release();
  device_->release();
}

void Renderer::Draw(MTK::View* view) {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer* command_buffer = command_queue_->commandBuffer();
  MTL::RenderPassDescriptor* render_pass = view->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* command_encoder =
      command_buffer->renderCommandEncoder(render_pass);

  command_encoder->setRenderPipelineState(pipeline_state_);
  command_encoder->setVertexBuffer(positions_buffer_, 0, 0);
  command_encoder->setVertexBuffer(colors_buffer_, 0, 1);
  command_encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                  static_cast<NS::UInteger>(0),
                                  static_cast<NS::UInteger>(vertex_count_));

  command_encoder->endEncoding();
  command_buffer->presentDrawable(view->currentDrawable());
  command_buffer->commit();

  pool->release();
}
#pragma endregion Renderer }

int main() { return 0; }
