#pragma once
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_set>
#include <vector>
#include "pstreams-1.0.3/pstream.h"

#include "gurobi_c++.h"

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
int                 totalValue;
int                 kernalizationMaxAuctionValue = 0;
std::vector<Bid>    bids;
std::vector<Bid>    bidsExcludedFromMwvcByKernalization;
std::vector<Edge>   edges;

// General solver functions
void resetState();
bool intersects();
std::string convertToDzn(std::string fileName);
int readAuction(std::string auctionFileName);
int readCatsAuction(std::string auctionFileName);
void buildConflictGraph();
int writeGraphToMwvcFile();
int outputOptimalAuction(std::string mwvcOutLine1, std::string mwvcOutLine2);

// Kernalization logic
int kernalize();
void refactorConflictGraphEdges();

/*
 * Resets auction state
 */
void resetState() {
    numItems = 0;
    numBids = 0;
    totalValue = 0;
    kernalizationMaxAuctionValue = 0;

    bids.clear();
    bidsExcludedFromMwvcByKernalization.clear();
    edges.clear();
}

/*
 * Returns whether two bids share a bid item
 * assuming that the bidItems vectors are sorted
 */
int intersects(Bid& bid1, Bid& bid2) {
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
 * Converts CATS-generated file into dzn format
 */
std::string convertToDzn(std::string fileName) {
    // Define file name of dzn
    std::string nameWithoutExtension = fileName.substr(0, fileName.find_last_of("."));
    std::string newName = nameWithoutExtension + ".dzn";

    // Create output stream for dzn file
    std::ofstream outfile(newName);

    if (outfile.is_open()) {
        // Create input stream for auction file
        std::ifstream infile(fileName);
        if (infile.is_open()) {
            int goods;
            int dummy;
            int nBids;

            // Read line by line
            std::string line;
            while (std::getline(infile, line)) {
                std::istringstream split(line);

                std::string readValue;
                split >> readValue;

                if (readValue == "goods") {
                    split >> goods;
                } else if (readValue == "bids") {
                    split >> nBids;
                } else if (readValue == "dummy") {
                    split >> dummy;
                    break;
                }
            }

            // Sum total number of goods
            int nGoods = goods + dummy;

            // Consume empty line
            std::getline(infile, line);

            // Vectors to store bids and bid values
            std::vector<std::vector<int>> bids;
            std::vector<int> bidValues;

            // Read in and populate bids and bidValues vector
            for (int i = 0; i < nBids; i++) {
                std::vector<int> bid;
                int bidValue;
                std::string bidItem;

                // Read bid line
                std::getline(infile, line);
                std::istringstream bidSplit(line);

                // Skip row number, then read bid value and first item
                bidSplit >> bidValue;
                bidSplit >> bidValue;
                bidSplit >> bidItem;

                // Read bid items
                while (bidItem != "#") {
                    bid.push_back(std::stoi(bidItem) + 1);
                    bidSplit >> bidItem;
                }

                bids.push_back(bid);
                bidValues.push_back(bidValue);
            }

            infile.close();

            // Write dzn file header
            outfile << "nitems = " << nGoods << ";" << std::endl << std::endl;
            outfile << "nbids = " << nBids << ";" << std::endl;

            // Write bids
            outfile << "bid = [ ";
            for (int i = 0; i < bids.size(); i++) {
                // Print newline every 10 ints
                if ((i % 10 == 0) && (i > 0)) {
                    outfile << std::endl;
                }

                // Print bid items
                outfile << "{";
                for (int j = 0; j < bids[i].size(); j++) {
                    // Print item number
                    outfile << bids[i][j];

                    // Separate values with commas
                    if (j != (bids[i].size() - 1)) {
                        outfile << ",";
                    }
                }
                outfile << "}";

                // Separate values with commas
                if (i != (bids.size() - 1)) {
                    outfile << ", ";
                }
            }

            // Write bid values
            outfile << "];\nbidvalue = [";
            for (int i = 0; i < bidValues.size(); i++) {
                // Print newline every 10 ints
                if ((i % 10 == 0) && (i > 0)) {
                    outfile << std::endl;
                }

                // Print bid value
                outfile << bidValues[i];

                // Separate values with commas
                if (i != (bidValues.size() - 1)) {
                    outfile << ", ";
                }
            }
            outfile << " ];";

            return newName;
        }
        outfile.close();  
    }
    return "";
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

            bids.push_back(newBid);
        }

        infile.close();
        return 0;
    }

    return 1;
}

