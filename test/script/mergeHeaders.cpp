#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::set;
using std::string;
using std::vector;

/// A class implementing std::exception that can be thrown by the program when
/// deemed necessary.
class Exception : public std::exception {
    private:
        /// An explanation of what went wrong.
        std::string message;
    
    public:
        /// Creates an Exception.
        ///
        /// @param message An explanation of what went wrong.
        Exception(std::string message);

        /// An override of std::exception::what() that returns the message.
        ///
        /// @return A constant pointer to the message.
        const char* what() const noexcept override;
};

Exception::Exception(
    std::string message)
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
        // Return an empty string if the string only contains whitespace
        // characters.
        return "";
    }
    else {
        // Remove the initial whitespaces.
        string trimmed = target.substr(firstNonWhitespace);

        // Return the trimmed string.
        return trimmed;
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
        throw Exception("Cannot open file");
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
        throw Exception("Cannot create a file handle");
    }

    // Process each line.
    for(string line : lines) {
        // Write the current line to the file.
        file << line << "\n";
    }

    // Close the file handle.
    file.close();
}

/// Checks if a file exists.
///
/// @param The path of the file to check.
/// @return True if a file exists and false otherwise.
bool fileExists(string path) {
    // Create an ifstream for the file.
    ifstream file(path);

    // A file exists if file.good() is true.
    bool exists = file.good();

    // Close the file handle.
    file.close();

    // Return exists.
    return exists;
}

/// The start of an include statement.
const string includeStart = "#include <";

/// The length of the start of an include statement.
const int includeStartLength = includeStart.length();

/// Processes a certain header.
///
/// @param includeFolder The folder including all headers.
/// @param path The path to the header relative to the include folder.
/// @param internalHeaders A set containing the paths to all known internal
/// headers.
/// @param externalHeaders A set containing the paths to all known external
/// headers.
/// @param regularLines The regular lines to be included in the merged header.
void processHeader(
    string includeFolder,
    string path,
    set<string>& internalHeaders,
    set<string>& externalHeaders,
    vector<string>& regularLines) {
    // Read the header.
    vector<string> lines = readFile(includeFolder + path);

    // Process each line in the header.
    for(string line : lines) {
        // Trim the line.
        string compareLine = trimStart(line);

        // Check if the line is #pragma once.
        if(compareLine == "#pragma once") {
            // Ignore the line if it's #pragma once.
            continue;
        }

        // Get if the line refers to another header.
        bool isHeader = startsWith(includeStart, compareLine);

        // Check isHeader.
        if(!isHeader) {
            // Push the line to regularLines unless it refers to a header.
            regularLines.push_back(line);

            // Continue with next line.
            continue;
        }

        // Get the path to the found header.
        string header = compareLine.substr(
            includeStartLength,
            compareLine.length() - includeStartLength - 1);

        // Check if the header already has been found.
        bool alreadyExists = internalHeaders.count(header) == 0
            && externalHeaders.count(header) == 0;

        // Check alreadyExists.
        if(!alreadyExists) {
            // Continue with the lext line if the header already has been found.
            continue;
        }

        // Get if the header is an internal header by checking if a file with
        // its path exists.
        bool isInternal = fileExists(includeFolder + header);

        // Check isInternal.
        if(!isInternal) {
            // Add the header to externalHeaders unless it's an internal header.
            externalHeaders.insert(header);

            // Continue with the next line.
            continue;
        }

        // Add the header to internalHeaders. 
        internalHeaders.insert(header);

        // Process the header recursively.
        processHeader(
            includeFolder,
            header,
            internalHeaders,
            externalHeaders,
            regularLines);
    }
}

/// Removes any duplicate empty lines.
///
/// @param The list of lines to process.
/// @param A list of lines without duplicate empty lines.
vector<string> removeDuplicateEmptyLines(vector<string>& lines) {
    // Declare a variable for if the previous line is empty and initialize it to
    // false as no lines have been processed yet.
    bool previousEmpty = false;

    // Create a vector for the result.
    vector<string> result;

    // Process each line.
    for(string line : lines) {
        // Check if the line is empty.
        if(line == "") {
            // Check if the previous line is also empty.
            if(!previousEmpty) {
                // Push the line to result if the previous line is not empty.
                result.push_back(line);

                // Set previousEmpty to true as the current line is empty.
                previousEmpty = true;
            }
        }
        else {
            // Push the line to result as the line is not empty.
            result.push_back(line);

            // Set previousEmpty to false as tge current line is not empty.
            previousEmpty = false;
        }
    }

    // Return the result.
    return result;
}

/// Merges all headers reachable from the given root header.
///
/// @param includeFolder The folder including all headers.
/// @param rootHeaderPath The path to the root header.
/// @return The lines in the merged header.
vector<string> mergeHeaders(
    string includeFolder,
    string rootHeaderPath) {
    // Create a set for the internal headers.
    set<string> internalHeaders;

    // Create a set for the external headers.
    set<string> externalHeaders;

    // Create a vector for the outputted header.
    vector<string> regularLines;

    // Call processHeader to process the root header.
    processHeader(
        includeFolder + "/",
        rootHeaderPath,
        internalHeaders,
        externalHeaders,
        regularLines);

    // Create a vector for the lines to be included in the merged header.
    vector<string> mergedHeaderLines;

    // Push a #pragma once statement.
    mergedHeaderLines.push_back("#pragma once\n");

    // Process each external header.
    for(string externalHeader : externalHeaders) {
        // Push the current external header.
        mergedHeaderLines.push_back("#include <" + externalHeader + ">");
    }

    // Process each regular line.
    for(string outHeaderLine : regularLines) {
        // Push the current regular line.
        mergedHeaderLines.push_back(outHeaderLine);
    }

    // Remove any duplicate empty lines.
    vector<string> trimmedMergedHeaderLines =
        removeDuplicateEmptyLines(mergedHeaderLines);

    // Return the trimmed merged header lines.
    return trimmedMergedHeaderLines;
}

/// The main method.
///
/// @param argc The number of arguments.
/// @param argv The arguments.
/// @return Zero.
/// @throws Throws an Exception if the number of arguments is less than four.
int main(int argc, char** argv) {
    // Check if the number of arguments is less than four.
    if(argc < 4) {
        // Throw an exception if the number of arguments is less than four.
        throw Exception("Too few arguments");
    }

    // Get the include folder.
    string includeFolder = argv[1];

    // Get the path to the root header.
    string rootHeaderPath = argv[2];

    // Get the path to write the output to.
    string outHeaderPath = argv[3];

    // Merge the headers.
    vector<string> mergedHeaderLines = mergeHeaders(
        includeFolder,
        rootHeaderPath);

    // Write a file with the output header.
    writeFile(outHeaderPath, mergedHeaderLines);
}
