#include <IMock.hpp>

/// An interface representing a calculator.
class ICalculatorSecondary {
    public:
        virtual int add(int, int) = 0;
        virtual int subtract(int, int) = 0;
        virtual int multiply(int, int) = 0;
        virtual int divide(int, int) = 0;
};

/// Performs a simple mock test to verify IMock.hpp can be included in two
/// separate source code files without causing linking problems.
void mockSecondaryFile() {
    // Create a Mock of ICalculatorSecondary.
    IMock::Mock<ICalculatorSecondary> mock;

    // Mock add.
    IMock::CallCount callCount = when(mock, add)
        .with(1, 1)
        .returns(2);

    // Verify the mock case never has been called.
    callCount.verifyNeverCalled();

    // Call add with the mocked values.
    mock.get().add(1, 1);

    // Verify the mock case has been called once.
    callCount.verifyCalledOnce();
}