/*
 * Initializes all bids in bids vector
 * given an input CATS-generated auction file
 */
int readCatsAuction(std::string auctionFileName) {
    // Create input stream for auction file
    std::ifstream infile(auctionFileName);

    // Read from CATS file
    if (infile.is_open()) {
        int goods;
        int dummy;

        // Read line by line
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream split(line);

            std::string readValue;
            split >> readValue;

            if (readValue == "goods") {
                split >> goods;
            } else if (readValue == "bids") {
                split >> numBids;
            } else if (readValue == "dummy") {
                split >> dummy;
                break;
            }
        }

        // Sum total number of goods
        numItems = goods + dummy;

        // Consume empty line
        std::getline(infile, line);

        int bidNum = 1;

        // Read in and populate bids and bidValues vector
        for (int i = 0; i < numBids; i++) {
            // Initialize bid and fields
            Bid newBid = {};
            std::vector<int> bid;
            std::vector<int> readBidItems;
            int bidValue;
            std::string bidItem;
            newBid.bidId = bidNum;
            bidNum++;

            // Read bid line
            std::getline(infile, line);
            std::istringstream bidSplit(line);

            // Skip row number, then read bid value and first item
            bidSplit >> bidValue;
            bidSplit >> bidValue;
            bidSplit >> bidItem;

            // Set bid value
            newBid.value = bidValue;
            totalValue += bidValue;

            // Read bid items
            while (bidItem != "#") {
                readBidItems.push_back(std::stoi(bidItem) + 1);
                bidSplit >> bidItem;
            }

            std::sort(readBidItems.begin(), readBidItems.end());
            newBid.bidItems = readBidItems;
            bids.push_back(newBid);
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
            Bid& bid1 = bids[bidIndex1];
            Bid& bid2 = bids[bidIndex2];

            // Add edge if shared bid items are found
            if (intersects(bid1, bid2)) {
                Edge newEdge = {bid1.bidId, bid2.bidId};
                edges.push_back(newEdge);
            }
        }
    }
}

/*
 * Write the auction's conflict graph to a MWVC file,
 * using the desired format
 */
