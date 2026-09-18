#include <simd/simd.h>

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <cassert>
#include <cstring>
#include <iostream>

constexpr const char* kShaderLibraryPath = "build/basic.metallib";
constexpr const char* kWindowTitle = "Metal-cpp Sample";
constexpr const char* kVertexShaderFunctionName = "VertexMain";
constexpr const char* kFragmentShaderFunctionName = "FragmentMain";

// Must match [[buffer(n)]] indices in shaders/basic.metal (this is not checked
// by the compiler)
constexpr NS::UInteger kPositionsBufferIndex = 0;
constexpr NS::UInteger kColorsBufferIndex = 1;
constexpr MTL::PixelFormat kColorPixelFormat =
    MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB;

class Renderer {
 public:
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;
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
  command_queue_ = device_->newCommandQueue();
  assert(command_queue_ != nullptr && "Failed to create command queue.");

  NS::Error* shader_library_error = nullptr;
  NS::URL* shader_library_url = NS::URL::fileURLWithPath(NS::String::string(
      kShaderLibraryPath, NS::StringEncoding::UTF8StringEncoding));

  MTL::Library* shader_library =
      device_->newLibrary(shader_library_url, &shader_library_error);
  assert(shader_library != nullptr && "Failed to create shader library.");

  MTL::Function* vertex_main = shader_library->newFunction(NS::String::string(
      kVertexShaderFunctionName, NS::StringEncoding::UTF8StringEncoding));
  assert(vertex_main != nullptr && "Failed to create shader vertex function.");

  MTL::Function* fragment_main = shader_library->newFunction(NS::String::string(
      kFragmentShaderFunctionName, NS::StringEncoding::UTF8StringEncoding));
  assert(fragment_main != nullptr &&
         "Failed to create shader fragment function.");

  MTL::RenderPipelineDescriptor* pipeline_descriptor =
      MTL::RenderPipelineDescriptor::alloc()->init();

  pipeline_descriptor->setVertexFunction(vertex_main);
  pipeline_descriptor->setFragmentFunction(fragment_main);
  pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(
      kColorPixelFormat);

  NS::Error* pipeline_state_error = nullptr;
  pipeline_state_ = device_->newRenderPipelineState(pipeline_descriptor,
                                                    &pipeline_state_error);
  assert(pipeline_state_ != nullptr &&
         "Failed to create render pipeline state.");

  std::array positions{simd::float3{-0.675F, 0.675F, 0.0F},
                       simd::float3{0.0F, -0.675F, 0.0F},
                       simd::float3{+0.675F, 0.675F, 0.0F}};

  vertex_count_ = std::size(positions);

  std::array colors{simd::float3{1.0F, 0.0F, 0.0F},
                    simd::float3{0.0F, 1.0F, 0.0F},
                    simd::float3{0.0F, 0.0F, 1.0F}};

  // NOTE: ResourceStorageModeManaged path is untested (I don't have access to
  // a Mac with dedicated graphics..!)
  MTL::ResourceOptions storage_mode = device->hasUnifiedMemory()
                                          ? MTL::ResourceStorageModeShared
                                          : MTL::ResourceStorageModeManaged;

  positions_buffer_ = device_->newBuffer(sizeof(positions), storage_mode);
  assert(positions_buffer_ != nullptr && "Failed to create positions buffer.");
  memcpy(positions_buffer_->contents(), positions.data(), sizeof(positions));

  colors_buffer_ = device_->newBuffer(sizeof(colors), storage_mode);
  assert(colors_buffer_ != nullptr && "Failed to create colors buffer.");
  memcpy(colors_buffer_->contents(), colors.data(), sizeof(colors));

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
  command_encoder->setVertexBuffer(positions_buffer_, 0, kPositionsBufferIndex);
  command_encoder->setVertexBuffer(colors_buffer_, 0, kColorsBufferIndex);
  command_encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                  static_cast<NS::UInteger>(0),
                                  static_cast<NS::UInteger>(vertex_count_));

  command_encoder->endEncoding();
  command_buffer->presentDrawable(view->currentDrawable());
  command_buffer->commit();

  pool->release();
}

