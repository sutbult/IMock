#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::ifstream;
using std::ofstream;
using std::stringstream;

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

bool startsWithNoSpace(string start, string target) {
    string targetStart = target.substr(0, start.length());
    bool match = targetStart == start;
    return match;
}

string trimStart(string target) {
    int firstNonWhitespace = target.find_first_not_of(" \t\n\r");

    if(firstNonWhitespace < 0) {
        return target;
    }
    else {
        string trimmed = target.substr(firstNonWhitespace);
        return trimmed;
    }
}

bool startsWith(string start, string target) {
    string trimmedTarget = trimStart(target);
    bool result = startsWithNoSpace(start, trimmedTarget);
    return result;
}

int getFirstValue(string start, string target) {
    int startLength = start.length();
    int firstComma = target.find_first_of(',');
    string stringLine = target.substr(startLength, firstComma - startLength);
    int value = stoi(stringLine);
    return value;
}

int getSecondValue(string start, string target) {
    int startLength = start.length();
    int firstComma = target.find_first_of(',');
    string stringLine = target.substr(firstComma + 1);
    int value = stoi(stringLine);
    return value;
}

vector<string> readFile(string path) {
    ifstream file(path);
    if(!file.is_open()) {
        throw Exception("Cannot open file");
    }
    else {
        vector<string> lines;
        string line;
        while(getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
        return lines;
    }
}

void writeFile(string path, vector<string> lines) {
    ofstream file(path);
    if(!file.is_open()) {
        throw Exception("Cannot open file");
    }
    else {
        for(string line : lines) {
            file << line << "\n";
        }
        file.close();
    }
}

int main(int argc, char** argv) {
    if(argc < 3) {
        throw Exception("Too few arguments");
    }
    
    string inPath = argv[1];
    string outPath = argv[2];

    vector<string> coverage = readFile(inPath);
    vector<string> source;
    vector<string> filteredCoverage;
    bool hasSource = false;

    for(int i = 0; i < coverage.size(); i++) {
        string coverageLine = coverage[i];

        if(startsWithNoSpace("SF:", coverageLine)) {
            string path = coverageLine.substr(3);
            source = readFile(path);
            hasSource = true;
            filteredCoverage.push_back(coverageLine);
        }
        else if(startsWithNoSpace("BRDA:", coverageLine)) {
            int line = getFirstValue("BRDA:", coverageLine);
            string sourceLine = source[line - 1];

            if(startsWith("if(", sourceLine)) {
                filteredCoverage.push_back(coverageLine);
            }
        }
        else if(startsWithNoSpace("FN:", coverageLine)) {
            int firstComma = coverageLine.find_first_of(',');
            string signatureLine = coverageLine;
            string signature = signatureLine.substr(0, firstComma);
            string functionName = signatureLine.substr(firstComma + 1);
            filteredCoverage.push_back(signatureLine);
            
            int totalCalls = 0;

            bool squash = true;
            while(squash) {
                string nextCoverageLine = coverage[i + 1];

                if(startsWith("FNDA:", nextCoverageLine)) {
                    int calls = getFirstValue("FNDA:", nextCoverageLine);
                    totalCalls += calls;

                    i++;
                    coverageLine = coverage[i];
                }

                nextCoverageLine = coverage[i + 1];
                if(startsWithNoSpace(signature, nextCoverageLine)) {
                    i++;
                    coverageLine = coverage[i];
                }
                else {
                    squash = false;
                }
            }

            std::stringstream out;
            out << "FNDA:" << totalCalls << "," << functionName;
            filteredCoverage.push_back(out.str());
        }
        else if(startsWithNoSpace("DA:", coverageLine)) {
            int line = getFirstValue("DA:", coverageLine);
            int covered = getSecondValue("DA:", coverageLine);

            // Remove any uncovered lines starting with } as certain methods
            // returning values sometimes (especially with Apple clang) reports
            // these lines as uncovered.
            if(covered != 0 || !startsWith("}", source[line - 1])) {
                filteredCoverage.push_back(coverageLine);
            }
        }
        else {
            filteredCoverage.push_back(coverageLine);
        }
    }
    writeFile(outPath, filteredCoverage);
    return 0;
}
