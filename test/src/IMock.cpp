#include <vector>

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include <IMock.hpp>

TEST_CASE("can mock a basic interface", "[basic]") {
    // Declare a basic interface.
    class ICalculator {
        public:
            virtual int add(int, int) = 0;
            virtual int subtract(int, int) = 0;
            virtual int multiply(int, int) = 0;
            virtual int divide(int, int) = 0;
    };

    // Create a Mock of ICalculator.
    IMock::Mock<ICalculator> mock;

    SECTION("call add when it has not been mocked") {
        // Perform the call and verify it throws an UnknownCallException.
        REQUIRE_THROWS_MATCHES(
            mock.get().add(1, 1),
            IMock::Exception::UnknownCallException,
            Catch::Message("A call was made to a method that has not been "
                "mocked."));
    }

    SECTION("mock add") {
        // Generate a bool to have two configurations.
        bool withReturns = GENERATE(true, false);

        // Mock add.
        IMock::MockCaseCallCount mockCaseCallCount = withReturns
            // Use returns if withReturns is true.
            ? when(mock, add)
                .with(1, 1)
                .returns(2)

            // Use fake if withReturns is false.
            : when(mock, add)
                .with(1, 1)
                .fake([](int a, int b) {
                    return a + b;
                });

        SECTION("no calls have initially been made") {
            SECTION("verifyNeverCalled() does not throw any exception") {
                // Perform the call and verify it does not throw an exception.
                REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
            }

            SECTION("verifyCalledOnce() throws a WrongCallCountException") {
                // Perform the call and verify it throws a
                // WrongCallCountException.
                REQUIRE_THROWS_MATCHES(
                    mockCaseCallCount.verifyCalledOnce(),
                    IMock::Exception::WrongCallCountException,
                    Catch::Message("Expected the method to be called 1 time "
                        "but it was called 0 times."));
            }
        }

        SECTION("call add with the mocked values") {
            // Call add with the mocked values.
            int result = mock.get().add(1, 1);

            SECTION("the result is correct") {
                // Verify the result equals 2.
                REQUIRE(result == 2);
            }

            SECTION("the call count is one") {
                SECTION("verifyCalledOnce() does not throw any exception") {
                    // Perform the call and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
                }

                SECTION("verifyNeverCalled() throws a "
                    "WrongCallCountException") {
                    // Perform the call and verify it throws a
                    // WrongCallCountException.
                    REQUIRE_THROWS_MATCHES(
                        mockCaseCallCount.verifyNeverCalled(),
                        IMock::Exception::WrongCallCountException,
                        Catch::Message("Expected the method to be called 0 "
                            "times but it was called 1 time."));
                }
            }
        }

        SECTION("call add with unmocked values") {
            // Perform the call and verify it throws an UnmockedCallException.
            REQUIRE_THROWS_MATCHES(
                mock.get().add(1, 2),
                IMock::Exception::UnmockedCallException,
                Catch::Message("The call mock.get().add(1, 2) does not match "
                    "any mocked case."));
        }

        SECTION("call another method that has not been mocked") {
            // Perform the call and verify it throws an UnknownCallException.
            REQUIRE_THROWS_MATCHES(
                mock.get().subtract(1, 1),
                IMock::Exception::UnknownCallException,
                Catch::Message("A call was made to a method that has not been "
                    "mocked."));
        }

        SECTION("mock add again") {
            // Mock add with other values.
            IMock::MockCaseCallCount mockCaseCallCountSecond = when(mock, add)
                .with(2, 2)
                .returns(5);

            SECTION("call add with the second mock") {
                // Call add with the values of the second mock.
                int result = mock.get().add(2, 2);

                SECTION("the result is correct") {
                    // Verify the result is 5.
                    REQUIRE(result == 5);
                }

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond.verifyCalledOnce());
                }

                SECTION("the call count for the first mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
                }
            }

            SECTION("call add with the first mock") {
                // Call add with the values of the first mock.
                int result = mock.get().add(1, 1);

                SECTION("the result is correct") {
                    // Verify the result is 2.
                    REQUIRE(result == 2);
                }

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
                }

                SECTION("the call count for the second mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond
                        .verifyNeverCalled());
                }
            }
        }

        SECTION("mock subtract") {
            // Mock subtract.
            IMock::MockCaseCallCount mockCaseCallCountSecond =
                when(mock, subtract)
                    .with(1, 1)
                    .returns(0);

            SECTION("call subtract") {
                // Call subtract with the mocked values.
                int result = mock.get().subtract(1, 1);

                SECTION("the result is correct") {
                    // Verify the result is 0.
                    REQUIRE(result == 0);
                }

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond.verifyCalledOnce());
                }

                SECTION("the call count for the first mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
                }
            }

            SECTION("call add") {
                // Call add with the mocked values.
                int result = mock.get().add(1, 1);

                SECTION("the result is correct") {
                    // Verify the result is 2.
                    REQUIRE(result == 2);
                }

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
                }

                SECTION("the call count for the second mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond
                        .verifyNeverCalled());
                }
            }
        }
    }

    SECTION("mock add in a loop") {
        // Create a vector for the MockCaseCallCount instances.
        std::vector<IMock::MockCaseCallCount> mockCaseCallCounts;

        // Iterate twice.
        for(int i = 0; i < 2; i++) {
            // Mock add.
            mockCaseCallCounts.push_back(when(mock, add)
                .with(i, i)
                .returns(i * 2));
        }

        SECTION("call add with the mocked values") {
            // Continue with each index.
            int i = GENERATE(0, 1);

            // Call add using the current index.
            int result = mock.get().add(i, i);

            SECTION("the result is correct") {
                // Verify the result is i * 2.
                REQUIRE(result == i * 2);
            }

            SECTION("the call count is one") {
                // Call verifyCalledOnce and verify it does not throw an
                // exception.
                REQUIRE_NOTHROW(mockCaseCallCounts[i].verifyCalledOnce());
            }
        }
    }

    SECTION("mock add twice with the same MockWithArguments instance") {
        // Create a MockWithArguments instance.
        auto mockWithArguments = when(mock, add).with(1, 1);

        // Mock add.
        mockWithArguments.returns(2);

        // Mock add again and verify it throws a
        // MockWithArgumentsUsedTwiceException.
        REQUIRE_THROWS_MATCHES(
            mockWithArguments.returns(3),
            IMock::Exception::MockWithArgumentsUsedTwiceException,
            Catch::Message("A MockWithArguments instance was reused."
                " This is not possible since the arguments are moved when"
                " adding a case."));
    }
}

