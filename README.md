# IMock

[![Build](https://github.com/sutbult/IMock/actions/workflows/build.yml/badge.svg)](https://github.com/sutbult/IMock/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

IMock is a C++11 mocking library for interfaces.
It supports regular stack-allocated values,
reference values and values without copy constructors as either arguments or
return values.

## Getting started

### Installation

IMock is packaged as [a single header](singleHeader/IMock.hpp).
Download the header, place it in your project and include it.

### Usage

### Basic example

Suppose you want to mock this interface:

```
class ICalculator {
    public:
        virtual int add(int, int) = 0;
};
```

First, create a `Mock` instance for the calculator:

```
using namespace IMock;

Mock<ICalculator> mock;
```

Then, instruct the interface to return a certain value when `add` is called with
certain arguments:

```
CallCount callCount = when(mock, add).with(1, 1).returns(2);
```

`callCount` will be used later.

Get a reference to the mocked interface:

```
ICalculator& calculator = mock.get();
```

Call `add`:

```
int sum = calculator.add(1, 1);

// Prints 2.
cout << sum << endl;
```

Use `callCount` to verify that `add` has been called once with the terms `1` and
`1`:

```
callCount.verifyCalledOnce();
```

`verifyCalledOnce` would have thrown an exception if `add` had never been called
or if `add` had been called more than once.

### Faking

`fake` can be called instead of `returns` if you desire to handle the call
yourself:

```
CallCount callCount = when(mock, add).with(1, 1).fake([](int a, int b) {
    return a + b;
});
```

The behavior will be identical to the basic example as seen above.

`fake` can be called without calling `with`:

```
CallCount callCount = when(mock, add).fake([](int a, int b) {
    return a + b;
});
```

In this case, all calls to `add` will be handled by the provided lambda.
Notice that later mock cases takes precidence over earlier mock cases.
Therefore, if you desire to have a number of specific cases using `with` and
then have a general case not using `with`, you should mock the general case
first to achieve the expected result.

### Call counts

Call `getCallCount` on `callCount` to get the number of times `add` has been
called where the added mock case has been matched:

```
// Prints 0 if add has never been called or 1 if add has been called once.
cout << callCount.getCallCount() << endl;
```

Call `verifyNeverCalled` before `add` is called to verify the method has never
been called:

```
callCount.verifyNeverCalled();
```

`verifyNeverCalled` will throw an exception if `add` has been called at least
once.

Call `verifyCallCount` to verify the method has been called a certain number of
times:

```
callCount.verifyCallCount(2);
```

`verifyCallCount` will throw an exception unless `add` has been called exactly
twice.

### Reference types

Interfaces that uses reference types for arguments and return
values can be mocked:

```
class ICalculator {
    public:
        virtual const int& add(const int&, const int&) = 0;
};

using namespace IMock;

Mock<ICalculator> mock;

int one = 1;
int two = 2;

CallCount callCount = when(mock, add).with(one, one).returns(two);

{
    int scopedOne = 1;

    // Prints 2.
    cout << mock.get().add(scopedOne, scopedOne) << endl;
}

callCount.verifyCalledOnce();
```

However, it is necessary to keep the memory containing values used in mock cases
alive for the duration of the `Mock` instance as only references are stored if
arguments and/or return values are declared as references.
Nevertheless, values used to call the mocked method only needs to be kept alive for the duration of the call and can be safely deleted afterwards.

### Methods without arguments

A method does not need to have any arguments:

```
class INoArguments {
    public:
        virtual int getInt() = 0;
};

using namespace IMock;

Mock<INoArguments> mock;
```

In this case, call `with` without any arguments:

```
CallCount callCount = when(mock, getInt).with().returns(1);

// Prints 2.
cout << mock.get().getInt() << endl;

callCount.verifyCalledOnce();
```

### Methods without return values

A method does not need to have a return value:

```
class INoReturnValue {
    public:
        virtual void setInt(int) = 0;
};

using namespace IMock;

Mock<INoReturnValue> mock;
```

In this case, call `returns` without a return value:

```
CallCount callCount = when(mock, setInt).with(1).returns();

mock.get().setInt(1);

callCount.verifyCalledOnce();
```

### Values without copy constructors

Suppose you want to mock an interface with arguments and return values without
copy constructors:

```
class NoCopy {
    private:
        std::unique_ptr<int> _value;

    public:
        NoCopy(int value)
            : _value(IMock::Internal::makeUnique<int>(std::move(value))) {
        }

        const int& getValue() const {
            return *_value;
        }

        bool operator == (const NoCopy& other) const {
            return this->getValue() == other.getValue();
        }
};

class INoCopy {
    public:
        virtual void setInt(NoCopy) = 0;
        virtual NoCopy getInt() = 0;
};

using namespace IMock;

Mock<INoCopy> mock;
```

Arguments without copy constructors are supported as long as IMock is allowed to
own the arguments:

```
CallCount callCount = when(mock, setInt).with(NoCopy(1)).returns()
```

`setInt` can now be called with another instance of `NoCopy` provided the
library (just like with the arguments) can take ownership of the argument:

```
mock.get().setInt(NoCopy(1));
```

Similarly, return values without copy constructors are also supported.
However, `fake` must be used in this case as it has to be possible to return any
value given to `returns` more than once:

```
CallCount callCount = when(mock, getInt).with().fake([]() {
    return NoCopy(1);
});
```

## Testing

The folder test contains a test suite for the library.
These dependencies are required to run it:

- GCC
- Clang
- CMake
- Make
- LCOV

Execute `make test` to run the test suite.

Alternatively, if Docker is installed and is running, you can execute
`make docker-test` to run the test suite in a Docker container.
Only Make has to be installed on the host in this case.

Execute `make merge-headers` to update the single header.

Execute `make test-all` to run the test suite regularly, update the single
header and compile and run the test suite using the single header with GCC and
Clang and with C++11 and C++14.
`make docker-test-all` is the corresponding Docker command.

## Benchmarks

The test suite contains a benchmarking case where a method is mocked an
increasing number of times and then called with the argument of the first mock
case, traversing all cases.
The benchmark is not run with the other tests by default but can be run with
`make benchmark` or `make docker-benchmark`.

## Documentation

Documentation in an HTML format can be generated using `make docs`.
The resulting documentation can afterwards be found in `html`.

## Support

The library and its tests have been tested on these compilers:

- GCC (11.2.0, 9.4.0)
- Clang (14.0.0, 10.0.0)
- Apple clang (13.0.0)
- Microsoft Visual C++ 2022
