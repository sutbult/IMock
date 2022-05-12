#pragma once

#include <string>
#include <vector>

namespace IMock {
namespace Internal {

/// Joins vectors of strings using a provided delimiter.
class JoinStrings {
    public:
        /// JoinStrings is not supposed to be instantiated since it only
        /// contains static methods.
        JoinStrings() = delete;

        /// Joins a vector of strings using a provided delimiter.
        ///
        /// @param delimiter The string to insert between each string.
        /// @param strings The vector of strings to be joined.
        static std::string joinStrings(
            std::string delimiter,
            std::vector<std::string> strings) {
            // Create an empty list to store the result.
            std::string result = "";

            // Get the beginning of the string vector.
            std::vector<std::string>::iterator begin = strings.begin();

            // Process each string.
            for(std::vector<std::string>::iterator iterator = begin;
                iterator < strings.end();
                iterator++) {
                // Check if the current string is a non-initial string.
                if(iterator > begin) {
                    // Append the delimiter if that's the case.
                    result.append(delimiter);
                }

                // Append the current string.
                result.append(*iterator);
            }

            // Return the joined string.
            return result;
        }
};

}
}
