#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
// #include "pstreams-1.0.3/pstream.h"

#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>


/*
 * UNFINISHED: script for automating CATS testing/benchmarking
 */
int main(int argc, char *argv[]) {
    int numIterations = std::stoi(argv[1]);
    int numGoods = std::stoi(argv[2]);
    int numBids = std::stoi(argv[3]);
    std::string distribution = argv[4];
    std::string outputFileName = argv[5];
    std::ofstream outputfile;
    outputfile.open(outputFileName);

    // const char *compileBenchmarkCmd = "g++ -std=c++11 -m64 -g benchmark.cpp -o benchmark -Iinclude/ -Llib -lgurobi_c++ -lgurobi95 -lm";
    // std::string generateAuctionString = "CATS/cats.exe -goods " + std::to_string(numGoods) + " -bids " + std::to_string(numBids) + " -int_prices -d \"" + distribution + "\"";
    // const char *generateAuctionCmd = generateAuctionString.c_str();
    // system(compileBenchmarkCmd);

    // for (int i = 0; i < numIterations; i++) {
    //     system(generateAuctionCmd);

    //     const char *runBenchmark = "./benchmark CATS/0000.txt 1";
    //     redi::ipstream proc(runBenchmark, redi::pstreams::pstdout);
    //     std::string outputLine;

    //     while (std::getline(proc.out(), outputLine)) {
    //         outputfile << outputLine << std::endl;
    //     }
    // }

    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("ls -a", "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    std::cout << result;

    outputfile.close();
}

// #include <boost/filesystem.hpp>
// #include <boost/foreach.hpp>
// #include <boost/regex.hpp>
// #include <sstream>
// #include <vector>

// int main(int argc, char *argv[]) {
//     // Initialize test parameters
//     int numTests = 20;
//     int numGoods = 10;
//     int numBids = 20;

//     // // Generate combinatorial auction test files
//     // if (generateTestFiles(numTests, numGoods, numBids) != 0) {
//     //     std::cerr << "Error generating test files" << std::endl;
//     //     return 1;
//     // }

//     // Define regular expression for CATS generated test files
//     const boost::regex my_filter("^[0-9]{4}.txt$");

//     // Vector to hold all test file names
//     std::vector<std::string> testFileNames;

//     // Filepath of CATS test file directory
//     boost::filesystem::path filepath("/Users/gordonsu/Documents/combinatorial-auction-solver/CATS");

//     // Define directory iterators
//     boost::filesystem::directory_iterator it(filepath);
//     boost::filesystem::directory_iterator end;

//     // Iterate though CATS test file directory
//     BOOST_FOREACH(boost::filesystem::path const &p, std::make_pair(it, end)) {
//          if(boost::filesystem::is_regular_file(p)) {
//               boost::match_results<std::string::const_iterator> what;

//               // Append test file names to vector
//               if (regex_search(it->path().filename().string(), what, my_filter, boost::match_default)) {
//                    std::string res = what[1];
//                    testFileNames.push_back(res);
//               }
//          }
//     }

//     // // Iterate over all test files
//     // for (std::string fileName : testFileNames) {
//     //     // Benchmark fastWVC pipeline
//     //     benchmarkFastWVCPipeline(fileName);

//     //     // Convert to dzn file
//     //     dnzFileName = convertToDzn(fileName);

//     //     // Benchmark our pipeline
//     //     benchmarkMiniZinc(dnzFileName);
//     // }

//     return 0;
// }