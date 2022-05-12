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

smatch getMatch(
    const regex& regex,
    const string& target,
    int values) {
    sregex_iterator begin(target.begin(), target.end(), regex);
    sregex_iterator end;

    int matches = distance(begin, end);

    if(matches != 1) {
        cout << matches << " " << values << endl;
        throw Exception("The number of matches must be one. Target: " + target);
    }

    smatch match = *begin;

    if(match.end() - match.begin() < values + 1) {
        throw Exception("The number of values are too few. Target: " + target);
    }

    return match;
}

tuple<int, int, int> getThreeValues(
    const regex& regex,
    string target) {
    smatch match = getMatch(regex, target, 3);

    tuple<int, int, int> values = {
        stoi(match[1]),
        stoi(match[2]),
        stoi(match[3]),
    };

    return values;
}

tuple<int, int, int, int> getFourValues(
    const regex& regex,
    string target) {
    smatch match = getMatch(regex, target, 4);

    tuple<int, int, int, int> values = {
        stoi(match[1]),
        stoi(match[2]),
        stoi(match[3]),
        stoi(match[4]),
    };

    return values;
}

tuple<int, int, int, int> getBRDAValues(string coverageLine) {
    if(coverageLine.find_first_of('-') == string::npos) {
        regex brdaRegex(
            "BRDA:(\\d+),(\\d+),(\\d+),(\\d+)",
            ECMAScript);

        return getFourValues(brdaRegex, coverageLine);
    }
    else {
        regex brdaRegex(
            "BRDA:(\\d+),(\\d+),(\\d+),-",
            ECMAScript);
        
        tuple<int, int, int> threeTuple
            = getThreeValues(brdaRegex, coverageLine);

        tuple<int, int, int, int> fourTuple = {
            get<0>(threeTuple),
            get<1>(threeTuple),
            get<2>(threeTuple),
            0,
        };

        return fourTuple;
    }
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
            tuple<int, int, int, int> values = getBRDAValues(coverageLine);
            int line = get<0>(values);
            string sourceLine = source[line - 1];

            if(startsWith("if(", sourceLine)) {
                filteredCoverage.push_back(coverageLine);

                int firstComma = coverageLine.find_first_of(',');
                string signature = coverageLine.substr(0, firstComma);

                bool squash = true;
                int firstBranchHits = 0;
                int secondBranchHits = 0;

                while(squash) {
                    string nextCoverageLine = coverage[i + 1];
                    if(startsWithNoSpace(signature, nextCoverageLine)) {
                        tuple<int, int, int, int> values
                            = getBRDAValues(nextCoverageLine);

                        int line = get<0>(values);
                        int branch = get<2>(values);
                        int hits = get<3>(values);

                        bool onSecondBranch = branch % 2;
                        if(onSecondBranch) {
                            secondBranchHits += hits;
                        }
                        else {
                            firstBranchHits += hits;
                        }

                        i++;
                        coverageLine = coverage[i];
                    }
                    else {
                        squash = false;
                    }
                }

                stringstream firstBranchOut;
                firstBranchOut
                    << "BRDA:"
                    << line
                    << ",0,0,"
                    << firstBranchHits;

                filteredCoverage.push_back(firstBranchOut.str());

                stringstream secondBranchOut;
                secondBranchOut
                    << "BRDA:"
                    << line
                    << ",0,1,"
                    << secondBranchHits;

                filteredCoverage.push_back(secondBranchOut.str());
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

            stringstream out;
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
