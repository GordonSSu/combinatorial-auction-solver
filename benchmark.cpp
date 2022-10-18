#include <chrono>
#include <sstream>
#include "benchmark.h"

template <class T>
__attribute__((always_inline)) inline void DoNotOptimize(const T &value) {
    asm volatile("" : "+m"(const_cast<T &>(value)));
}

int main(int argc, char *argv[]) {
    // Timepoints for benchmarking
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

    bool solvedByKernelization;

    // Input arguments missing
    if (argc < 3) {
        std::cerr << "Missing argument(s)." << std::endl;
        std::cout << "Usage: ./auction-solver [auction file name] [kernalization flag]" << std::endl;
        return 1;
    }

    // Read in name of input auction file
    std::string auctionFileName = argv[1];


    /****************************************************************************************************

    FastWVC Benchmark

    ****************************************************************************************************/


    // Output header
    std::cout << "============================FastWVC============================" << std::endl;

    // Read all auction information from input file
    if (readCatsAuctionMwvc(auctionFileName) != 0) {
        std::cerr << "Error reading from auction file." << std::endl;
        return 1;
    }

    // Build the conflict graph
    buildConflictGraph();

    // Kernalize
    if (argv[2][0] != '0') {
        // Begin benchmark
        startTime = std::chrono::high_resolution_clock::now();
        int kernalizationOutput = kernalize();

        // Kernalization error
        if (kernalizationOutput == -1) {
            return 1;
        }

        // Kernalization has "solved" the problem
        else if (kernalizationOutput == 0) {
            // Output benchmarking results
            endTime = std::chrono::high_resolution_clock::now();
            auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
            auto duration = end - start;
            auto ms = duration * 0.001;
            std::cout << "FastWVC - solved via kernalization (ms): " << ms << std::endl;
            outputOptimalAuction("", "");
            solvedByKernelization = true;
        }
    } else {
        // Begin benchmark
        startTime = std::chrono::high_resolution_clock::now();
    }

    if (!solvedByKernelization) {
        // Write conflict graph to mwvc file
        if (writeGraphToMwvcFile() != 0) {
            std::cerr << "Error writing conflict graph to mwvc file." << std::endl;
            return 1;
        }

        // Compile and run fastmwvc solver on conflict graph
        const char *compileFastWvc = "g++ fastwvc/mwvc.cpp -O3 --std=c++11 -o mwvc";
        system(compileFastWvc);

        // Run fastwvc and create a streambuf that reads its stdout and stderr
        const char *runFastWvc = "./mwvc auction.mwvc 0 1 0";
        redi::ipstream proc(runFastWvc, redi::pstreams::pstdout);

        // Read fastwvc's stdout
        std::string fastwvcOutputLine1;
        std::string fastwvcOutputLine2;
        std::getline(proc.out(), fastwvcOutputLine1);
        std::getline(proc.out(), fastwvcOutputLine2);

        endTime = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
        auto duration = end - start;
        auto ms = duration * 0.001;
        std::cout << "FastWVC (ms): " << ms << std::endl;

        // Output optimal auction results
        if (outputOptimalAuction(fastwvcOutputLine1, fastwvcOutputLine2) != 0) {
            std::cerr << "Error writing optimal auction results to file." << std::endl;
            std::cerr << fastwvcOutputLine1 << std::endl;
            std::cerr << fastwvcOutputLine2 << std::endl;
        }
    }


    /****************************************************************************************************

    Gurobi MWVC Benchmark

    ****************************************************************************************************/


    try {
        // Output header
        std::cout << "\n============================Gurobi MWVC Formulation============================" << std::endl;

        resetState();
        // Read in auction and build conflict graph
        readCatsAuctionMwvc(auctionFileName);
        buildConflictGraph();
        startTime = std::chrono::high_resolution_clock::now();

        long long bestGurobiMwvcAuction = gurobiMwvcSolve();

        // Output benchmarking results
        endTime = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
        auto duration = end - start;
        auto ms = duration * 0.001;
        std::cout << "Gurobi MWVC (ms): " << ms << std::endl;
        std::cout << "MAX AUCTION VALUE: " << bestGurobiMwvcAuction << std::endl;
    } catch (...) {
        std::cerr << "Error in Gurobi MWVC benchmark." << std::endl;
    }


    /****************************************************************************************************

    Kernalized Gurobi MWVC Benchmark

    ****************************************************************************************************/


    // // Output header
    // std::cout << "\n============================Gurobi MWVC Formulation============================" << std::endl;

    // resetState();
    // // Read in auction and build conflict graph
    // readCatsAuctionMwvc(auctionFileName);
    // buildConflictGraph();

    // // Kernalize
    // if (argv[2][0] != '0') {
    //     // Begin benchmark
    //     startTime = std::chrono::high_resolution_clock::now();
    //     int kernalizationOutput = kernalize();

    //     // Kernalization error
    //     if (kernalizationOutput == -1) {
    //         return 1;
    //     }

    //     // Kernalization has "solved" the problem
    //     else if (kernalizationOutput == 0) {
    //         // Output benchmarking results
    //         endTime = std::chrono::high_resolution_clock::now();
    //         auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
    //         auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
    //         auto duration = end - start;
    //         auto ms = duration * 0.001;
    //         std::cout << "Gurobi MWVC - solved via kernalization (ms): " << ms << std::endl;
    //         outputOptimalAuction("", "");
            
    //         solvedByKernelization = true;
    //     }
    // } else {
    //     // Begin benchmark
    //     startTime = std::chrono::high_resolution_clock::now();
    // }

    // if (!solvedByKernelization) {
    //     long long bestGurobiMwvcAuction = gurobiMwvcSolve();

    //     // Output benchmarking results
    //     endTime = std::chrono::high_resolution_clock::now();
    //     auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
    //     auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
    //     auto duration = end - start;
    //     auto ms = duration * 0.001;
    //     std::cout << "Gurobi MWVC (ms): " << ms << std::endl;
    //     std::cout << "MAX AUCTION VALUE: " << bestGurobiMwvcAuction << std::endl;
    // }


    /****************************************************************************************************

    Gurobi Set Packing Benchmark

    ****************************************************************************************************/


    try {
        // Output header
        std::cout << "\n============================Gurobi Set Packing Formulation============================" << std::endl;

        resetState();
        // Read in auction and build conflict graph
        readCatsAuctionSetPacking(auctionFileName);
        startTime = std::chrono::high_resolution_clock::now();

        long long bestGurobiSetPackingAuction = gurobiSetPackingSolve();

        // Output benchmarking results
        endTime = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
        auto duration = end - start;
        auto ms = duration * 0.001;
        std::cout << "Gurobi Set Packing (ms): " << ms << std::endl;
        std::cout << "MAX AUCTION VALUE: " << bestGurobiSetPackingAuction << std::endl << std::endl;
    } catch (...) {
        std::cerr << "Error in Gurobi set packing benchmark." << std::endl;
    }


    /***************************************************************************************************

    MiniZinc Benchmark

    ***************************************************************************************************/


    // // Output header
    // std::cout << "\n============================MiniZinc============================" << std::endl;

    // resetState();
    // std::string dznFileName = convertToDzn(auctionFileName);

    // // Command to run minizinc
    // std::string runMiniZincString = "minizinc --solver Gecode combauct.mzn " + dznFileName;
    // const char *runMiniZinc = (const char *) runMiniZincString.c_str();

    // // Benchmark
    // startTime = std::chrono::high_resolution_clock::now();
    // system(runMiniZinc);
    // endTime = std::chrono::high_resolution_clock::now();
    
    // auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
    // auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
    // auto duration = end - start;
    // auto ms = duration * 0.001;
    // std::cout << "Minizinc (ms): " << ms << std::endl;

    return 0;
}