TEST_CASE("can mock an interface where every argument and return value is a "
    "constant reference", "[reference]") {
    // Declare an interface.
    class ICalculator {
        public:
            virtual const int& add(const int&, const int&) = 0;
            virtual const int& subtract(const int&, const int&) = 0;
            virtual const int& multiply(const int&, const int&) = 0;
            virtual const int& divide(const int&, const int&) = 0;
    };

    // Create a Mock of ICalculator.
    IMock::Mock<ICalculator> mock;

    SECTION("mock add") {
        // Declare variables for one and two. It is necessary to keep the memory
        // containing used test values alive for the duration of the test since
        // IMock only stores references to values if arguments and/or return
        // values is declared as references.
        int one = 1;
        int two = 2;

        // Mock add.
        IMock::MockCaseCallCount mockCaseCallCount = when(mock, add)
            .with(one, one)
            .returns(two);

        SECTION("call add with the mocked values") {
            // Call add with the mocked values.
            const int& result = mock.get().add(one, one);

            SECTION("the result is correct") {
                // Verify the result equals two.
                REQUIRE(result == two);
            }

            SECTION("the call count is one") {
                // Call verifyCalledOnce and verify it does not throw an
                // exception.
                REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
            }
        }

        SECTION("mock add again") {
            // Declare a variable for five.
            int five = 5;

            // Mock add with other values.
            IMock::MockCaseCallCount mockCaseCallCountSecond = when(mock, add)
                .with(two, two)
                .returns(five);

            SECTION("call add with the second mock") {
                // Call add with the values of the second mock.
                const int& result = mock.get().add(two, two);

                SECTION("the result is correct") {
                    // Verify the result is five.
                    REQUIRE(result == five);
                }

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond.verifyCalledOnce());
                }

                SECTION("the call count for the first mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
                }
            }

            SECTION("call add with the first mock") {
                // Call add with the values of the first mock.
                const int& result = mock.get().add(one, one);

                SECTION("the result is correct") {
                    // Verify the result is two.
                    REQUIRE(result == two);
                }

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
                }

                SECTION("the call count for the second mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond
                        .verifyNeverCalled());
                }
            }
        }
    }
}

