#pragma once

#include <memory>
#include <utility>

#include <internal/IReturnValue.hpp>

namespace IMock::Internal {

template <typename TReturn>
class CaseMatch {
    private:
        std::unique_ptr<IReturnValue<TReturn>> _returnValue;

    public:
        CaseMatch(std::unique_ptr<IReturnValue<TReturn>> returnValue)
            : _returnValue(std::move(returnValue)) {
        }

        CaseMatch(CaseMatch&& other)
            : _returnValue(std::move(other._returnValue)) {
        }

        virtual ~CaseMatch() {
        }

        bool isMatch() {
            return _returnValue.get() != nullptr;
        }

        IReturnValue<TReturn>& getReturnValue() {
            return *_returnValue.get();
        }
};

}
