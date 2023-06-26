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

## Getting started

⚠️ **Note:** This is a **very work-in-progress** library, expect breaking and frequent interface changes, things
not working, and a complete lack of documentation.

### Setting up the compiler

_TODO set up compiler for reflection_

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
cmake --build . -j4

# Run an example
example/empty_example
test/ok-serializer_test
```

## Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

# Licensing

Licensed under MIT. See [LICENSE](LICENSE) for details.
