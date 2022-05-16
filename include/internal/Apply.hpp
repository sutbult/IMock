#pragma once

#include <tuple>
#include <functional>

namespace IMock {
namespace Internal {

/// Utility making it possible to call a callback using arguments contained in
/// a tuple.
class Apply {
    private:
        //! @cond Doxygen_Suppress
        // A trick to statically produce a list of integers. The solution has
        // been taken from: https://stackoverflow.com/a/7858971/6188897
        template<int ...>
        struct seq {
        };

        template<int N, int ...S>
        struct gens : gens<N-1, N-1, S...> {
        };

        template<int ...S>
        struct gens<0, S...>{
            typedef seq<S...> type;
        };
        //! @endcond

        /// Calls the provided callback with the arguments in the provided
        /// tuple.
        ///
        /// Also includes a "seq" making it possible to extract the arguments
        /// from the tuple.
        ///
        /// @param callback The function to call.
        /// @param arguments The arguments to call the callback with.
        /// @return The return value from the callback.
        /// @tparam A counter used to extract arguments.
        /// @tparam TReturn The return type of the callback.
        /// @tparam TArguments The types of the arguments of the callback.
        template<int ...S, typename TReturn,
            typename ...TArguments>
        static TReturn applyWithSeq(
            seq<S...>,
            std::function<TReturn (TArguments...)> callback,
            std::tuple<TArguments...> arguments) {
            // Call callback with the extracted arguments.
            return callback(std::move(std::get<S>(arguments))...);
        }

    public:
        /// Apply is not supposed to be instantiated since it only contains
        /// static methods.
        Apply() = delete;

        /// Calls the provided callback with the arguments in the provided
        /// tuple.
        ///
        /// @param callback The function to call.
        /// @param arguments The arguments to call the callback with.
        /// @return The return value from the callback.
        /// @tparam TReturn The return type of the callback.
        /// @tparam TArguments The types of the arguments of the callback.
        template<typename TReturn, typename ...TArguments>
        static TReturn apply(
            std::function<TReturn (TArguments...)> callback,
            std::tuple<TArguments...> arguments) {
            // Create a "gens" with the number of arguments.
            return applyWithSeq(typename gens<sizeof...(TArguments)>::type(),
                callback,
                std::move(arguments));
        }
};

}
}
