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
int                 totalValue;
std::vector<Bid>    bids;
std::vector<Edge>   edges;

bool intersects();
int readAuction(std::string auctionFileName);
void buildConflictGraph();
int writeGraphToMwvcFile();
int outputOptimalAuction(std::string mwvcOutLine1, std::string mwvcOutLine2);

/*
 * Returns whether two bids share a bid item
 * assuming that the bidItems vectors are sorted
 */
bool intersects(Bid& bid1, Bid& bid2) {
    auto b1Iter = bid1.bidItems.begin();
    auto b2Iter = bid2.bidItems.begin();

    // Loop: if shared bid item is found, return true
    while (b1Iter != bid1.bidItems.end() && 
        b2Iter != bid2.bidItems.end()) {
        if (*b1Iter < *b2Iter) {
            ++b1Iter;
        } else if (*b1Iter > *b2Iter) {
            ++b2Iter;
        } else {
            return true;
        }
    }

    return false;
}

/*
 * Initializes all bids in bids vector
 * given an input auction file
 */
int readAuction(std::string auctionFileName) {
    // Create input stream for auction file
    std::ifstream infile(auctionFileName);

    if (infile.is_open()) {
        // Read in the number of items and bids in the auction
        infile >> numItems >> numBids;

        // Consume new line character
        std::string line;
        std::getline(infile, line);

        // Read all bids and populate bids vector
        for (int bidNum = 1; bidNum <= numBids; bidNum++) {
            std::getline(infile, line);
            char delim = ',';
            std::istringstream split(line);
            
            Bid newBid = {};
            newBid.bidId = bidNum;

            // Read bid value
            int readValue;
            split >> readValue;
            newBid.value = readValue;
            totalValue += readValue;

            // Read bid items
            std::vector<int> readBidItems;
            for (std::string each; 
                std::getline(split, each, delim); 
                readBidItems.push_back(std::stoi(each)));
            std::sort(readBidItems.begin(), readBidItems.end());
            newBid.bidItems = readBidItems;

            bids.emplace_back(newBid);
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
    // Iterate over all pairs of bids
    for (int bidIndex1 = 0; bidIndex1 < (bids.size() - 1); bidIndex1++) {
        for (int bidIndex2 = bidIndex1 + 1; 
                bidIndex2 < bids.size(); bidIndex2++) {
            Bid& bid1 = bids.at(bidIndex1);
            Bid& bid2 = bids.at(bidIndex2);

            // Add edge if shared bid items are found
            if (intersects(bid1, bid2)) {
                Edge newEdge = {bid1.bidId, bid2.bidId};
                edges.emplace_back(newEdge);
            }
        }
    }
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
        outfile << "p edge " << numBids << " " 
            << edges.size() << std::endl;

        // Write vertices to file
        for (Bid bid : bids) {
            outfile << "v " << bid.bidId << " " 
                << bid.value << std::endl;
        }

        // Write edges to file
        for (Edge edge : edges) {
            outfile << "e " << edge.v1 << " " 
                << edge.v2 << std::endl;
        }

        outfile.close();
        return 0;
    }

    return 1;
}

int outputOptimalAuction(std::string mwvcOutLine1, std::string mwvcOutLine2) {
    // Create input stream for auction file
    std::ofstream outfile("auction_results.txt");

    if (outfile.is_open()) {
        // Read in mwvc value
        int valueStart = mwvcOutLine1.find_first_of(", ");
        int valueEnd = mwvcOutLine1.find_first_of(", ", valueStart + 2);
        int mwvcValue = std::stoi(
            mwvcOutLine1.substr(valueStart + 2, valueEnd - valueStart - 2));
        int maxValue = totalValue - mwvcValue;

        // Write and print mwvc file header
        outfile << maxValue << " " << std::endl;
        std::cout << "Max auction value: " << maxValue << std::endl;

        // Read bids in mwvc
        std::istringstream split(mwvcOutLine2);
        char delim = ',';
        std::vector<int> mwvcBids;
        for (std::string each; 
            std::getline(split, each, delim); 
            mwvcBids.push_back(std::stoi(each)));

        // Write and print bids in optimal auction
        std::cout << "Bids: " << std::endl;
        int mwvcBidsIndex = 0;
        for (Bid bid : bids) {
            if (bid.bidId == mwvcBids.at(mwvcBidsIndex)) {
                mwvcBidsIndex++;
            } else {
                outfile << bid.value << "\t";
                std::cout << bid.value << "\t";

                std::string separator = "";

                for (int bidItem : bid.bidItems) {
                    outfile << separator << bidItem;
                    std::cout << separator << bidItem;
                    separator = ","; 
                }

                outfile << std::endl;
                std::cout << std::endl;
            }
        }

        outfile.close();
        return 0;
    }

    return 1;
}
