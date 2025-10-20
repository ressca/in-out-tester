#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <windows.h>

namespace fs = std::filesystem;

// ANSI color codes for Windows console
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"

struct TestResult {
    std::string testName;
    bool passed;
    std::string expected;
    std::string actual;
};

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Function to get first line from a string
std::string getFirstLine(const std::string& str) {
    size_t pos = str.find('\n');
    if (pos == std::string::npos) {
        return str; // Return entire string if no newline found
    }
    return str.substr(0, pos);
}

std::string executeProgram(const std::string& program, const std::string& inputFile) {
    // Use type command to pass file content directly to program
    std::string command = "type \"" + inputFile + "\" | \"" + program + "\"";
    
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        return "ERROR: Could not execute program";
    }
    
    char buffer[128];
    std::string result = "";
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    int status = _pclose(pipe);
    
    return result;
}

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    
    if (start == std::string::npos) {
        return "";
    }
    
    return str.substr(start, end - start + 1);
}

bool compareOutput(const std::string& expected, const std::string& actual, bool compareFirstLineOnly, bool showAll) {
    if (compareFirstLineOnly) {
        // Compare only first lines
        std::string expectedFirstLine = trim(getFirstLine(expected));
        std::string actualFirstLine = trim(getFirstLine(actual));
        
        // Show detailed comparison only if -show-all flag is present
        if (showAll) {
            std::cout << "  First line comparison:\n";
            std::cout << "  Expected: '" << expectedFirstLine << "'\n";
            std::cout << "  Actual:   '" << actualFirstLine << "'\n";
            std::cout << "  Match: " << (expectedFirstLine == actualFirstLine ? COLOR_GREEN "YES" COLOR_RESET : COLOR_RED "NO" COLOR_RESET) << "\n";
        }
        
        return expectedFirstLine == actualFirstLine;
    } else {
        // Compare entire output
        return trim(expected) == trim(actual);
    }
}

std::vector<TestResult> runTests(const std::string& testProgram, 
                                const std::string& inputDir, 
                                const std::string& outputDir,
                                bool compareFirstLineOnly,
                                bool showAll) {
    std::vector<TestResult> results;
    
    // Check if the test program exists
    if (!fs::exists(testProgram)) {
        std::cerr << COLOR_RED << "Error: Test program '" << testProgram << "' does not exist!" << COLOR_RESET << "\n";
        return results;
    }
    
    // Check if the input directory exists
    if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
        std::cerr << COLOR_RED << "Error: Input directory '" << inputDir << "' does not exist!" << COLOR_RESET << "\n";
        return results;
    }
    
    // Check if the output directory exists
    if (!fs::exists(outputDir) || !fs::is_directory(outputDir)) {
        std::cerr << COLOR_RED << "Error: Output directory '" << outputDir << "' does not exist!" << COLOR_RESET << "\n";
        return results;
    }
    
    // Search for .in files in the input directory
    for (const auto& entry : fs::directory_iterator(inputDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".in") {
            std::string inputFile = entry.path().string();
            std::string testName = entry.path().stem().string();
            std::string outputFile = (fs::path(outputDir) / (testName + ".out")).string();
            
            // Check if the corresponding .out file exists
            if (!fs::exists(outputFile)) {
                std::cout << COLOR_YELLOW << testName << " - no matching .out file" << COLOR_RESET << "\n";
                continue;
            }
            
            // Show debug information only if -show-all flag is present
            if (showAll) {
                std::cout << COLOR_CYAN << "=== All input and output info: " << testName << " ===" << COLOR_RESET << "\n";
                std::string inputContent = readFile(inputFile);
                std::cout << "Input content:\n" << inputContent << "\n";
            }
            
            // Execute the test program with the given input
            // Input file is always fully used
            std::string actualOutput = executeProgram(testProgram, inputFile);
            std::string expectedOutput = readFile(outputFile);
            
            // Show output details only if -show-all flag is present
            if (showAll) {
                if (compareFirstLineOnly) {
                    std::cout << "Expected output (full):\n" << expectedOutput << "\n";
                    std::cout << "Actual output (full):\n" << actualOutput << "\n";
                    std::cout << "Comparing FIRST LINES only:\n";
                } else {
                    std::cout << "Expected output:\n" << expectedOutput << "\n";
                    std::cout << "Actual output:\n" << actualOutput << "\n";
                }
            }
            
            // Compare the results with the specified mode
            bool passed = compareOutput(expectedOutput, actualOutput, compareFirstLineOnly, showAll);
            
            // Color-coded result
            if (passed) {
                std::cout << testName << " - " << COLOR_GREEN << "true" << COLOR_RESET << "\n";
            } else {
                std::cout << testName << " - " << COLOR_RED << "false" << COLOR_RESET << "\n";
            }
            
            // Close debug section if -show-all flag is present
            if (showAll) {
                std::cout << COLOR_CYAN << "========================" << COLOR_RESET << "\n\n";
            }
            
            results.push_back({testName, passed, expectedOutput, actualOutput});
        }
    }
    
    return results;
}

void printSummary(const std::vector<TestResult>& results, bool compareFirstLineOnly, bool showAll) {
    int passedCount = 0;
    for (const auto& result : results) {
        if (result.passed) {
            passedCount++;
        }
    }
    
    std::cout << "\n" << COLOR_BLUE << "=== SUMMARY ===" << COLOR_RESET << "\n";
    
    // Color-coded summary lines
    std::cout << "Tests passed: " << COLOR_GREEN << passedCount << COLOR_RESET << "\n";
    std::cout << "Tests failed: " << COLOR_RED << (results.size() - passedCount) << COLOR_RESET << "\n";
    std::cout << "Total tests: " << results.size() << "\n";
    
    // Display comparison mode information
    if (compareFirstLineOnly) {
        std::cout << "Comparison mode: " << COLOR_YELLOW << "First line only" << COLOR_RESET << "\n";
    } else {
        std::cout << "Comparison mode: " << COLOR_YELLOW << "Full output" << COLOR_RESET << "\n";
    }
    
    // Display debug mode information if -show-all was used
    if (showAll) {
        std::cout << COLOR_YELLOW << "Detailed output shown" << COLOR_RESET << "\n";
    }
}

// Function to enable ANSI escape codes on Windows
void enableANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

int main(int argc, char* argv[]) {
    // Enable ANSI escape codes for colors on Windows
    enableANSI();
    
    bool compareFirstLineOnly = false;
    bool showAll = false;
    std::string testProgram, inputDir, outputDir;
    
    // Parse command line arguments
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <test_program> <input_folder> <output_folder> [-one] [-show-all]\n";
        std::cout << "Example: " << argv[0] << " test.exe in out\n";
        std::cout << "Example with first-line comparison: " << argv[0] << " test.exe in out -one\n";
        std::cout << "Example with debug output: " << argv[0] << " test.exe in out -show-all\n";
        std::cout << "Example with both flags: " << argv[0] << " test.exe in out -one -show-all\n";
        return 1;
    }
    
    testProgram = argv[1];
    inputDir = argv[2];
    outputDir = argv[3];
    
    // Check for flags
    for (int i = 4; i < argc; i++) {
        if (std::string(argv[i]) == "-one") {
            compareFirstLineOnly = true;
        } else if (std::string(argv[i]) == "-show-all") {
            showAll = true;
        }
    }
    
    auto results = runTests(testProgram, inputDir, outputDir, compareFirstLineOnly, showAll);
    
    if (!results.empty()) {
        printSummary(results, compareFirstLineOnly, showAll);
    }
    
    return 0;
}