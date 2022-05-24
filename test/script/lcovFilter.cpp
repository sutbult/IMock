#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using std::exception;
using std::get;
using std::ifstream;
using std::ofstream;
using std::regex;
using std::regex_constants::ECMAScript;
using std::smatch;
using std::sregex_iterator;
using std::string;
using std::stringstream;
using std::tuple;
using std::vector;

/// A class implementing exception that can be thrown by the program when
/// deemed necessary.
class Exception : public exception {
    private:
        /// An explanation of what went wrong.
        string message;
    
    public:
        /// Creates an Exception.
        ///
        /// @param message An explanation of what went wrong.
        Exception(string message);

        /// An override of exception::what() that returns the message.
        ///
        /// @return A constant pointer to the message.
        const char* what() const noexcept override;
};

Exception::Exception(
    string message)
    : message(message) {
}

const char* Exception::what() const noexcept {
    // Return a constant pointer to the message.
    return message.c_str();
}

/// Checks if target starts with start.
///
/// @param start The start to check with.
/// @param target The target string to be checked.
/// @return True if target starts with start and false otherwise.
bool startsWith(string start, string target) {
    // Get the start of the target with the same length as start.
    string targetStart = target.substr(0, start.length());

    // Check if the start of the target starts with start.
    bool match = targetStart == start;

    // Return the result.
    return match;
}

/// Removes any initial whitespaces.
///
/// @param target The string to be trimmed.
/// @return The target without any initial whitespaces.
string trimStart(string target) {
    // Get the first character that is not a whitespace.
    int firstNonWhitespace = target.find_first_not_of(" \t\n\r");

    // Check if any non-whitespace characters exist.
    if(firstNonWhitespace < 0) {
        // Return the target if the string only contains whitespaces.
        return target;
    }
    else {
        // Remove the initial whitespaces.
        string trimmed = target.substr(firstNonWhitespace);

        // Return the trimmed string.
        return trimmed;
    }
}

/// Checks if target starts with start and ignores any initial whitespace.
///
/// @param start The start to check with.
/// @param target The target string to be checked.
/// @return True if target starts with start and false otherwise.
bool startsWithTrimmed(string start, string target) {
    string trimmedTarget = trimStart(target);
    bool result = startsWith(start, trimmedTarget);
    return result;
}

/// Get the first integer value after the provided start.
///
/// @param start The length of this starting string is skipped.
/// @param target The target string to get a value from.
/// @return The extracted value.
int getFirstValue(string start, string target) {
    // Get the length of the start string.
    int startLength = start.length();

    // Get the first comma in the target.
    int firstComma = target.find_first_of(',');

    // Get the value appearing after the start string but before the comma.
    string stringLine = target.substr(startLength, firstComma - startLength);

    // Parse an integer.
    int value = stoi(stringLine);

    // Return the integer.
    return value;
}

/// Get the second integer value after the provided start.
///
/// @param start The length of this starting string is skipped.
/// @param target The target string to get a value from.
/// @return The extracted value.
int getSecondValue(string start, string target) {
    // Get the length of the start string.
    int startLength = start.length();

    // Get the first comma in the target.
    int firstComma = target.find_first_of(',');

    // Get the value appearing after the first comma.
    string stringLine = target.substr(firstComma + 1);

    // Parse an integer.
    int value = stoi(stringLine);

    // Return the integer.
    return value;
}

/// Matches a string with a regular expression.
///
/// @param regex The regular expression to match the string with.
/// @param target The string to match.
/// @param values The number of values to extract.
/// @return An smatch describing the match.
smatch getMatch(
    const regex& regex,
    const string& target,
    int values) {
    // Match the string.
    sregex_iterator begin(target.begin(), target.end(), regex);

    // Get the number of matches.
    sregex_iterator end;
    int matches = distance(begin, end);

    // Check that the number of matches is one.
    if(matches != 1) {
        // Throw an exception since the number of matches is not one.
        throw Exception("The number of matches must be one. Target: " + target);
    }

    // Get the first match.
    smatch match = *begin;

    // Check that the number of values in the first match is correct.
    if(match.end() - match.begin() < values + 1) {
        // Throw an exception since the number of values in the first match is
        // incorrect.
        throw Exception("The number of values are too few. Target: " + target);
    }

    // Return the first match.
    return match;
}

/// Parses three values from a regular expression.
///
/// @param regex The regular expression to match the string with.
/// @param target The string to get the values from.
/// @return A tuple containing the three values.
tuple<int, int, int> getThreeValues(
    const regex& regex,
    string target) {
    // Match the string.
    smatch match = getMatch(regex, target, 3);

    // Convert the values from the match to integers and place them in a tuple.
    tuple<int, int, int> values = {
        stoi(match[1]),
        stoi(match[2]),
        stoi(match[3]),
    };

    // Return the tuple.
    return values;
}

