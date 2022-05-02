#pragma once

#include <string>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

namespace IMock::Internal {

/// Converts values to strings.
class ToString {
    private:
        // A trick to see if two types can use the insertion operator.
        // The solution has been taken from:
        // https://stackoverflow.com/a/22759544/6188897
        template<typename S, typename T>
        class is_streamable
        {
            private:
                template<typename SS, typename TT>
                static auto test(int) -> decltype(
                    std::declval<SS&>() << std::declval<TT>(),
                    std::true_type());

                template<typename, typename>
                static auto test(...) -> std::false_type;

            public:
                static const bool value = decltype(test<S,T>(0))::value;
        };

    public:
        /// ToString is not supposed to be instantiated since it only contains
        /// static methods.
        ToString() = delete;

        /// Converts a value to a string.
        ///
        /// Uses the insertion operator since it's available.
        ///
        /// @param value The value to be converted.
        /// @return A string representation of the value.
        template <typename TValue>
        static std::string toString(
            typename std::enable_if<
                is_streamable<std::stringstream, TValue>::value,
                TValue>::type value) {
            // Create a stringstream.
            std::stringstream out;

            // Append the value.
            out << value;

            // Create a string and return it.
            return out.str();
        }

        /// Converts a value to a string.
        ///
        /// Returns a question mark since the insertion operator is unavailable.
        ///
        /// @param value The value to be converted.
        /// @return A string representation of the value.
        template <typename TValue>
        static std::string toString(
            typename std::enable_if<
                !is_streamable<std::stringstream, TValue>::value,
                TValue>::type value) {
            // Return a question mark.
            return "?";
        }

        /// Converts a number of values to strings.
        /// Uses ToString::toString internally.
        ///
        /// @param values The values to be converted.
        /// @return String representations of the values.
        template <typename ...TValue>
        static std::vector<std::string> toStrings(TValue... values) {
            // Call toString for each value.
            std::vector<std::string> strings = {
                toString<TValue>(std::move(values))...
            };

            // Return the strings.
            return strings;
        }
};

}
