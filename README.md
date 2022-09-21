# Computing Turán Density Coefficients for k-PDGs (k-uniform partially directed hypergraphs)

The core component of this project is a general-purpose computational library to enumerate k-uniform partially directed hypergraphs (or k-uniform hypergraphs) in order to compute their Turán density coefficients (or Turán numbers).
Also given here are a few examples of using this code to perform computational tasks where specific partially-directed hypergraphs are forbidden in large graphs. 

See the [Math, Data Structures and Algorithms](doc/math_alg.md) document for mathematical background, as well as exposition on the data structures and algorithms used in this code.
See the [Development](doc/develop.md) document for information related to the development environment setup, and how to contribute.

A significant result from this code is the verification of the the Bollobás-Brightwell-Leader k-SAT enumeration conjecture for the k=5 case; a result published in the appendix of [this paper](https://arxiv.org/abs/2107.09233) by Dong, Mani, and Zhao. An update—the full conjecture has been proven by Balogh et al.; see [here](https://arxiv.org/abs/2209.04894).
