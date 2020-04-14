from conans import ConanFile, CMake, tools

'''
This recipe does the equivalent of:
- git clone https://github.com/foonathan/memory.git
- cd memory
- git checkout v0.6-2
- git submodule update --init --recursive
- mkdir build && cd build
- cmake -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DFOONATHAN_MEMORY_BUILD_EXAMPLES=OFF \
  -DFOONATHAN_MEMORY_BUILD_TESTS=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON ..
  make && make install
'''
class FoonathanmemoryConan(ConanFile):
    name = "foonathan_memory"
    version = "0.6.2"
    license = "zlib"
    author = "Vilas Chitrakaran chitrakaran@arrival.com"
    url = "https://github.com/cvilas/conan-foonathan-memory"
    description = "STL compatible C++ memory allocator library."
    topics = ("allocator", "memory", "fast-rtps")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": True}
    generators = "cmake"

    def source(self):
        git = tools.Git()
        git.clone("https://github.com/foonathan/memory.git", "v0.6-2")
        git.run("submodule update --init --recursive")
        
    def configure_cmake(self):
        cmake = CMake(self, parallel=True)
        cmake.definitions["FOONATHAN_MEMORY_BUILD_EXAMPLES"] = False
        cmake.definitions["FOONATHAN_MEMORY_BUILD_TESTS"] = False
        cmake.configure()
        return cmake
    
    def build(self):
        cmake = self.configure_cmake()
        cmake.build()
        cmake.install()

    def package(self):
        pass
    
    def package_info(self):
        self.cpp_info.libs = ["foonathan_memory"]