TEST_CASE("can mock an interface without arguments", "[no_arguments]") {
    // Declare an interface without arguments.
    class INoArguments {
        public:
            virtual int getInt() = 0;
    };

    // Create a Mock of INoArguments.
    IMock::Mock<INoArguments> mock;

    SECTION("call getInt when it has not been mocked") {
        // Perform the call and verify it throws an UnknownCallException.
        REQUIRE_THROWS_MATCHES(
            mock.get().getInt(),
            IMock::Exception::UnknownCallException,
            Catch::Message("A call was made to a method that has not been "
                "mocked."));
    }

    SECTION("mock getInt") {
        // Generate a bool to have two configurations.
        bool withReturns = GENERATE(true, false);

        // Mock getInt.
        IMock::MockCaseCallCount mockCaseCallCount = withReturns
            // Use returns if withReturns is true.
            ? when(mock, getInt)
                .with()
                .returns(1)

            // Use fake if withReturns is false.
            : when(mock, getInt)
                .with()
                .fake([]() {
                    return 1;
                });

        SECTION("no calls have initially been made") {
            // Call verifyNeverCalled and verify it does not throw an exception.
            REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
        }

        SECTION("call getInt") {
            // Call getInt.
            int result = mock.get().getInt();

            SECTION("the result is correct") {
                // Verify the result equals 1.
                REQUIRE(result == 1);
            }

            SECTION("the call count is one") {
                // Call verifyNeverCalled and verify it does not throw an
                // exception.
                REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
            }
        }

        SECTION("mock getInt again") {
            // Mock getInt with other values.
            IMock::MockCaseCallCount mockCaseCallCountSecond =
                when(mock, getInt)
                    .with()
                    .returns(2);

            SECTION("call getInt") {
                // Call getInt.
                int result = mock.get().getInt();

                SECTION("the result is correct") {
                    // Verify the result is 2.
                    REQUIRE(result == 2);
                }

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond.verifyCalledOnce());
                }

                SECTION("the call count for the first mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
                }
            }
        }
    }
}

TEST_CASE("can mock an interface without any return value",
    "[no_return_value]") {
    // Declare a basic interface.
    class INoReturnValue {
        public:
            virtual void setInt(int) = 0;
    };

    // Create a Mock of INoReturnValue.
    IMock::Mock<INoReturnValue> mock;

    SECTION("call setInt when it has not been mocked") {
        // Perform the call and verify it throws an UnknownCallException.
        REQUIRE_THROWS_MATCHES(
            mock.get().setInt(1),
            IMock::Exception::UnknownCallException,
            Catch::Message("A call was made to a method that has not been "
                "mocked."));
    }

    SECTION("mock setInt") {
        // Generate a bool to have two configurations.
        bool withReturns = GENERATE(true, false);

        // Mock setInt.
        IMock::MockCaseCallCount mockCaseCallCount = withReturns
            // Use returns if withReturns is true.
            ? when(mock, setInt)
                .with(1)
                .returns()

            // Use fake if withReturns is false.
            : when(mock, setInt)
                .with(1)
                .fake([](int i) {
                });

        SECTION("no calls have initially been made") {
            // Call verifyNeverCalled and verify it does not throw an exception.
            REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
        }

        SECTION("call setInt with the mocked values") {
            // Call setInt with the mocked value.
            mock.get().setInt(1);

            SECTION("the call count is one") {
                // Call verifyCalledOnce and verify it does not throw an
                // exception.
                REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
            }
        }

        SECTION("call setInt with an unmocked value") {
            // Perform the call and verify it throws an UnmockedCallException.
            REQUIRE_THROWS_MATCHES(
                mock.get().setInt(2),
                IMock::Exception::UnmockedCallException,
                Catch::Message("The call mock.get().setInt(2) does not match "
                    "any mocked case."));
        }

        SECTION("mock setInt again") {
            // Mock setInt with another value.
            IMock::MockCaseCallCount mockCaseCallCountSecond =
                when(mock, setInt)
                    .with(2)
                    .returns();

            SECTION("call add with the second mock") {
                // Call add with the value of the second mock.
                mock.get().setInt(2);

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond.verifyCalledOnce());
                }

                SECTION("the call count for the first mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
                }
            }

            SECTION("call add with the first mock") {
                // Call setInt with the value of the first mock.
                mock.get().setInt(1);

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
                }

                SECTION("the call count for the second mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond
                        .verifyNeverCalled());
                }
            }
        }
    }
}

