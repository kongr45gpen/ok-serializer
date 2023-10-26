@page simple-types Simple Types
@section simple-types-integers Integers

The simplest supported types are integers.

Unsigned integers are represented by @ref okser::uint.

```cpp
using namespace okser;

uint8_t number = 3;
serialize_to_string<uint<1>>(number);
// Output: 0x03

uint16_t number = 999;
serialize_to_string<uint<2>>(number);
// Output: 0x03E7

uint32_t number = 999999;
serialize_to_string<uint<4>>(number);
// Output: 0x000F423F
```

The number of `Bytes` used to represent the integer is not deducted automatically, to
prevent mismatches in case the variable size is not clear.

Signed integers (two's complement) are represented by @ref okser::sint.

```cpp
using namespace okser;

int16_t number = -999;
serialize_to_string<sint<2>>(number);
// Output: 0xFC19
```

All numbers are big-endian by default, but this can be specified:

```cpp
using namespace okser;

uint16_t number = 999;

serialize_to_string<uint<2, end::le>>(number); // Output: 0xE703
serialize_to_string<uint<2, end::be>>(number); // Output: 0x03E7
```

@section simple-types-floats Floating-point numbers

Floating-point numbers are represented by @ref okser::floatp.

```cpp
using namespace okser;

float number = 3.14;
serialize_to_string<floatp>(number);
// Output: 0x4048F5C3
```

Floating-point numbers are 32-bit by default, but their size and endianess can be specified:

```cpp
using namespace okser;

double number = 3.14f;
serialize_to_string<floatp<8, end::le>>(number); // Output: 0x1F85EB51B81E0940

// Alternatively, you can use:
serialize_to_string<doublep>(number);
```

@section simple-types-enum Enumerations

A convenience type exists for enumerations to automatically convert them to their underlying
integer implementation. The size can be manually specified, but by default is the size of the
enumeration's underlying type.

Enums are serialized by @ref okser::enumv.

```cpp
using namespace okser;

enum class Fox : uint8_t {
    RedFox = 0,
    GreenFox = 1,
    PastelFox = 2
};

auto fox = Fox::GreenFox;
serialize_to_string<enumv<Fox>>(fox); // Output: 0x01

// Specifying bytes and endianness:
serialize_to_string<enumv<Fox, 2, end::le>>(fox); // Output: 0x0001
```


