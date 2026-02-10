<img src="https://github.com/KMakarychev/algorithms/blob/main/prize_collector/illustration.png" alt="Chessboard with prizes" width="800">

**Problem.**  
Consider an $n \times n$ chessboard in which each square with coordinates $(i,j)$ contains a non-negative prize of $a(i,j)$ dollars. A king starts on the bottom-left square $(1,1)$ and must reach the top-right square $(n,n)$. At each step, the king may move one square up, one square to the right, or one square diagonally up and to the right. The king collects the prize from every square it visits, including both the starting and ending squares. Design an algorithm that computes the maximum total prize the king can collect along any valid path from $(1,1)$ to $(n,n)$.

**Solution.**  
We solve this problem using Dynamic Programming. We use the following subproblem: $OPT(i,j)$ is the maximum total prize the king can collect if he starts in square $(i,j)$. As before, he collects prizes from all squares he visits and is allowed to move one square up, one square to the right, or one square diagonally up and to the right.

Before writing a recurrence relation, we make the following observation. Since all prizes are non-negative, any path in which the king makes a diagonal move can be improved by replacing the diagonal move $(k,k) \to (k+1,k+1)$ with a sequence of two moves $(k,k) \to (k,k+1) \to (k+1,k+1)$. The new path visits one additional square compared to the original path and therefore collects at least as much prize. Because visiting additional squares can never decrease the total prize collected, we may assume without loss of generality that the king always moves either one square to the right or one square up.

We now write the recurrence relation. First, consider the case when $i < n$ and $j < n$. Then, the king can move either right to square $(i,j+1)$ or up to square $(i+1,j)$. In both cases, he collects the prize at square $(i,j)$. If he moves to square $(i,j+1)$, then he can collect $OPT(i,j+1)$ dollars from that point on; if he moves to square $(i+1,j)$, then he can collect $OPT(i+1,j)$ dollars from that point on. The optimal strategy therefore chooses the better of these two options. Thus,
$$
OPT(i,j) = a(i,j) + \max\bigl(OPT(i,j+1),\, OPT(i+1,j)\bigr).
$$

The remaining cases correspond to boundary squares, where only one move is possible. If $i = n$ but $j < n$, then the king can only move right, and
$$
OPT(i,j) = a(i,j) + OPT(i,j+1).
$$
Similarly, if $i < n$ but $j = n$, then
$$
OPT(i,j) = a(i,j) + OPT(i+1,j).
$$
Finally, if $i = n$ and $j = n$, then
$$
OPT(i,j) = a(i,j),
$$
since this is the last square on the king’s path.

The answer to the original question is $OPT(1,1)$, which is the maximum total prize the king can collect by starting at square $(1,1)$.

**Implementation.**  
We compute the recurrence relation using a top-down approach. To this end, we create a DP table `opt` and evaluate its entries using the helper function `compute_opt(i,j,opt)`. 

The source code in Python and C++ is available here: 

[Python](https://github.com/KMakarychev/algorithms/blob/main/prize_collector/find_best_path.py) 

[C++](https://github.com/KMakarychev/algorithms/blob/main/prize_collector/prize_collector.cpp).

