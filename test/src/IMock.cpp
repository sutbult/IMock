#include <vector>

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
            IMock::UnknownCallException,
            Catch::Message("A call was made to a method that has not been "
                "mocked."));
    }

    SECTION("mock add") {
        // Mock add.
        IMock::MockCaseCallCount mockCaseCallCount = when(mock, add)
            .with(1, 1)
            .returns(2);

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
                    IMock::WrongCallCountException,
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
                        IMock::WrongCallCountException,
                        Catch::Message("Expected the method to be called 0 "
                            "times but it was called 1 time."));
                }
            }
        }

        SECTION("call add with unmocked values") {
            // Perform the call and verify it throws an UnmockedCallException.
            REQUIRE_THROWS_MATCHES(
                mock.get().add(1, 2),
                IMock::UnmockedCallException,
                Catch::Message("A call was made to a method that has been "
                    "mocked but the arguments does not match the mocked "
                    "arguments."));
        }

        SECTION("call another method that has not been mocked") {
            // Perform the call and verify it throws an UnknownCallException.
            REQUIRE_THROWS_MATCHES(
                mock.get().subtract(1, 1),
                IMock::UnknownCallException,
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
