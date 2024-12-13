# Clang AArch64 Cross-Compilation Toolchain File with libc++
# Notes: Clang is a cross compiler in itself. Therefore, the compiler on the host machine can be
# used to compile for the target. But only _compile_ not _link_. To link and generate executables,
# we need to have the libraries from the target machine to be locally available. Seems like it is
# not possible to apt install this. In the following example, I copied /usr/lib/llvm18 from a 
# raspberry pi and made it available for linking with lld. 

# Target architecture
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the cross-compiler
set(CROSS_TARGET "aarch64-linux-gnu")

# Compiler paths
set(LLVM_HOST_DIR "/usr/lib/llvm-20") # llvm location in the host machine
set(LLVM_TARGET_DIR "${CMAKE_SOURCE_DIR}/llvm18") # location of llvm toolchain for the target machine
set(LLVM_INCLUDE_DIR "${LLVM_HOST_DIR}/include/c++/v1") # headers from the host
set(LLVM_LIBRARY_DIR "${LLVM_TARGET_DIR}/lib") # link against libraries for target (copied from raspberry pi)

set(CMAKE_C_COMPILER   "${LLVM_HOST_DIR}/bin/clang")
set(CMAKE_CXX_COMPILER "${LLVM_HOST_DIR}/bin/clang++")

# compile flags. Compiler on the host machine can generate object files just fine
set(CROSS_COMPILE_FLAGS "-target ${CROSS_TARGET} -stdlib=libc++ -I${LLVM_INCLUDE_DIR}")
set(CMAKE_C_FLAGS_INIT   "${CROSS_COMPILE_FLAGS}" CACHE STRING "Initial C compiler flags")
set(CMAKE_CXX_FLAGS_INIT "${CROSS_COMPILE_FLAGS}" CACHE STRING "Initial C++ compiler flags")

# Linker flags. To generate binaries, link against libraries for target machine
set(CMAKE_EXE_LINKER_FLAGS_INIT 
    "-fuse-ld=lld --target=${CROSS_TARGET} -L${LLVM_LIBRARY_DIR} -lc++ -lc++abi" 
    CACHE STRING "Initial linker flags"
)

# Optional: Specify additional search paths for libraries and headers
set(CMAKE_FIND_ROOT_PATH "/usr/${CROSS_TARGET}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