int writeGraphToMwvcFile() {
    // Create output stream for auction file
    std::ofstream outfile("auction.mwvc");

    if (outfile.is_open()) {
        // Write MWVC file header
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
        int maxValue;

        // Read MWVC value from line, if not empty
        if (!mwvcOutLine1.empty()) {
            // Read in MWVC value
            int minValueStart = mwvcOutLine1.find_first_of(", ");
            int minValueEnd = mwvcOutLine1.find_first_of(", ", minValueStart + 2);
            int mwvcValue = std::stoi(
                mwvcOutLine1.substr(minValueStart + 2, minValueEnd - minValueStart - 2));
            maxValue = totalValue - mwvcValue;
        }

        // Take max value from kernalization step
        else {
            maxValue = kernalizationMaxAuctionValue;
        }

        // Write and print MWVC file header
        outfile << maxValue << " " << std::endl;
        std::cout << "Max auction value: " << maxValue << std::endl;
        std::cout << "Bids:" << std::endl;

        // Read bids, if line not empty
        if (!mwvcOutLine2.empty()) {
            // Read bids
            std::istringstream split(mwvcOutLine2);
            char delim = ',';
            std::vector<int> mwvcBids;
            for (std::string each; 
                std::getline(split, each, delim); 
                mwvcBids.push_back(std::stoi(each)));

            // Write and print bids in optimal auction
            int mwvcBidsIndex = 0;
            for (Bid bid : bids) {
                // Skip over bids in MWVC bids
                if (mwvcBidsIndex < mwvcBids.size() && bid.bidId == mwvcBids[mwvcBidsIndex]) {
                    mwvcBidsIndex++;
                }

                // Print bids NOT in MWVC bids
                else {
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
        }

        // Print bids confirmed by the kernalization step
        for (Bid bid : bidsExcludedFromMwvcByKernalization) {
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

        outfile.close();
        return 0;
    }

    return 1;
}

/*
 * Kernalize conflict graph via a MWVC
 * (with LP relaxation to leverage the half-integrality property)
 * Return the number of bids remaining to search through
 * (to determine whether fastwvc is necessary)
 */
int kernalize() {
    try {
        // Create new environment
        GRBEnv* env = new GRBEnv();

        // Create new model
        GRBModel model = GRBModel(*env);
        model.set(GRB_StringAttr_ModelName, "kernelizer");

        std::vector<GRBVar> bidVars;

        // Create a decision variable for each bid
        for (int i = 0; i < numBids; i++) {
            GRBVar newVar = model.addVar(0.0, 1.0, bids[i].value, GRB_CONTINUOUS, "");
            bidVars.push_back(newVar);
        }

        // Create objective function
        GRBLinExpr* objFunction = new GRBLinExpr();
        for (int i = 0; i < numBids; i++) {
            GRBLinExpr* term = new GRBLinExpr(bidVars[i], bids[i].value);
            *objFunction += *term;
        }

        // Set objective function to minimize
        model.setObjective(*objFunction, GRB_MINIMIZE);

        // Add edge constraints
        for (Edge edge : edges) {
            model.addConstr(bidVars[edge.v1 - 1] + bidVars[edge.v2 - 1] >= 1.0f, "");
        }

        // Solve
        model.optimize();

        // New bid vector after pruning bids confirmed to be included/excluded from MWVC
        std::vector<Bid> remainingBids;

        // Reconfigure bid vectors based on kernalization results
        for (int i = 0; i < numBids; i++) {
            // By the half-integrality property, assignedValue must be in {0, 0.5, 1}
            double assignedValue = bidVars[i].get(GRB_DoubleAttr_X);

            // Bids confirmed to be excluded the MWVC
            if (assignedValue == 0.0) {
                bidsExcludedFromMwvcByKernalization.push_back(bids[i]);
                kernalizationMaxAuctionValue += bids[i].value;
            }

            // Ambiguous bids that still require search
            else if (assignedValue == 0.5) {
                remainingBids.push_back(bids[i]);
            }
        }

        // Set the bid vector to the remaining bids still require search
        bids = remainingBids;

        // Reconfigure edges in conflict graph
        refactorConflictGraphEdges();

        // Return the number of bids remaining to search through
        return bids.size();
    } catch (GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    } catch (...) {
        std::cout << "Exception during optimization" << std::endl;
    }

    return -1;
}

/*
 * Refactor the conflict graph,
 * Removing edges containing bids "pruned" by the kernalization
 */
void refactorConflictGraphEdges() {
    // New edges vector after removing edges contained pruned bids
    std::vector<Edge> remainingEdges;

    // Bid IDs of remaining bids
    std::unordered_set<int> remainingBidIds;

    // Populate remainingBidIds hash set
    for (Bid bid : bids) {
        remainingBidIds.insert(bid.bidId);
    }

    // Filter out all edges containing pruned bids
    for (Edge edge : edges) {
        // Add edge if and only if it connects two remaining (non-pruned) bids
        if (remainingBidIds.find(edge.v1) != remainingBidIds.end() &&
            remainingBidIds.find(edge.v2) != remainingBidIds.end()) {
            remainingEdges.push_back(edge);
        }
    }

    // Set the edge vector to the remaining edges
    edges = remainingEdges;
}
