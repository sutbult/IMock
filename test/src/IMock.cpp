#include <vector>

#include <catch2/catch.hpp>

#include <IMock.hpp>

TEST_CASE("can mock a basic interface", "[basic]") {
    class ICalculator {
        public:
            virtual ~ICalculator() {
            }
            virtual int add(int, int) = 0;
            virtual int subtract(int, int) = 0;
            virtual int multiply(int, int) = 0;
            virtual int divide(int, int) = 0;
    };

    IMock::Mock<ICalculator> mock;

    SECTION("call add when it has not been mocked") {
        REQUIRE_THROWS_MATCHES(
            mock.get().add(1, 1),
            IMock::UnknownCallException,
            Catch::Message("A call was made to a method that has not been "
                "mocked."));
    }

    SECTION("mock add") {
        IMock::MockCaseCallCount mockCaseCallCount = when(mock, add)
            .with(1, 1)
            .returns(2);

        SECTION("no calls have initially been made") {
            SECTION("verifyNeverCalled() does not throw any exception") {
                REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
            }

            SECTION("verifyCalledOnce() throws a WrongCallCountException") {
                REQUIRE_THROWS_MATCHES(
                    mockCaseCallCount.verifyCalledOnce(),
                    IMock::WrongCallCountException,
                    Catch::Message("Expected the method to be called 1 time "
                        "but it was called 0 times."));
            }
        }

        SECTION("call add with the mocked values") {
            int result = mock.get().add(1, 1);

            SECTION("the result is correct") {
                REQUIRE(result == 2);
            }

            SECTION("the call count is one") {
                SECTION("verifyCalledOnce() does not throw any exception") {
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
                }

                SECTION("verifyNeverCalled() throws a "
                    "WrongCallCountException") {
                    REQUIRE_THROWS_MATCHES(
                        mockCaseCallCount.verifyNeverCalled(),
                        IMock::WrongCallCountException,
                        Catch::Message("Expected the method to be called 0 "
                            "times but it was called 1 time."));
                }
            }
        }

        SECTION("call add with unmocked values") {
            REQUIRE_THROWS_MATCHES(
                mock.get().add(1, 2),
                IMock::UnmockedCallException,
                Catch::Message("A call was made to a method that has been "
                    "mocked but the arguments does not match the mocked "
                    "arguments."));
        }

        SECTION("call another method that has not been mocked") {
            REQUIRE_THROWS_MATCHES(
                mock.get().subtract(1, 1),
                IMock::UnknownCallException,
                Catch::Message("A call was made to a method that has not been "
                    "mocked."));
        }

        SECTION("mock add again") {
            IMock::MockCaseCallCount mockCaseCallCountSecond = when(mock, add)
                .with(2, 2)
                .returns(5);

            SECTION("call add with the second mock") {
                int result = mock.get().add(2, 2);

                SECTION("the result is correct") {
                    REQUIRE(result == 5);
                }

                SECTION("the call count is one") {
                    REQUIRE_NOTHROW(mockCaseCallCountSecond.verifyCalledOnce());
                }

                SECTION("the call count for the first mock is zero") {
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
                }
            }

            SECTION("call add with the first mock") {
                int result = mock.get().add(1, 1);

                SECTION("the result is correct") {
                    REQUIRE(result == 2);
                }

                SECTION("the call count is one") {
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
                }

                SECTION("the call count for the second mock is zero") {
                    REQUIRE_NOTHROW(mockCaseCallCountSecond
                        .verifyNeverCalled());
                }
            }
        }

        SECTION("mock subtract") {
            IMock::MockCaseCallCount mockCaseCallCountSecond =
                when(mock, subtract)
                    .with(1, 1)
                    .returns(0);

            SECTION("call subtract") {
                int result = mock.get().subtract(1, 1);

                SECTION("the result is correct") {
                    REQUIRE(result == 0);
                }

                SECTION("the call count is one") {
                    REQUIRE_NOTHROW(mockCaseCallCountSecond.verifyCalledOnce());
                }

                SECTION("the call count for the first mock is zero") {
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyNeverCalled());
                }
            }

            SECTION("call add") {
                int result = mock.get().add(1, 1);

                SECTION("the result is correct") {
                    REQUIRE(result == 2);
                }

                SECTION("the call count is one") {
                    REQUIRE_NOTHROW(mockCaseCallCount.verifyCalledOnce());
                }

                SECTION("the call count for the second mock is zero") {
                    REQUIRE_NOTHROW(mockCaseCallCountSecond
                        .verifyNeverCalled());
                }
            }
        }
    }

    SECTION("mock add in a loop") {
        std::vector<IMock::MockCaseCallCount> mockCaseCallCounts;

        for(int i = 0; i < 2; i++) {
            mockCaseCallCounts.push_back(when(mock, add)
                .with(i, i)
                .returns(i * 2));
        }

        SECTION("call add with the mocked values") {
            int i = GENERATE(0, 1);

            int result = mock.get().add(i, i);

            SECTION("the result is correct") {
                REQUIRE(result == i * 2);
            }

            SECTION("the call count is one") {
                REQUIRE_NOTHROW(mockCaseCallCounts[i].verifyCalledOnce());
            }
        }
    }
}
