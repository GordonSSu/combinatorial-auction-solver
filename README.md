# combinatorial-auction-solver
Solves combinatorial auctions by invoking the fastwvc solver:  
https://github.com/kjcm150/fastwvc

### Input format
First Line: _numItemsInAuction_,_numbBids_  
Next Lines (one line per bid): _bidAmount_ _commaSeparatedListOfBidItems_
E.g:
```
4,2
23 1
35 2,4
46 1,2,3
```

### Output format
First Line: _maxValue_
Next Lines (one line per bid): _bidAmount_ _commaSeparatedListOfBidItems_
E.g:
```
58
23 1
35 2,4
```

### Compiling and Running
Compiling:
```
g++ auction-solver.cpp --std=c++11 -o auction-solver
```
Running:
```
./auction-solver auction.txt
```
