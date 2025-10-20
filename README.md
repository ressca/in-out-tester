# Tester for .in and .out files
It runs selected program and test it with all .in files from a folder and compares the output to matching .out files.
Prints results for each test and makes a summary

### Important!
.in and .out files must be a pair
both file in a pair must have the same name, only different extension (.in, .out)

### Needed
g++ c++ compiler

### Compile using this command
`g++ -O3 -static file-name.cpp -std=c++23`

### Running
`./tester program.exe in out`

- program.exe - tested program
- in - path to folder with .in files
- out - path to folder with .out files
