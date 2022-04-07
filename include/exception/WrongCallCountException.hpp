#pragma once

#include <sstream>

#include <exception/MockException.hpp>

namespace IMock::Exception {

class WrongCallCountException : public MockException {
    public:
        WrongCallCountException(
            int expectedCallCount,
            int actualCallCount) 
            : MockException(getMessage(
                expectedCallCount,
                actualCallCount)) {
        }

    private:
        static std::string getMessage(
            int expectedCallCount,
            int actualCallCount) {
            std::stringstream out;

            out << "Expected the method to be called "
                << expectedCallCount
                << " time";
            
            if(expectedCallCount != 1) {
                out << "s";
            }

            out << " but it was called "
                << actualCallCount
                << " time";

            if(actualCallCount != 1) {
                out << "s";
            }

            out << ".";

            return out.str();
        }
};

}