TEST_CASE("can mock an interface with an argument that can't be copied",
    "[no_copy_argument]") {
    // Declare a class that can't be copied.
    class NoCopy {
        private:
            /// An integer held by a unique_ptr, which can't be copied.
            std::unique_ptr<int> _value;

        public:
            /// Creates a NoCopy with a provided value.
            ///
            /// @param value The value to held.
            NoCopy(int value)
                : _value(IMock::Internal::makeUnique<int>(std::move(value))) {
            }

            /// Gets the value.
            ///
            /// @return The value.
            const int& getValue() const {
                // Return the value.
                return *_value;
            }

            /// Compares the NoCopy with another NoCopy by comparing the
            /// contained values.
            ///
            /// @param other The NoCopy to compare to.
            /// @return True if the contained values equal each other and false
            /// otherwise.
            bool operator == (const NoCopy& other) const {
                // Compare the values and return the result.
                return this->getValue() == other.getValue();
            }
    };

    // Declare a basic interface.
    class INoCopyArgument {
        public:
            virtual void setInt(NoCopy) = 0;
            virtual NoCopy getInt() = 0;
            virtual NoCopy id(NoCopy) = 0;
    };

    // Create a Mock of INoCopyArgument.
    IMock::Mock<INoCopyArgument> mock;

    SECTION("call setInt when it has not been mocked") {
        // Perform the call and verify it throws an UnknownCallException.
        REQUIRE_THROWS_MATCHES(
            mock.get().setInt(NoCopy(1)),
            IMock::Exception::UnknownCallException,
            Catch::Message("A call was made to a method that has not been "
                "mocked."));
    }

    SECTION("mock setInt") {
        // Generate a bool to have two configurations.
        bool withReturns = GENERATE(true, false);

        // Mock setInt.
        IMock::MockCaseCallCount mockCaseCallCount = withReturns
            // Use returns if withReturns is true.
            ? when(mock, setInt)
                .with(NoCopy(1))
                .returns()

            // Use fake if withReturns is false.
            : when(mock, setInt)
                .with(NoCopy(1))
                .fake([](NoCopy noCopy) {
                });

        SECTION("no calls have initially been made") {
            // Call verifyNeverCalled and verify it does not throw an exception.
            REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
        }

        SECTION("call setInt with the mocked values") {
            // Call setInt with the mocked value.
            mock.get().setInt(NoCopy(1));

            SECTION("the call count is one") {
                // Call verifyCalledOnce and verify it does not throw an
                // exception.
                REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
            }
        }

        SECTION("call setInt with an unmocked value") {
            // Perform the call and verify it throws an UnmockedCallException.
            REQUIRE_THROWS_MATCHES(
                mock.get().setInt(NoCopy(2)),
                IMock::Exception::UnmockedCallException,
                Catch::Message("The call mock.get().setInt(?) does not match "
                    "any mocked case."));
        }

        SECTION("mock setInt again") {
            // Mock setInt with another value.
            IMock::MockCaseCallCount mockCaseCallCountSecond =
                when(mock, setInt)
                    .with(NoCopy(2))
                    .returns();

            SECTION("call add with the second mock") {
                // Call add with the value of the second mock.
                mock.get().setInt(NoCopy(2));

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond.verifyCalledOnce());
                }

                SECTION("the call count for the first mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
                }
            }

            SECTION("call add with the first mock") {
                // Call setInt with the value of the first mock.
                mock.get().setInt(NoCopy(1));

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
                }

                SECTION("the call count for the second mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond
                        .verifyNeverCalled());
                }
            }
        }

        SECTION("mock another method") {
            // Generate a bool to have two configurations.
            bool withGetInt = GENERATE(true, false);

            // Mock id.
            IMock::MockCaseCallCount mockCaseCallCountSecond = withGetInt
                // Mock getInt if withGetInt is true.
                ? when(mock, getInt)
                    .with()
                    .fake([]() {
                        // Create a NoCopy and return it.
                        return NoCopy(1);
                    })

                // Mock id if withGetInt is false.
                : when(mock, id)
                    .with(NoCopy(1))
                    .fake([](NoCopy noCopy) {
                        // Return the argument.
                        return noCopy;
                    });

            SECTION("call the other method") {
                // Call id.
                NoCopy result = withGetInt
                    // Call getInt if withGetInt is true.
                    ? mock.get().getInt()

                    // Call id if withGetInt is false.
                    : mock.get().id(NoCopy(1));

                SECTION("the result is correct") {
                    // Verify the result is 1.
                    REQUIRE(result.getValue() == 1);
                }

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond.verifyCalledOnce());
                }

                SECTION("the call count for the first mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
                }
            }

            SECTION("call setInt") {
                // Call setInt with the value of the first mock.
                mock.get().setInt(NoCopy(1));

                SECTION("the call count is one") {
                    // Call verifyCalledOnce and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
                }

                SECTION("the call count for the second mock is zero") {
                    // Call verifyNeverCalled and verify it does not throw an
                    // exception.
                    REQUIRE_NOTHROW(mockCaseCallCountSecond
                        .verifyNeverCalled());
                }
            }
        }
    }
}

