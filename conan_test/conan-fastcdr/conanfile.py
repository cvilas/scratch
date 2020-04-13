from conans import ConanFile, CMake, tools


class FastcdrConan(ConanFile):
    name = "fastcdr"
    version = "1.0.13"
    license = "Apache License 2.0"
    author = "Vilas Chitrakaran chitrakaran@arrival.com"
    url = "https://git.tra.ai/robohive/conan-fastcdr"
    description = "Fast serialisation using CDR standard"
    topics = ("omg", "cdr", "fast-rtps")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": True}
    generators = "cmake"

    def source(self):
        git = tools.Git()
        git.clone("https://github.com/eProsima/Fast-CDR.git", "v1.0.13")

    def build(self):
        cmake = CMake(self,parallel=True)
        cmake.configure(source_folder="")
        cmake.build()
        cmake.install()

    def package(self):
        pass
    
    def package_info(self):
        self.cpp_info.libs = ["fastcdr"]

