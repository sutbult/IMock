#pragma once

#include <memory>
#include <utility>

#include <internal/IReturnValue.hpp>

namespace IMock::Internal {

/// Indicates if a call to a mock case resulted in a match with a return value
/// or if it resulted in no match.
template <typename TReturn>
class CaseMatch {
    private:
        /// A possible return value.
        std::unique_ptr<IReturnValue<TReturn>> _returnValue;

    public:
        /// Creates a CaseMatch.
        ///
        /// @param returnValue A possible return value.
        CaseMatch(std::unique_ptr<IReturnValue<TReturn>> returnValue)
            : _returnValue(std::move(returnValue)) {
        }

        /// Move constructor for CaseMatch.
        CaseMatch(CaseMatch&& other)
            : _returnValue(std::move(other._returnValue)) {
        }

        /// Indicates if the call resulted in a match.
        ///
        /// @return True if the call resulted in a match and false otherwise.
        bool isMatch() const {
            // A match happened if the a return value exists.
            return _returnValue.get() != nullptr;
        }

        /// Gets the return value. Calling this on an instance where isMatch()
        /// returns false results in a segmentation fault.
        ///
        /// @return The return value.
        IReturnValue<TReturn>& getReturnValue() const {
            // Get and return the return value.
            return *_returnValue.get();
        }
};

}
