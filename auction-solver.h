#pragma once

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "pstreams-1.0.3/pstream.h"

struct Bid {
    int bidId;
    int value;
    std::vector<int> bidItems;
};

struct Edge {
    int v1;
    int v2;
};

int                 numItems;
int                 numBids;
std::vector<Bid>    bids;
std::vector<Edge>   edges;

int buildConflictGraph(std::string auctionFileName);
int writeGraphToMwvcFile();

/*
 * Build the auction's conflict graph,
 * given an input auction file
 */
int buildConflictGraph(std::string auctionFileName) {
    int numItems, numBids;

    // Create input stream for auction file
    std::ifstream infile(auctionFileName);

    if (infile.is_open()) {
        // Read in the number of items and bids in the auction
        infile >> numItems >> numBids;

        // Consume new line character
        std::string line;
        std::getline(infile, line);

        // Read all lines and populate bids vector
        for (int v = 0; v < numBids; v++) {
            std::getline(infile, line);
            char split_char = ',';

            std::istringstream split(line);
            std::vector<int> bidItems;

            int value;
            split >> value;
            std::cout << "Value: " << value << std::endl;

            for (std::string each; 
                std::getline(split, each, split_char); 
                bidItems.push_back(std::stoi(each))) {
                std::cout << each << std::endl;
            }

            std::cout << std::endl;
        }

        // Initialize map of item: bids that contain it
        // Initialize 
        // Iterate over all bids:
        //     update map accordingly
        //     Make vertex for every bid and add to vector
        // Iterate

        infile.close();
        return writeGraphToMwvcFile();
    }

    return 1;
}

/*
 * Write the auction's conflict graph to a mwvc file,
 * using the desired format
 */
int writeGraphToMwvcFile() {
    // Create input stream for auction file
    std::ofstream outfile("auction.mwvc");

    if (outfile.is_open()) {
        // Write mwvc file header
        outfile << "p edge " << numBids << " " << edges.size() << std::endl;

        // Write vertices to file
        for (Bid bid : bids) {
            outfile << "v " << bid.bidId << " " << bid.value << std::endl;
        }

        // Write edges to file
        for (Edge edge : edges) {
            outfile << "e " << edge.v1 << " " << edge.v2 << std::endl;
        }

        return 0;
    }

    return 1;
}