/// See IMockSecondary.hpp
void mockSecondaryFile();

TEST_CASE("can mock an interface in a secondary file") {
    // Call mockSecondaryFile.
    REQUIRE_NOTHROW(mockSecondaryFile());
}

TEST_CASE("benchmark", "[.][benchmark]") {
    // Declare a utility class.
    class HeavyMock {
        public:
            /// Sets up a Mock with a method with a large number of mocks.
            /// Then, the first mock to have been made is called.
            ///
            /// @param mockCount The number of mocks to set up.
            static void heavyMock(int mockCount) {
                // Declare a basic interface.
                class IIdentity {
                    public:
                        virtual int id(int) = 0;
                };

                // Create a Mock of IIdentity.
                IMock::Mock<IIdentity> mock;

                // Mock id for 0.
                IMock::MockCaseCallCount mockCaseCallCount = when(mock, id)
                    .with(0)
                    .returns(0);

                // Mock id mockCount minus one times.
                for(int i = 1; i < mockCount; i++) {
                    // Mock id.
                    when(mock, id)
                        .with(i)
                        .returns(i);
                }

                // Call id with 0.
                mock.get().id(0);

                // Verify id has been called once with 0.
                mockCaseCallCount.verifyCalledOnce();
            }
    };

    // Declare a macro for benchmark cases.
    #define benchmarkMockCalls(mockCalls) \
    BENCHMARK(#mockCalls) { \
        HeavyMock::heavyMock(mockCalls); \
    };

    // Create benchmark cases for values that doubles for each new case.
    benchmarkMockCalls(1)
    benchmarkMockCalls(2)
    benchmarkMockCalls(4)
    benchmarkMockCalls(8)
    benchmarkMockCalls(16)
    benchmarkMockCalls(32)
    benchmarkMockCalls(64)
    benchmarkMockCalls(128)
    benchmarkMockCalls(256)
    benchmarkMockCalls(512)
    benchmarkMockCalls(1024)
    benchmarkMockCalls(2048)
    benchmarkMockCalls(4096)
    benchmarkMockCalls(8192)
    benchmarkMockCalls(16384)
    benchmarkMockCalls(32768)
    benchmarkMockCalls(65536)
    benchmarkMockCalls(131072)
    benchmarkMockCalls(262144)
    benchmarkMockCalls(524288)
    benchmarkMockCalls(1048576)
}
