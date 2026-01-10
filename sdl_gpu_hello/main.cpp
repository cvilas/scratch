#define SDL_MAIN_USE_CALLBACKS
#include <array>
#include <cstdint>
#include <memory>
#include <print>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// the vertex input layout
struct Vertex {
  float x, y, z;     // vec3 position
  float r, g, b, a;  // vec4 color
};

struct UniformBuffer {
  float time{};
};

struct Application {
  SDL_Window* window{ nullptr };
  SDL_GPUDevice* device{ nullptr };

  SDL_GPUBuffer* vertex_buffer{ nullptr };
  SDL_GPUTransferBuffer* transfer_buffer{ nullptr };
  SDL_GPUGraphicsPipeline* graphics_pipeline{ nullptr };
  UniformBuffer time_uniform;
};

//-------------------------------------------------------------------------------------------------
auto SDL_AppInit(void** appstate, int /*argc*/, char** /*argv*/) -> SDL_AppResult {
  static auto app = std::make_unique<Application>();
  *appstate = app.get();

  static constexpr auto WIN_W = 960U;
  static constexpr auto WIN_H = 540U;
  app->window = SDL_CreateWindow("Hello, Triangle!", WIN_W, WIN_H, SDL_WINDOW_RESIZABLE);
  if (app->window == nullptr) {
    std::println("SDL_CreateWindow failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  app->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
  if (app->device == nullptr) {
    std::println("SDL_CreateGPUDevice failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (not SDL_ClaimWindowForGPUDevice(app->device, app->window)) {
    std::println("SDL_ClaimWindowForGPUDevice failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // load and create vertex shader
  std::size_t vertex_code_size{ 0 };
  void* vertex_code = SDL_LoadFile("vertex.spv", &vertex_code_size);
  if (vertex_code == nullptr) {
    std::println("SDL_LoadFile(vertex) failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  auto vertex_info = SDL_GPUShaderCreateInfo{};
  vertex_info.code = static_cast<Uint8*>(vertex_code);
  vertex_info.code_size = vertex_code_size;
  vertex_info.entrypoint = "main";
  vertex_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
  vertex_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
  vertex_info.num_samplers = 0;
  vertex_info.num_storage_buffers = 0;
  vertex_info.num_storage_textures = 0;
  vertex_info.num_uniform_buffers = 0;
  auto* vertex_shader = SDL_CreateGPUShader(app->device, &vertex_info);
  SDL_free(vertex_code);
  if (vertex_shader == nullptr) {
    std::println("SDL_CreateGPUShader(vertex) failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // load and create fragment shader
  std::size_t fragment_code_size{ 0 };
  void* fragment_code = SDL_LoadFile("fragment.spv", &fragment_code_size);
  if (fragment_code == nullptr) {
    std::println("SDL_LoadFile(fragment) failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  auto fragment_info = SDL_GPUShaderCreateInfo{};
  fragment_info.code = static_cast<Uint8*>(fragment_code);
  fragment_info.code_size = fragment_code_size;
  fragment_info.entrypoint = "main";
  fragment_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
  fragment_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
  fragment_info.num_samplers = 0;
  fragment_info.num_storage_buffers = 0;
  fragment_info.num_storage_textures = 0;
  fragment_info.num_uniform_buffers = 1;
  auto* fragment_shader = SDL_CreateGPUShader(app->device, &fragment_info);
  SDL_free(fragment_code);
  if (fragment_shader == nullptr) {
    std::println("SDL_CreateGPUShader(fragment) failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // create the graphics pipeline
  auto pipeline_info = SDL_GPUGraphicsPipelineCreateInfo{};
  pipeline_info.vertex_shader = vertex_shader;
  pipeline_info.fragment_shader = fragment_shader;
  pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

  // describe the vertex buffers
  auto vertex_buffer_descriptions = std::array<SDL_GPUVertexBufferDescription, 1U>{};
  vertex_buffer_descriptions[0].slot = 0;
  vertex_buffer_descriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
  vertex_buffer_descriptions[0].instance_step_rate = 0;
  vertex_buffer_descriptions[0].pitch = sizeof(Vertex);

  pipeline_info.vertex_input_state.num_vertex_buffers = 1;
  pipeline_info.vertex_input_state.vertex_buffer_descriptions = vertex_buffer_descriptions.data();

  // describe the vertex attribute
  auto vertex_attributes = std::array<SDL_GPUVertexAttribute, 2U>{};

  // a_position
  vertex_attributes[0].buffer_slot = 0;
  vertex_attributes[0].location = 0;
  vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
  vertex_attributes[0].offset = 0;

  // a_color
  vertex_attributes[1].buffer_slot = 0;
  vertex_attributes[1].location = 1;
  vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
  vertex_attributes[1].offset = sizeof(float) * 3;

  pipeline_info.vertex_input_state.num_vertex_attributes = 2;
  pipeline_info.vertex_input_state.vertex_attributes = vertex_attributes.data();

  // describe the color target
  auto color_target_descriptions = std::array<SDL_GPUColorTargetDescription, 1U>{};
  color_target_descriptions[0] = {};
  color_target_descriptions[0].blend_state.enable_blend = true;
  color_target_descriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
  color_target_descriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
  color_target_descriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
  color_target_descriptions[0].blend_state.dst_color_blendfactor =
      SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
  color_target_descriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
  color_target_descriptions[0].blend_state.dst_alpha_blendfactor =
      SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
  color_target_descriptions[0].format = SDL_GetGPUSwapchainTextureFormat(app->device, app->window);

  pipeline_info.target_info.num_color_targets = 1;
  pipeline_info.target_info.color_target_descriptions = color_target_descriptions.data();

  // create the pipeline
  app->graphics_pipeline = SDL_CreateGPUGraphicsPipeline(app->device, &pipeline_info);
  if (app->graphics_pipeline == nullptr) {
    std::println("SDL_CreateGPUGraphicsPipeline failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_ReleaseGPUShader(app->device, vertex_shader);
  SDL_ReleaseGPUShader(app->device, fragment_shader);

  // a list of vertices in the order [top, bottom left, bottom right]
  static constexpr auto VERTICES = std::array<Vertex, 3U>{
    Vertex{ .x = 0.0F, .y = 0.5F, .z = 0.0F, .r = 1.0F, .g = 0.0F, .b = 0.0F, .a = 1.0F },
    Vertex{ .x = -0.5F, .y = -0.5F, .z = 0.0F, .r = 1.0F, .g = 1.0F, .b = 0.0F, .a = 1.0F },
    Vertex{ .x = 0.5F, .y = -0.5F, .z = 0.0F, .r = 1.0F, .g = 0.0F, .b = 1.0F, .a = 1.0F }
  };

  // create the vertex buffer
  auto buffer_info = SDL_GPUBufferCreateInfo{};
  buffer_info.size = sizeof(VERTICES);
  buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
  app->vertex_buffer = SDL_CreateGPUBuffer(app->device, &buffer_info);
  if (app->vertex_buffer == nullptr) {
    std::println("SDL_CreateGPUBuffer failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // create a transfer buffer to upload to the vertex buffer
  auto transfer_info = SDL_GPUTransferBufferCreateInfo{};
  transfer_info.size = sizeof(VERTICES);
  transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  app->transfer_buffer = SDL_CreateGPUTransferBuffer(app->device, &transfer_info);
  if (app->transfer_buffer == nullptr) {
    std::println("SDL_CreateGPUTransferBuffer failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // fill the transfer buffer
  auto* data =
      static_cast<Vertex*>(SDL_MapGPUTransferBuffer(app->device, app->transfer_buffer, false));
  SDL_memcpy(data, VERTICES.data(), sizeof(VERTICES));
  SDL_UnmapGPUTransferBuffer(app->device, app->transfer_buffer);

  // start a copy pass
  auto* command_buffer = SDL_AcquireGPUCommandBuffer(app->device);
  if (command_buffer == nullptr) {
    std::println("SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  auto* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

  // where is the data
  auto location = SDL_GPUTransferBufferLocation{};
  location.transfer_buffer = app->transfer_buffer;
  location.offset = 0;

  // where to upload the data
  auto region = SDL_GPUBufferRegion{};
  region.buffer = app->vertex_buffer;
  region.size = sizeof(VERTICES);
  region.offset = 0;

  // upload the data
  SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);

  // end the copy pass
  SDL_EndGPUCopyPass(copy_pass);
  if (not SDL_SubmitGPUCommandBuffer(command_buffer)) {
    std::println("SDL_SubmitGPUCommandBuffer failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppIterate(void* appstate) -> SDL_AppResult {
  auto* app = static_cast<Application*>(appstate);

  // acquire the command buffer
  auto* command_buffer = SDL_AcquireGPUCommandBuffer(app->device);
  if (command_buffer == nullptr) {
    std::println("SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());
    return SDL_APP_CONTINUE;
  }

  // get the swapchain texture
  SDL_GPUTexture* swapchain_texture{ nullptr };
  Uint32 width{};
  Uint32 height{};
  if (not SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, app->window, &swapchain_texture,
                                                &width, &height)) {
    std::println("SDL_WaitAndAcquireGPUSwapchainTexture failed: {}", SDL_GetError());
    SDL_SubmitGPUCommandBuffer(command_buffer);
    return SDL_APP_CONTINUE;
  }

  static constexpr auto CLEAR_COLOR = SDL_FColor{ .r = 0.94F, .g = 0.94F, .b = 0.94F, .a = 1.0F };
  auto color_target_info = SDL_GPUColorTargetInfo{};
  color_target_info.clear_color = CLEAR_COLOR;
  color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
  color_target_info.store_op = SDL_GPU_STOREOP_STORE;
  color_target_info.texture = swapchain_texture;

  auto* render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, nullptr);

  //----- DRAW CALLS BEGIN ----------

  // bind the pipeline
  SDL_BindGPUGraphicsPipeline(render_pass, app->graphics_pipeline);

  // bind the vertex buffer
  auto buffer_bindings = std::array<SDL_GPUBufferBinding, 1U>{};
  buffer_bindings[0].buffer = app->vertex_buffer;
  buffer_bindings[0].offset = 0;
  SDL_BindGPUVertexBuffers(render_pass, 0, buffer_bindings.data(), 1);

  // update the time uniform
  static constexpr auto NANO = 1E-9F;
  app->time_uniform.time = static_cast<float>(SDL_GetTicksNS()) * NANO;
  SDL_PushGPUFragmentUniformData(command_buffer, 0, &app->time_uniform, sizeof(UniformBuffer));

  // issue a draw call
  SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);

  //----- DRAW CALLS END ----------

  SDL_EndGPURenderPass(render_pass);
  if (not SDL_SubmitGPUCommandBuffer(command_buffer)) {
    std::println("SDL_SubmitGPUCommandBuffer failed: {}", SDL_GetError());
  }

  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppEvent(void* /*appstate*/, SDL_Event* event) -> SDL_AppResult {
  if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
    return SDL_APP_SUCCESS;
  }
  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/) {
  auto* app = static_cast<Application*>(appstate);
  if (app == nullptr) {
    return;
  }
  if (app->device != nullptr) {
    if (app->vertex_buffer != nullptr) {
      SDL_ReleaseGPUBuffer(app->device, app->vertex_buffer);
    }
    if (app->transfer_buffer != nullptr) {
      SDL_ReleaseGPUTransferBuffer(app->device, app->transfer_buffer);
    }
    if (app->graphics_pipeline != nullptr) {
      SDL_ReleaseGPUGraphicsPipeline(app->device, app->graphics_pipeline);
    }
    SDL_DestroyGPUDevice(app->device);
  }
  if (app->window != nullptr) {
    SDL_DestroyWindow(app->window);
  }
}
