#pragma once
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "pstreams-1.0.3/pstream.h"

#include "gurobi_c++.h"

struct Bid {
    std::vector<int> bidGoods;
    int bidId;
    int value;
};

struct Edge {
    int v1;
    int v2;
};

int                     numGoods;
int                     numBids;
int                     numPruned0;
int                     numPruned1;
long long               totalValue;
long long               excludedBidsValues = 0;
long long               includedBidsValues = 0;
std::vector<Bid>        bids;
std::vector<Bid>        bidsExcludedFromMwvcByKernalization;
std::vector<Edge>       edges;
std::unordered_map<int, std::vector<int>>   bidsContainingGood;

// General solver functions
void resetState();
bool intersects();
std::string convertToDzn(std::string fileName);
int readAuctionMwvc(std::string auctionFileName);
int readCatsAuctionMwvc(std::string auctionFileName);
int readCatsAuctionSetPacking(std::string auctionFileName);
void buildConflictGraph();
int writeGraphToMwvcFile();
int outputOptimalAuction(std::string mwvcOutLine1, std::string mwvcOutLine2);
long long gurobiMwvcSolve();
long long gurobiSetPackingSolve();

// Kernalization logic
int kernalize();
void refactorConflictGraph(std::vector<Bid>& remainingBids);

/*
 * Resets auction state
 */
void resetState() {
    numGoods = 0;
    numBids = 0;
    numPruned0 = 0;
    numPruned1 = 0;
    totalValue = 0;
    excludedBidsValues = 0;
    includedBidsValues = 0;

    bids.clear();
    bidsExcludedFromMwvcByKernalization.clear();
    edges.clear();
    bidsContainingGood.clear();
}

/*
 * Returns whether two bids share a good
 * assuming that the bidGoods vectors are sorted
 */
