#pragma once

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "pstreams-1.0.3/pstream.h"

struct Bid {
    int bidId;
    int setId;
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

bool intersects();
int readAuction(std::string auctionFileName);
void buildConflictGraph();
int writeGraphToMwvcFile();

/*
 * Returns whether two sorted vectors intersect
 */
bool intersects() {
    // TODO
    return false;
}

/*
 * Initializes all bids in bids vector
 * given an input auction file
 */
int readAuction(std::string auctionFileName) {
    int numItems, numBids;

    // Create input stream for auction file
    std::ifstream infile(auctionFileName);

    if (infile.is_open()) {
        // Read in the number of items and bids in the auction
        infile >> numItems >> numBids;

        // Consume new line character
        std::string line;
        std::getline(infile, line);

        // Read all bids and populate bids vector
        for (int v = 0; v < numBids; v++) {
            std::getline(infile, line);
            char split_char = ',';
            std::istringstream split(line);
            
            Bid newBid = {};

            // Read bid value
            int readValue;
            split >> readValue;
            newBid.value = readValue;

            // Read bid items
            std::vector<int> readBidItems;
            for (std::string each; 
                std::getline(split, each, split_char); 
                readBidItems.push_back(std::stoi(each)));
            std::sort(readBidItems.begin(), readBidItems.end());
            newBid.bidItems = readBidItems;

            std::cout << "Value: " << readValue << std::endl;
            for (int i = 0; i < readBidItems.size(); i++)
                std::cout << "Item: " << readBidItems[i] << std::endl;
            std::cout << std::endl;
        }

        infile.close();
        return 0;
    }

    return 1;
}

/*
 * Build the auction's conflict graph,
 * given that the bids vector is populated
 */
void buildConflictGraph() {

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

        outfile.close();
        return 0;
    }

    return 1;
}