class ViewDelegate : public MTK::ViewDelegate {
 public:
  ViewDelegate(const ViewDelegate&) = delete;
  ViewDelegate& operator=(const ViewDelegate&) = delete;
  explicit ViewDelegate(MTL::Device* device);
  ~ViewDelegate() override;
  void drawInMTKView(MTK::View* view) override;

 private:
  Renderer* renderer_;
};

ViewDelegate::ViewDelegate(MTL::Device* device)
    : renderer_(new Renderer(device)) {}

ViewDelegate::~ViewDelegate() { delete renderer_; }

void ViewDelegate::drawInMTKView(MTK::View* view) { renderer_->Draw(view); }

class ApplicationDelegate : public NS::ApplicationDelegate {
 public:
  ApplicationDelegate() = default;
  ApplicationDelegate(const ApplicationDelegate&) = delete;
  ApplicationDelegate& operator=(const ApplicationDelegate&) = delete;
  ~ApplicationDelegate() override;
  void applicationWillFinishLaunching(NS::Notification* notification) override;
  void applicationDidFinishLaunching(NS::Notification* notification) override;
  bool applicationShouldTerminateAfterLastWindowClosed(
      NS::Application* sender) override;

 private:
  MTL::Device* device_;
  NS::Window* window_;
  MTK::View* view_;
  ViewDelegate* view_delegate_;
};

ApplicationDelegate::~ApplicationDelegate() {
  view_->release();
  window_->release();
  device_->release();
  delete view_delegate_;
}

void ApplicationDelegate::applicationWillFinishLaunching(
    NS::Notification* notification) {
  NS::Application::sharedApplication()->setActivationPolicy(
      NS::ActivationPolicyRegular);
}

void ApplicationDelegate::applicationDidFinishLaunching(
    NS::Notification* notification) {
  device_ = MTL::CreateSystemDefaultDevice();
  assert(device_ != nullptr && "Failed to create device.");

  CGRect content_rect = (CGRect){{128.0, 128.0}, {1024.0, 1024.0}};
  NS::WindowStyleMask window_style_mask =
      NS::WindowStyleMaskTitled | NS::WindowStyleMaskClosable;
  NS::BackingStoreType window_backing = NS::BackingStoreBuffered;
  bool defer_onscreen_allocation = false;

  window_ =
      NS::Window::alloc()->init(content_rect, window_style_mask, window_backing,
                                defer_onscreen_allocation);
  assert(window_ != nullptr && "Failed to create window.");

  view_ = MTK::View::alloc()->init(content_rect, device_);
  assert(view_ != nullptr && "Failed to create view.");

  view_->setColorPixelFormat(kColorPixelFormat);
  view_->setClearColor(MTL::ClearColor::Make(0.0, 0.0, 0.0, 1.0));

  view_delegate_ = new ViewDelegate(device_);
  view_->setDelegate(view_delegate_);

  window_->setContentView(view_);
  window_->setTitle(
      NS::String::string(kWindowTitle, NS::StringEncoding::UTF8StringEncoding));
  window_->makeKeyAndOrderFront(nullptr);

  NS::Application::sharedApplication()->activateIgnoringOtherApps(true);

  const char* device_name = device_->name()->utf8String();
  std::cout << "'" << kWindowTitle << "' running using " << device_name << " ("
            << (device_->hasUnifiedMemory() ? "Integrated" : "Dedicated")
            << " GPU)\n";
}

bool ApplicationDelegate::applicationShouldTerminateAfterLastWindowClosed(
    NS::Application* sender) {
  return true;
}

int main() {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

  ApplicationDelegate application_delegate;

  NS::Application* shared_application = NS::Application::sharedApplication();
  shared_application->setDelegate(&application_delegate);
  shared_application->run();

  pool->release();
  return 0;
}
