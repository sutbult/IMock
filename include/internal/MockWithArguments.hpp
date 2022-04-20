#pragma once

#include <functional>

#include <internal/CaseMatch.hpp>
#include <internal/CaseMatchFactory.hpp>
#include <internal/ICase.hpp>

namespace IMock::Internal {

template <typename TReturn, typename ...TArguments>
class MockWithArgumentsCase : public ICase<TReturn, TArguments...> {
    private:
        std::tuple<TArguments...> _arguments;
        std::function<CaseMatch<TReturn> (TArguments...)> _fake;

    public:
        MockWithArgumentsCase(
            std::tuple<TArguments...> arguments,
            std::function<CaseMatch<TReturn> (TArguments...)> fake)
            : _arguments(std::move(arguments))
            , _fake(std::move(fake)) {
            }

        CaseMatch<TReturn> matches(std::tuple<TArguments...>& arguments) {
            bool match = arguments == _arguments;
            if(match) {
                return Apply::apply(_fake, std::move(arguments));
            }
            else {
                return CaseMatchFactory::noMatch<TReturn>();
            }
        }
};

}
