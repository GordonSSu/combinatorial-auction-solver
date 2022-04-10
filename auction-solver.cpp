#include <sstream>
#include "auction-solver.h"

int main(int argc, char *argv[]) {
    // Input auction file name missing
    if (argc < 2) {
        std::cerr << "Missing argument(s)." << std::endl;
        std::cout << "Usage: ./auction-solver [auction file name]" << std::endl;
        return 1;
    }

    // Read in name of input auction file
    std::string auctionFileName = argv[1];

    // Read all auction information from input file
    if (readAuction(auctionFileName) != 0) {
        std::cerr << "Error reading from auction file." << std::endl;
        return 1;
    }

    // Build the conflict graph and write to mwvc file
    buildConflictGraph();
    if (writeGraphToMwvcFile() != 0) {
        std::cerr << "Error writing conflict graph to mwvc file." << std::endl;
        return 1;
    }

    // Compile and run fastmwvc solver on conflict graph
    const char *compileFastwvcCommand = 
        "g++ fastwvc/mwvc.cpp -O3 --std=c++11 -o mwvc";
    system(compileFastwvcCommand);

    // Run fastwvc and create a streambuf that reads its stdout and stderr
    const char *runFastwvcCommand = "./mwvc auction.mwvc 0 1 0";
    redi::ipstream proc(runFastwvcCommand, redi::pstreams::pstdout);

    // Read fastwvc's stdout
    std::string fastwvcOutputLine1;
    std::string fastwvcOutputLine2;
    std::getline(proc.out(), fastwvcOutputLine1);
    std::getline(proc.out(), fastwvcOutputLine2);

    // Output optimal auction results
    if (outputOptimalAuction(fastwvcOutputLine1, fastwvcOutputLine2) != 0) {
        std::cerr << "Error writing optimal auction results to file." << std::endl;
        return 1;
    }

    return 0;
}
