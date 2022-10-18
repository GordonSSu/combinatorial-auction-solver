% Combinatorial Auctions in MiniZinc

int: nitems;   % number of items
set of int: ITEM = 1..nitems;

int: nbids;    % number of bids
set of int: BID = 1..nbids;
array[BID] of set of ITEM: bid;  % the subset of each bid
array[BID] of int: bidvalue;     % the value of the bid

array[BID] of var bool: selected; % which bids are selected

% clauses to represent compatible bids
constraint forall(b1, b2 in BID where b1 < b2)
                 (if bid[b1] intersect bid[b2] != {} 
                  then not selected[b1] \/ not selected[b2] endif);

var int: obj;
constraint obj = sum(b in BID)(selected[b]*bidvalue[b]);                 
solve maximize obj;                                       