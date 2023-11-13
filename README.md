# ok-serializer

A binary data serializer and deserializer (or marshaller and unmarshaller, if you prefer)
for C++, that's quite OK.

The purpose of this library is not to provide an advanced framework to serialize and parse
well-defined objects, but to allow **working with binary data of all kinds** and different
source.

## Features

- **Header-only** library
- Built from the ground up using **C++23**.
- **Compile-time** type definitions for minimal overhead.
- Optional support for the C++ **reflection** technical specification.
- ⚠️ **Work in progress**: Expect undocumented info and broken interfaces.

## Example code

```cpp
using namespace okser;

/**
 * Simple serialization
 */
std::string data = simple_serialize<sint<1>, floatp>(63, 3.14f);

/**
 * More abstractions
 */
std::string result;

// A "bundle" defines the structure that needs to be serialised
using MyBundle = bundle<sint<1>, floatp>;
// An "output" defines where the data is going to be stored, a file, a string, a socket etc.
auto output = out::stdstring{result};

serialize<MyBundle>(output, 63, 3.14f);
```

### Example with reflection

If you have extra-fancy reflection features enabled in your compiler, then you could just do this:

```cpp
struct MyStruct {
    int8_t a = 104;     // a is serialized as a 1-byte signed integer
    uint16_t b = 26913; // b is serialized as a 2-byte unsigned integer, big-endian by default
};

MyStruct object;
std::string data = okser::serialize_struct_to_string(object); // data = "hi!"
```

## Getting started

⚠️ **Note:** This is a **very work-in-progress** library, expect breaking and frequent interface changes, things
not working, and a complete lack of documentation.

### Setting up the compiler

This library requires a recent implementation of C++23 which implements concepts, `std::expected` with monadic
operations and more. This means that, as of 2023, you will have to use one of these compilers:

- GCC 12
- Clang 17
- MSVC 19.36 (untested)

The optional **reflection** features are not implemented in any compiler so far. We are using a proof-of-concept
implementation in [matus-chochlik's llvm fork](https://github.com/matus-chochlik/llvm-project), further updated
with Clang 16 in [kongr45gpen/llvm-project](https://github.com/kongr45gpen/llvm-project).

You will need to compile this fork on your own. Here's a quick set of steps that should work for this:

```shell
# You can add --depth=1 to make the clone slightly faster
git clone https://github.com/kongr45gpen/llvm-project.git
cd llvm-project
cmake -S llvm -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi"
cmake --build build -- -j 8 # 8 parallel jobs
```

Keep in mind the location of the produced `clang++` executable in `build-2/bin`, and the produced libc++ library
files in `build/lib/x86_64-linux-gnu` or a similar folder.

### Compiling

`ok-serializer` is built as a CMake project. The easiest way to run a few quick tests with it would be to run
the following:

```shell
# Clone the repository
git clone https://github.com/kongr45gpen/ok-serializer.git
cd ok-serializer

# Initialize and build CMake project
mkdir build && cd build
cmake ..

# Compile the code (add --target empty_example to specify a target)
cmake --build .

# Run an example
example/empty_example
test/ok-serializer_test
```

If you are using a different compiler than the default, or you want to use the reflection extensions, run `cmake`
as such:

```shell
# Enables reflection extensions, sets the standard library to libstdc++ and fakes more recent concept support
# to enable <expected>
cmake -DCMAKE_CXX_COMPILER="/path/to/llvm-project/build/bin/clang++" --preset reflection ..
```

To execute the software, you will need to provide the path to the libc++ library files, as such:

```shell
LD_LIBRARY_PATH="/path/to/llvm-project/build/lib/x86_64-linux-gnu" ./example/empty_example
```

Alternatively, you can link the standard library statically to the generated executable, by running CMake with:

```shell
cmake -DCMAKE_CXX_COMPILER="/path/to/llvm-project/build/bin/clang++" --preset reflection-static ..
```

### Docker container

You can run all the above steps in a docker container that should include a compiled version of LLVM with reflection:

```shell
docker run -it kongr45gpen/llvm-reflection bash
```

## Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

## Documentation

Doxygen output available at https://kongr45gpen.github.io/ok-serializer/.

# Licensing

Licensed under MIT. See [LICENSE](LICENSE.txt) for details.
