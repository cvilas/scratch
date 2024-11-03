// Camera interface using libcamera
// - `sudo apt install libcamera-dev libcamera-tools`
// - test camera with `qcam`
// - See https://git.libcamera.org/libcamera/simple-cam.git/tree/simple-cam.cpp
// 
// TODO
// - Capture image
// - encode to jpeg
// - publish
// - subscribe
// - show on GLFW window
// - error handling for all calls

#include <cstdlib>
#include <thread>
#include <print>

#include <libcamera/libcamera.h>

static std::shared_ptr<libcamera::Camera> camera;

static void requestComplete(libcamera::Request *request) {
  if (request->status() == libcamera::Request::RequestCancelled)
   return;

  const std::map<const libcamera::Stream*, libcamera::FrameBuffer*> &buffers = request->buffers();
  for (auto buffer_pair : buffers) {
    auto* buffer = buffer_pair.second;
    const auto& metadata = buffer->metadata();

    std::print(" seq: {}  bytes used: ", metadata.sequence);
    auto nplane = 0u;
    for (const auto& plane : metadata.planes()) {
      std::print("{}", plane.bytesused);
      if (++nplane < metadata.planes().size()) {
        std::print("/");
      }
    }
    std::println("");
  }
  request->reuse(libcamera::Request::ReuseBuffers);
  camera->queueRequest(request);
}

auto main() -> int {

  // start camera manager
  auto cm = std::make_unique<libcamera::CameraManager>();
  cm->start();
  
  // enumerate cameras
  auto cameras = cm->cameras();
  if (cameras.empty()) {
    std::println("No cameras found.");
    cm->stop();
    return EXIT_FAILURE;
  }
  
  // list available cameras
  for(auto const& camera : cameras) {
    std::println("{}", camera->id());
  }

  // pick the first one
  auto camera_id = cameras[0]->id();
  camera = cm->get(camera_id);
  camera->acquire();

  // Generate a default camera configuration for view finder role we want to use the camera for
  auto config = camera->generateConfiguration( { libcamera::StreamRole::Viewfinder } );
  auto& stream_config = config->at(0);
  std::println("Default viewfinder configuration is: {}", stream_config.toString());

  // change and validate configuration
  stream_config.size.width = 640;
  stream_config.size.height = 480;
  config->validate();
  std::println("Validated viewfinder configuration is: {}", stream_config.toString());

  // apply the configuration
  camera->configure(config.get());
  
  // allocate framebuffers
  auto* allocator = new libcamera::FrameBufferAllocator(camera);
  for (auto &cfg : *config) {
    int ret = allocator->allocate(cfg.stream());
    if (ret < 0) {
        std::println("Can't allocate buffers");
        return -ENOMEM;
    }
    auto allocated = allocator->buffers(cfg.stream()).size();
    std::println("Allocated {} buffers for stream", allocated);
  }

  // create request per buffer
  auto* stream = stream_config.stream();
  const auto& buffers = allocator->buffers(stream);

  std::vector<std::unique_ptr<libcamera::Request>> requests;
  for (auto i = 0u; i < buffers.size(); ++i) {
    auto request = camera->createRequest();
    if (!request) {
      std::println("Can't create request");
      return -ENOMEM;
    }

    const auto& buffer = buffers[i];
    auto ret = request->addBuffer(stream, buffer.get());
    if (ret < 0) {
      std::println("Can't set buffer for request");
      return ret;
    }
    requests.push_back(std::move(request));
  }
  
  // setup request completion handler
  camera->requestCompleted.connect(requestComplete);

  // start queueing requests
  camera->start();
  for (auto &request : requests) {
    camera->queueRequest(request.get());
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(3000));

  // cleanup and exit
  camera->stop();
  allocator->free(stream);
  delete allocator;
  camera->release();
  camera.reset();
  cm->stop();

  return EXIT_SUCCESS;
}
