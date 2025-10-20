# Tester for .in and .out files

It runs selected program and test it with all .in files from a folder and compares the output to matching .out files.
Prints results for each test and makes a summary

### Important!

.in and .out files must be a pair
both file in a pair must have the same name, only different extension (.in, .out)

### Needed

g++ c++ compiler

### Compile using this command

`g++ -o tester.exe tester.cpp -std=c++17 -lole32 -loleaut32 -luuid`

### Running

`tester.exe program.exe in out` - to test entire output

Available flags
`-one` - compares with only the first line from .out file
`-show-all` - shows whole input, outout and expected output

- program.exe - tested program
- in - path to folder with .in files
- out - path to folder with .out files
