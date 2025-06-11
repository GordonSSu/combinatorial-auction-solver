# combinatorial-auction-solver

Implementation and benchmakring of our approach proposed in *New Approaches for Winner Determination via Minimum Weighted Vertex Cover Computations* (2024 International Conference on AI: Theory and Applications)

### Paper Abstract
In this paper, we consider the winner determination problem on single-unit combinatorial auctions (CAs). While there already exist Integer Linear Programming formulations and other algorithms for solving them, our proposed approach casts them as Minimum Weighted Vertex Cover problems. This transformation allows us to leverage the Nemhauser-Trotter reduction for kernelization and utilize fast anytime algorithms such as FastWVC. Our approach yields significant speedups over other baseline methods with only a minor loss in solution quality on benchmark instances, paving the way for kernelization and anytime algorithms on more general CAs.