/// Parses four values from a regular expression.
///
/// @param regex The regular expression to match the string with.
/// @param target The string to get the values from. 
/// @return A tuple containing the four values.
tuple<int, int, int, int> getFourValues(
    const regex& regex,
    string target) {
    // Match the string.
    smatch match = getMatch(regex, target, 4);

    // Convert the values from the match to integers and place them in a tuple.
    tuple<int, int, int, int> values = {
        stoi(match[1]),
        stoi(match[2]),
        stoi(match[3]),
        stoi(match[4]),
    };

    // Return the tuple.
    return values;
}

/// Parse a BRDA line and return its values.
///
/// @param coverageLine The coverage line with the BRDA values.
/// @return The parsed BRDA values.
tuple<int, int, int, int> getBRDAValues(string coverageLine) {
    // Check if the coverage line ends with a hyphen.
    if(coverageLine.find_first_of('-') == string::npos) {
        // Parse four values if the coverage line does not end with a hyphen.

        // Create a regular expression.
        regex brdaRegex(
            "BRDA:(\\d+),(\\d+),(\\d+),(\\d+)",
            ECMAScript);

        // Parse and return the values.
        return getFourValues(brdaRegex, coverageLine);
    }
    else {
        // Parse three value if the coverage line does not end with a hyphen.

        // Create a regular expression.
        regex brdaRegex(
            "BRDA:(\\d+),(\\d+),(\\d+),-",
            ECMAScript);
        
        // Get three values.
        tuple<int, int, int> threeTuple
            = getThreeValues(brdaRegex, coverageLine);

        // Copy the three values and also add a zero, since that's what a hyphen
        // means.
        tuple<int, int, int, int> fourTuple = {
            get<0>(threeTuple),
            get<1>(threeTuple),
            get<2>(threeTuple),
            0,
        };

        // Return the tuple.
        return fourTuple;
    }
}

/// Reads a file into a list of lines.
///
/// @param path The path to the file to be read.
/// @return The lines in the file.
/// @throws Throws an Exception if the file could not be opened.
vector<string> readFile(string path) {
    // Create an ifstream for the file.
    ifstream file(path);

    // Check if the file is open.
    if(!file.is_open()) {
        // Throw an exception if the file could not be opened.
        throw Exception("Could not open the file at " + path);
    }

    // Create a list for the lines.
    vector<string> lines;

    // Declare a variable for the lines.
    string line;

    // Get lines while any exist.
    while(getline(file, line)) {
        // Add the current line to lines.
        lines.push_back(line);
    }

    // Close the file handle.
    file.close();

    // Return the lines.
    return lines;
}

/// Writes a file from a list of lines.
///
/// @param path The path to the file to be written.
/// @param lines The lines to write to the file.
/// @throws Throws an Exception if a file handle could not be created.
void writeFile(string path, vector<string> lines) {
    // Create an ofstream for the file.
    ofstream file(path);

    // Check if the file is open.
    if(!file.is_open()) {
        // Throw an exception if a file handle could not be created.
        throw Exception("Could not create a file handle for " + path);
    }

    // Process each line.
    for(string line : lines) {
        // Write the current line to the file.
        file << line << "\n";
    }

    // Close the file handle.
    file.close();
}

