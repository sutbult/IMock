#include <catch2/catch.hpp>

#include <IMock.hpp>

TEST_CASE("Can mock a basic interface", "[basic]") {
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

    SECTION("can mock add") {
        IMock::MockCaseCallCount mockCallCount = when(mock, add)
            .with(1, 1)
            .returns(2);

        SECTION("no calls have initially been made") {
            SECTION("verifyNeverCalled() does not throw any exception") {
                REQUIRE_NOTHROW(mockCallCount.verifyNeverCalled());
            }

            SECTION("verifyCalledOnce() throws a WrongCallCountException") {
                REQUIRE_THROWS_MATCHES(
                    mockCallCount.verifyCalledOnce(),
                    IMock::WrongCallCountException,
                    Catch::Message("Expected the method to be called 1 time "
                        "but it was called 0 times."));
            }
        }

        SECTION("Call add with the mocked values") {
            int result = mock.get().add(1, 1);

            SECTION("The result is correct") {
                REQUIRE(result == 2);
            }

            SECTION("The call count is one") {
                SECTION("verifyCalledOnce() does not throw any exception") {
                    REQUIRE_NOTHROW(mockCallCount.verifyCalledOnce());
                }

                SECTION("verifyNeverCalled() throws a "
                    "WrongCallCountException") {
                    REQUIRE_THROWS_MATCHES(
                        mockCallCount.verifyNeverCalled(),
                        IMock::WrongCallCountException,
                        Catch::Message("Expected the method to be called 0 "
                            "times but it was called 1 time."));
                }
            }
        }
    }
}
