#include <chrono>
#include <exception>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::system_clock;
using std::cout;
using std::endl;
using std::gmtime;
using std::ifstream;
using std::ofstream;
using std::map;
using std::pair;
using std::put_time;
using std::string;
using std::stringstream;
using std::vector;

/// The current version of IMock.
const string version = "1.0.0";

/// Defines which characters counts as whitespaces.
const std::string whitespaces = " \t\n\r";

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

/// Checks if a target is empty or only contains whitespaces.
///
/// @param target The target string to be checked.
/// @return True if target is empty or only contains whitespaces and false
/// otherwise.
bool isBlank(string target) {
    // Get the first character that is not a whitespace.
    int firstNonWhitespace = target.find_first_not_of(whitespaces);

    // Check if any non-whitespace characters exist.
    bool blank = firstNonWhitespace < 0;

    // Return blank.
    return blank;
}

/// Removes any initial or ending whitespaces.
///
/// @param target The string to be trimmed.
/// @return The target without any initial or ending whitespaces.
string trim(string target) {
    // Get the first character that is not a whitespace.
    int firstNonWhitespace = target.find_first_not_of(whitespaces);
    
    // Get the last character that is not a whitespace.
    int lastNonWhitespace = target.find_last_not_of(whitespaces);

    // Check if any non-whitespace characters exist.
    if(firstNonWhitespace < 0 || lastNonWhitespace < 0) {
        // Return an empty string if the string only contains whitespace
        // characters.
        return "";
    }
    else {
        // Remove the initial and ending whitespaces.
        string trimmed = target.substr(
            firstNonWhitespace,
            lastNonWhitespace + 1 - firstNonWhitespace);

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

/// Checks if a path points to a file.
///
/// @param path The path to check.
/// @return True if the path points to a file and false otherwise.
bool isFile(string path) {
    // Declare a variable for the result of stat.
    struct stat statInfo;

    // Call stat with the path.
    int statCode = stat(path.c_str(), &statInfo);

    // The path points to a file if statCode is zero (indicating something
    // exists) and statInfo.st_mode has the S_IFREG (indicating the found object
    // is a file).
    bool isFile = statCode == 0 && statInfo.st_mode & S_IFREG;

    // Return isFile.
    return isFile;
}

/// The start of an include statement.
const string includeStart = "#include <";

/// The length of the start of an include statement.
const int includeStartLength = includeStart.length();

/// Processes a certain header.
///
/// @param includeFolder The folder including all headers.
/// @param path The path to the header relative to the include folder.
/// @param pragmaOnceLine The line to be written as #pragma once.
/// @param internalHeaders A map containing the paths to all known internal
/// headers together with a raw line including the header.
/// @param externalHeaders A map containing the paths to all known external
/// headers together with a raw line including the header.
/// @param regularLines The regular lines to be included in the merged header.
void processHeader(
    string includeFolder,
    string path,
    string& pragmaOnceLine,
    map<string, string>& internalHeaders,
    map<string, string>& externalHeaders,
    vector<string>& regularLines) {
    // Read the header.
    vector<string> lines = readFile(includeFolder + path);

    // Process each line in the header.
    for(vector<string>::iterator iterator = lines.begin();
        iterator != lines.end();
        iterator++) {
        // Get the current line.
        string line = *iterator;

        // Trim the line.
        string compareLine = trim(line);

        // Check if the line is #pragma once.
        if(compareLine == "#pragma once") {
            // Check if pragmaOnceLine has been set.
            if(pragmaOnceLine.empty()) {
                // Set the line if not already set.

                // Check if any line exists below the #pragma once line and if
                // it's blank.
                string nextLine;
                if(iterator + 1 < lines.end()
                    && isBlank(nextLine = *(iterator + 1))) {
                    // Set pragmaOnceLine together with the next line.
                    pragmaOnceLine = line + "\n" + nextLine;
                }
                else {
                    // Set pragmaOnceLine to only the line itself.
                    pragmaOnceLine = line;
                }
            }

            // Do not process the line any further.
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
        bool isInternal = isFile(includeFolder + header);

        // Check isInternal.
        if(!isInternal) {
            // Add the header to externalHeaders unless it's an internal header.
            externalHeaders[header] = line;

            // Continue with the next line.
            continue;
        }

        // Add the header to internalHeaders.
        internalHeaders[header] = line;

        // Process the header recursively.
        processHeader(
            includeFolder,
            header,
            pragmaOnceLine,
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
        if(isBlank(line)) {
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

/// Gets the current time in UTC as a string.
///
/// @return The current time in UTC as a string.
string getCurrentTime() {
    // Get the current time.
    auto now = system_clock::now();

    // Convert the current time to time_t.
    time_t nowTime = system_clock::to_time_t(now);

    // Get the microsecond part of the current time.
    auto timeMicroseconds = duration_cast<microseconds>(now.time_since_epoch())
        % 1000000;

    // Create a stringstream.
    stringstream stringstream;
    stringstream
        // Print the current date and time.
        << put_time(gmtime(&nowTime), "%F %T")

        // Print a period as separator.
        << "."

        // Print the microsecond part.
        << std::setfill('0') << std::setw(6) << timeMicroseconds.count();

    // Create a string and return it.
    return stringstream.str();
}

/// Creates lines containing a comment with information about the version,
/// copyright and when the single header was generated.
///
/// @return The lines in the head comment.
vector<string> createHeadComment() {
    // Read the LICENSE file.
    vector<string> licenseLines = readFile("LICENSE");

    // Get the first line in the LICENSE file.
    string firstLicenseLine = licenseLines[0];

    // Get the potential whitespace ending of the first line.
    string lineEnding = firstLicenseLine.substr(
        firstLicenseLine.find_last_not_of(whitespaces) + 1);

    // Create a vector for the head comment.
    vector<string> headCommentLines;

    // Add a line starting a multiline comment.
    headCommentLines.push_back("/*" + lineEnding);

    // Add a line with the library name and the version.
    headCommentLines.push_back("IMock " + version + lineEnding);

    // Get the current time.
    string currentTime = getCurrentTime();

    // Add a line with the current time.
    headCommentLines.push_back("Generated " + currentTime + " UTC"
        + lineEnding);

    // Add an empty line.
    headCommentLines.push_back(lineEnding);

    // Process each LICENSE file line.
    for(string licenseLine : licenseLines) {
        // Add the current LICENSE file line.
        headCommentLines.push_back(licenseLine);
    }

    // Add a line ending the multiline comment. 
    headCommentLines.push_back("*/" + lineEnding);

    // Add an empty line.
    headCommentLines.push_back(lineEnding);

    // Return the created head comment lines.
    return headCommentLines;
}

/// Merges all headers reachable from the given root header.
///
/// @param includeFolder The folder including all headers.
/// @param rootHeaderPath The path to the root header.
/// @return The lines in the merged header.
vector<string> mergeHeaders(
    string includeFolder,
    string rootHeaderPath) {
    // Create an empty string to be assigned to a #pragma once line.
    string pragmaOnceLine = "";

    // Create a set for the internal headers.
    map<string, string> internalHeaders;

    // Create a set for the external headers.
    map<string, string> externalHeaders;

    // Create a vector for the outputted header.
    vector<string> regularLines;

    // Call processHeader to process the root header.
    processHeader(
        includeFolder + "/",
        rootHeaderPath,
        pragmaOnceLine,
        internalHeaders,
        externalHeaders,
        regularLines);

    // Create a vector for the lines to be included in the merged header.
    vector<string> mergedHeaderLines;

    // Push the #pragma once line.
    mergedHeaderLines.push_back(pragmaOnceLine);

    // Create head comment lines.
    vector<string> headCommentLines = createHeadComment();

    // Process each head comment line.
    for(string headCommentLine : headCommentLines) {
        // Push the current head comment line.
        mergedHeaderLines.push_back(headCommentLine);
    }

    // Process each external header.
    for(pair<string, string> externalHeader : externalHeaders) {
        // Push the current external header.
        mergedHeaderLines.push_back(externalHeader.second);
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