/// The main method.
///
/// @param argc The number of arguments.
/// @param argv The arguments.
/// @return Zero.
/// @throws Throws an Exception if the number of arguments is less than three.
int main(int argc, char** argv) {
    // Check if the number of arguments is less than three.
    if(argc < 3) {
        // Throw an exception if the number of arguments is less than three.
        throw Exception("Too few arguments.");
    }

    // Get the path of the orignal coverage file to filter.
    string inPath = argv[1];

    // Get the path of the coverage file to create.
    string outPath = argv[2];

    // Read the orignal coverage file.
    vector<string> coverage = readFile(inPath);

    // Create a vector to store the contents of a source file.
    vector<string> source;

    // Create a vector for the coverage file to create.
    vector<string> filteredCoverage;

    // Process each coverage line.
    for(int i = 0; i < coverage.size(); i++) {
        // Get the current line.
        string coverageLine = coverage[i];

        // Check if the current line is a source file line.
        if(startsWith("SF:", coverageLine)) {
            // Get the path to the source file.
            string path = coverageLine.substr(3);

            // Read the source file.
            source = readFile(path);

            // Push the source file line.
            filteredCoverage.push_back(coverageLine);
        }

        // Check if the current line is a branch line.
        else if(startsWith("BRDA:", coverageLine)) {
            // Get the BRDA value from the line.
            tuple<int, int, int, int> values = getBRDAValues(coverageLine);

            // Get the line number.
            int line = get<0>(values);

            // Get the source file line corresponding to the line number.
            string sourceLine = source[line - 1];

            // Check if the line starts with "if".
            if(startsWithTrimmed("if(", sourceLine)) {
                // On macOS, one branch is created for each template version.
                // This means all branches have to be merged to the true and the
                // false case to get full branch coverage.

                // Push the current coverage line.
                filteredCoverage.push_back(coverageLine);

                // Get the first comma.
                int firstComma = coverageLine.find_first_of(',');

                // Create a signature, i.e. a string describing a BRDA line with
                // the current line number.
                string signature = coverageLine.substr(0, firstComma);

                // Describes if cases should continue being squashed.
                bool squash = true;

                // Contains the number of hits on the first branch.
                int firstBranchHits = 0;

                // Contains the number of hits on the second branch.
                int secondBranchHits = 0;

                // Continue while squash is true.
                while(squash) {
                    // Get the next coverage line.
                    string nextCoverageLine = coverage[i + 1];

                    // Check if the next line starts with the signature.
                    if(startsWith(signature, nextCoverageLine)) {
                        // Get the BRDA values for the line.
                        tuple<int, int, int, int> values
                            = getBRDAValues(nextCoverageLine);

                        // Get the branch ID.
                        int branch = get<2>(values);

                        // Get the number of hits on the branch.
                        int hits = get<3>(values);

                        // The branches of the templates are placed in
                        // succession after each other, which means they
                        // alternate with regards to the actual branches.
                        bool onSecondBranch = branch % 2;

                        // Check if the branch is on the second branch.
                        if(onSecondBranch) {
                            // Add the number of hits to the total number of
                            // second branch hits.
                            secondBranchHits += hits;
                        }
                        else {
                            // Add the number of hits to the total number of
                            // first branch hits.
                            firstBranchHits += hits;
                        }

                        // Increment the line index.
                        i++;

                        // Update coverageLine.
                        coverageLine = coverage[i];
                    }
                    else {
                        // Squashing should stop if the next line does not start
                        // with the signature, i.e. it does not describe a
                        // branch in the current if-statement.
                        squash = false;
                    }
                }

                // Create a line for the first branch.
                stringstream firstBranchOut;
                firstBranchOut
                    << "BRDA:"
                    << line
                    << ",0,0,"
                    << firstBranchHits;

                // Push the line for the first branch.
                filteredCoverage.push_back(firstBranchOut.str());

                // Create a line for the second branch.
                stringstream secondBranchOut;
                secondBranchOut
                    << "BRDA:"
                    << line
                    << ",0,1,"
                    << secondBranchHits;

                // Push the line for the second branch.
                filteredCoverage.push_back(secondBranchOut.str());
            }
        }

        // Check if the current line is a function line.
        else if(startsWith("FN:", coverageLine)) {
            // The number of calls to functions are tracked for each template
            // version. This means all functions have to be merged to get full
            // branch coverage.

            // Get the first comma of the line.
            int firstComma = coverageLine.find_first_of(',');

            // Get the line to base a signature on.
            string signatureLine = coverageLine;

            // Create a signature, i.e. a string describing a FN line with
            // the current line number.
            string signature = signatureLine.substr(0, firstComma);

            // Get the function name.
            string functionName = signatureLine.substr(firstComma + 1);

            // Push the signature line.
            filteredCoverage.push_back(signatureLine);
            
            // Contains the number of calls to the function.
            int totalCalls = 0;

            // Describes if function calls should continue being squashed.
            bool squash = true;

            // Continue while squash is true.
            while(squash) {
                // Get the next coverage line.
                string nextCoverageLine = coverage[i + 1];

                // Check if the next coverage line is a line describing the
                // number of calls to the previously specified function.
                if(startsWith("FNDA:", nextCoverageLine)) {
                    // Get the number of calls.
                    int calls = getFirstValue("FNDA:", nextCoverageLine);

                    // Add the number of calls to totalCalls.
                    totalCalls += calls;

                    // Increment the line index.
                    i++;
                    
                    // Update coverageLine.
                    coverageLine = coverage[i];
                }

                // Get the next coverage line.
                nextCoverageLine = coverage[i + 1];

                // Check if the next coverage line starts with the signature.
                if(startsWith(signature, nextCoverageLine)) {
                    // Increment the line index.
                    i++;

                    // Update coverageLine.
                    coverageLine = coverage[i];

                    // This line's FNDA line will be processed during the next
                    // iteration.
                }
                else {
                    // Break the loop since the line is not related to the
                    // function.
                    squash = false;
                }
            }

            // Create a stringstream.
            stringstream out;

            // Create a new FNDA line with the total calls.
            out << "FNDA:" << totalCalls << "," << functionName;

            // Push the new line.
            filteredCoverage.push_back(out.str());
        }

        // Check if the current line is a statement line.
        else if(startsWith("DA:", coverageLine)) {
            // Get the line number.
            int line = getFirstValue("DA:", coverageLine);

            // Get the number of hits on the line.
            int hits = getSecondValue("DA:", coverageLine);

            // Remove any uncovered lines starting with } as certain methods
            // returning values sometimes (especially with Apple clang) reports
            // these lines as uncovered.
            if(hits != 0 || !startsWithTrimmed("}", source[line - 1])) {
                filteredCoverage.push_back(coverageLine);
            }
        }
        else {
            // Push the line without processing by default.
            filteredCoverage.push_back(coverageLine);
        }
    }

    // Write the new coverage file.
    writeFile(outPath, filteredCoverage);

    // Return zero.
    return 0;
}
