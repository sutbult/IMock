#pragma once

#include <exception/UnmockedCallException.hpp>
#include <internal/Apply.hpp>
#include <IReturnValue.hpp>
#include <MockCaseCallCount.hpp>

namespace IMock::Internal {

class ICaseNonGeneric {
    public:
        virtual ~ICaseNonGeneric() {
        }
};

template <typename TReturn, typename ...TArguments> 
class ICase : public ICaseNonGeneric {
    public:
        virtual TReturn call(TArguments... arguments) = 0;
};

template <typename TReturn, typename ...TArguments>
class UnmockedCase : public ICase<TReturn, TArguments...> {
    public:
        virtual TReturn call(TArguments... arguments) override {
            throw Exception::UnmockedCallException();
        }
};

template <typename TReturn, typename ...TArguments>
class MockedCase : public ICase<TReturn, TArguments...> {
    private:
        std::unique_ptr<ICase<TReturn, TArguments...>> _previousCase;
        std::shared_ptr<MockCaseMutableCallCount> _callCount;
        std::unique_ptr<IReturnValue<TReturn>> _returnValue;
        std::tuple<TArguments...> _arguments;

    public:
        MockedCase(
            std::unique_ptr<ICase<TReturn, TArguments...>> previousCase,
            std::shared_ptr<MockCaseMutableCallCount> callCount,
            std::unique_ptr<IReturnValue<TReturn>> returnValue,
            std::tuple<TArguments...> arguments)
            : _previousCase(std::move(previousCase))
            , _callCount(std::move(callCount))
            , _returnValue(std::move(returnValue))
            , _arguments(std::move(arguments)) {
        }

        virtual TReturn call(TArguments... arguments) override {
            std::tuple<TArguments...> tupleArguments(std::move(arguments)...);
            bool match = tupleArguments == _arguments;
            if(match) {
                _callCount->increase();
                return _returnValue->getReturnValue();
            }
            else {
                return Apply::apply(
                    &ICase<TReturn, TArguments...>::call,
                    *_previousCase,
                    std::move(tupleArguments));
            }
        }
};

}