int intersects(Bid& bid1, Bid& bid2) {
    auto b1Iter = bid1.bidGoods.begin();
    auto b2Iter = bid2.bidGoods.begin();

    // Loop: if shared good is found, return true
    while (b1Iter != bid1.bidGoods.end() && 
        b2Iter != bid2.bidGoods.end()) {
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
                std::string bidGood;

                // Read bid line
                std::getline(infile, line);
                std::istringstream bidSplit(line);

                // Skip row number, then read bid value and first good
                bidSplit >> bidValue;
                bidSplit >> bidValue;
                bidSplit >> bidGood;

                // Read bid's goods
                while (bidGood != "#") {
                    bid.push_back(std::stoi(bidGood) + 1);
                    bidSplit >> bidGood;
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
int readAuctionMwvc(std::string auctionFileName) {
    // Create input stream for auction file
    std::ifstream infile(auctionFileName);

    if (infile.is_open()) {
        // Read in the number of goods and bids in the auction
        infile >> numGoods >> numBids;

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
            totalValue += static_cast<long long>(readValue);

            // Read bid's goods
            std::vector<int> readbidGoods;
            for (std::string each; 
                std::getline(split, each, delim); 
                readbidGoods.push_back(std::stoi(each)));
            std::sort(readbidGoods.begin(), readbidGoods.end());
            newBid.bidGoods = readbidGoods;

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
int readCatsAuctionMwvc(std::string auctionFileName) {
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
        numGoods = goods + dummy;

        // Consume empty line
        std::getline(infile, line);

        // Read in and populate bids and bidValues vector
        for (int bidNum = 1; bidNum <= numBids; bidNum++) {
            // Initialize bid and fields
            Bid newBid = {};
            std::vector<int> readbidGoods;
            int bidValue;
            std::string bidGood;
            newBid.bidId = bidNum;

            // Read bid line
            std::getline(infile, line);
            std::istringstream bidSplit(line);

            // Skip row number, then read bid value and first good
            bidSplit >> bidValue;
            bidSplit >> bidValue;
            bidSplit >> bidGood;

            // Set bid value
            newBid.value = bidValue;
            totalValue += static_cast<long long>(bidValue);

            // Read bid's goods
            while (bidGood != "#") {
                readbidGoods.push_back(std::stoi(bidGood) + 1);
                bidSplit >> bidGood;
            }

            std::sort(readbidGoods.begin(), readbidGoods.end());
            newBid.bidGoods = readbidGoods;
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
int readCatsAuctionSetPacking(std::string auctionFileName) {
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
        numGoods = goods + dummy;

        // Consume empty line
        std::getline(infile, line);

        // Read in and populate bids (do not need to set bidGoods)
        for (int bidNum = 1; bidNum <= numBids; bidNum++) {
            // Initialize bid and fields
            Bid newBid = {};
            int bidValue;
            std::string bidGood;
            newBid.bidId = bidNum;

            // Read bid line
            std::getline(infile, line);
            std::istringstream bidSplit(line);

            // Skip row number, then read bid value and first good
            bidSplit >> bidValue;
            bidSplit >> bidValue;
            bidSplit >> bidGood;

            // Set bid value
            newBid.value = bidValue;
            bids.push_back(newBid);

            // Read bid's goods
            while (bidGood != "#") {
                int good = std::stoi(bidGood) + 1;
                auto findGood = bidsContainingGood.find(good);

                // Track bid that contains the given good
                if (findGood != bidsContainingGood.end()) {
                    (findGood -> second).push_back(bidNum);
                } else {
                    std::vector<int> bidsContainingThisGood{bidNum};
                    bidsContainingGood.insert(std::make_pair(good, bidsContainingThisGood));
                }

                bidSplit >> bidGood;
            }
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

            // Add edge if shared goods are found
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
        for (Bid& bid : bids) {
            outfile << "v " << bid.bidId << " " << bid.value << std::endl;
        }

        // Write edges to file
        for (Edge& edge : edges) {
            outfile << "e " << edge.v1 << " " 
                << edge.v2 << std::endl;
        }

        outfile.close();
        return 0;
    }

    return 1;
}

int outputOptimalAuction(std::string mwvcOutLine1, std::string mwvcOutLine2) {
    try {
        // Create input stream for auction file
        std::ofstream outfile("auction_results.txt");

        if (outfile.is_open()) {
            long long maxValue;

            // Read MWVC value from line, if not empty
            if (!mwvcOutLine1.empty()) {
                // Read in MWVC value
                int minValueStart = mwvcOutLine1.find_first_of(", ");
                int minValueEnd = mwvcOutLine1.find_first_of(", ", minValueStart + 2);
                int mwvcValue = std::stoll(
                    mwvcOutLine1.substr(minValueStart + 2, minValueEnd - minValueStart - 2));
                maxValue = totalValue - mwvcValue + excludedBidsValues;
            }

            // Take max value from kernalization step
            else {
                maxValue = excludedBidsValues;
            }

            // Write and print MWVC file header
            outfile << maxValue << " " << std::endl;
            std::cout << "MAX AUCTION VALUE: " << maxValue << std::endl;
            // std::cout << "Bids:" << std::endl;

            // Fields for validating winning auction
            int totalBidsValue = 0;

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
                for (Bid& bid : bids) {
                    // Skip over bids in MWVC bids
                    if ((mwvcBidsIndex < mwvcBids.size() && bid.bidId == mwvcBids[mwvcBidsIndex])) {
                        mwvcBidsIndex++;
                    }

                    // Print bids NOT in MWVC bids
                    else {
                        outfile << bid.value << "\t";
                        // std::cout << bid.value << "\t";
                        std::string separator = "";

                        // Validate winning auction value
                        totalBidsValue += bid.value;

                        for (int bidGood : bid.bidGoods) {
                            outfile << separator << bidGood;
                            // std::cout << separator << bidGood;
                            separator = ",";
                        }

                        outfile << std::endl;
                        // std::cout << std::endl;
                    }
                }
            }

            // Print bids confirmed by the kernalization step
            for (Bid& bid : bidsExcludedFromMwvcByKernalization) {
                outfile << bid.value << "\t";
                // std::cout << bid.value << "\t";

                std::string separator = "";

                // Validate winning auction value
                totalBidsValue += bid.value;

                for (int bidGood : bid.bidGoods) {
                    outfile << separator << bidGood;
                    // std::cout << separator << bidGood;
                    separator = ",";
                }

                outfile << std::endl;
                // std::cout << std::endl;
            }

            // Output validation results
            std::cout << "Validated Winning Auction Value: " << totalBidsValue << std::endl;
            // std::cout << "TotalValue: " << totalValue << std::endl;
            // std::cout << "FastWVC Line 1: " << mwvcOutLine1 << std::endl;
            // std::cout << "FastWVC Line 2: " << mwvcOutLine2 << std::endl;
            // std::cout << "Num bids excluded via kernalization: " << bidsExcludedFromMwvcByKernalization.size() << std::endl;      

            outfile.close();
            return 0;
        }
    } catch (...) {
        return 1;
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
        // Create new environment and suppress output
        GRBEnv* env = new GRBEnv();
        env -> set(GRB_IntParam_OutputFlag, 0);

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
        for (Edge& edge : edges) {
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

            // Bids confirmed to be excluded from the MWVC
            if (assignedValue == 0.0) {
                bidsExcludedFromMwvcByKernalization.push_back(bids[i]);
                excludedBidsValues += bids[i].value;
                numPruned0++;
            }

            else if (assignedValue == 1.0) {
                includedBidsValues += bids[i].value;
                numPruned1++;
            }

            // Ambiguous bids that still require search
            else {
                remainingBids.push_back(bids[i]);
            }
        }

        // Bids pruned
        if (bids.size() > remainingBids.size()) {
            // Reconfigure bids and edges in conflict graph
            totalValue = totalValue - excludedBidsValues - includedBidsValues;
            refactorConflictGraph(remainingBids);
        }
        
        // Output number of pruned bids
        std::cout << "Num pruned = 0: " << numPruned0 << std::endl;
        std::cout << "Num pruned = 1: " << numPruned1 << std::endl;

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
void refactorConflictGraph(std::vector<Bid>& remainingBids) {
    // Reassign IDs in remainingBids
    bids = remainingBids;
    numBids = bids.size();

    std::unordered_map<int, int> reassignedIds;
    for (int bidIndex = 0; bidIndex < numBids; bidIndex++) {
        if (bids[bidIndex].bidId != bidIndex + 1) {
            reassignedIds.insert(std::make_pair(bids[bidIndex].bidId, bidIndex + 1));
            bids[bidIndex].bidId = bidIndex + 1;
        }
    }

    // Refactor edges
    edges.clear();
    for (int bidIndex1 = 0; bidIndex1 < (numBids - 1); bidIndex1++) {
        for (int bidIndex2 = bidIndex1 + 1; 
                bidIndex2 < numBids; bidIndex2++) {
            Bid& bid1 = bids[bidIndex1];
            Bid& bid2 = bids[bidIndex2];

            // Add edge if shared goods are found
            if (intersects(bid1, bid2)) {
                Edge newEdge = {bid1.bidId, bid2.bidId};
                edges.push_back(newEdge);
            }
        }
    }
}

/*
 * Formulate combinatorial auction as min weighted set problem
 * Solve by invoking Gurobi
 */
long long gurobiMwvcSolve() {
    try {
        // Create new environment and suppress output
        GRBEnv* env = new GRBEnv();
        env -> set(GRB_IntParam_OutputFlag, 0);

        // Create new model
        GRBModel model = GRBModel(*env);
        model.set(GRB_StringAttr_ModelName, "gurobi_mwvc_solver");

        std::vector<GRBVar> bidVars;

        // Create a decision variable for each bid
        for (int i = 0; i < numBids; i++) {
            GRBVar newVar = model.addVar(0.0, 1.0, bids[i].value, GRB_BINARY, "");
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
        for (Edge& edge : edges) {
            model.addConstr(bidVars[edge.v1 - 1] + bidVars[edge.v2 - 1] >= 1.0f, "");
        }

        // Solve
        model.optimize();
        return totalValue + excludedBidsValues - static_cast<long long>(model.get(GRB_DoubleAttr_ObjVal));
    } catch (GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    } catch (...) {
        std::cout << "Exception during optimization" << std::endl;
    }

    return -1;
}

/*
 * Formulate combinatorial auction as min weighted set problem
 * Solve by invoking Gurobi
 */
long long gurobiSetPackingSolve() {
    try {
        // Create new environment and suppress output
        GRBEnv* env = new GRBEnv();
        env -> set(GRB_IntParam_OutputFlag, 0);

        // Create new model
        GRBModel model = GRBModel(*env);
        model.set(GRB_StringAttr_ModelName, "gurobi_set_packing_solver");

        std::vector<GRBVar> bidVars;

        // Create a decision variable for each bid
        for (int i = 0; i < numBids; i++) {
            GRBVar newVar = model.addVar(0.0, 1.0, bids[i].value, GRB_BINARY, "");
            bidVars.push_back(newVar);
        }
            
        // Create objective function
        GRBLinExpr* objFunction = new GRBLinExpr();
        for (int i = 0; i < numBids; i++) {
            GRBLinExpr* term = new GRBLinExpr(bidVars[i], bids[i].value);
            *objFunction += *term;
        }

        // Set objective function to minimize
        model.setObjective(*objFunction, GRB_MAXIMIZE);

        // Add constraints
        for (auto& goodAndBids : bidsContainingGood) {
            // If a good has multiple bids, apply a constraint
            if ((goodAndBids.second).size() > 1) {
                // 
                GRBLinExpr* constraint = new GRBLinExpr();

                for (int bidId : goodAndBids.second) {
                    *constraint += *(new GRBLinExpr(bidVars[bidId - 1]));
                }

                model.addConstr(*constraint <= 1.0f, "");
            }
        }

        // Solve
        model.optimize();
        return static_cast<long long>(model.get(GRB_DoubleAttr_ObjVal));
    } catch (GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    } catch (...) {
        std::cout << "Exception during optimization" << std::endl;
    }

    return -1;
}
