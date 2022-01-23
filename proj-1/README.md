# Project 1

The aim of the project is to use SIMD instructions to improve the
performance of matrix multiplication of large matrices.

## Thoughts

A matrix is a two dimensional array of numerical values. A matrix
has a number of rows and a number of columns. Matrix size is represented
by the form rows x cols. For example, the following is a 2x3 matrix

\[0\]\[1\]\[2\] \
\[2\]\[3\]\[4\]

Matrix multiplication is an operation that takes two matrices of size n x m
and m x k, and returns a matrix of size n x k. Each entry in the result n x k
matrix is the dot product of a row vector of the first matrix and a column vector
of the second matrix.

A dot product is merely a sum of a series of two factor products.

Therefore, the bulk of the computation boils down to n * k iterations of a sum of
m multiplications. Time complexity is thus O(n * k * m)


