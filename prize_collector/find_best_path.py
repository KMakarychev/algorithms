import numpy as np

class MaxPrizeSolver:
    def __init__(self, prize_board):
        """
        prize_board: an n x n NumPy array of non-negative integers
        """
        self._prize_board = prize_board
        self._n = prize_board.shape[0]
        self._dp = None

    def solve(self):
        """
        Computes the maximum total prize collectable from (0,0) to (n-1,n-1).
        """
        # Initialize DP table with -1 to indicate yet unknown entries
        self._dp = -np.ones((self._n, self._n), dtype=int)
        result = self._compute_opt(0, 0)
        return int(result)

    def _compute_opt(self, row, col):
        """
        Returns the value of OPT(row, col) using top-down DP with memoization.
        """
        # Return cached value if already computed
        if self._dp[row, col] != -1:
            return self._dp[row, col]
        
        if row + 1 < self._n and col + 1 < self._n:
            self._dp[row, col] = self._prize_board[row, col] + (
                    max(self._compute_opt(row + 1, col), self._compute_opt(row, col + 1)))
        elif row + 1 < self._n:
            self._dp[row, col] = self._prize_board[row, col] + self._compute_opt(row + 1, col)                  
        elif col + 1 < self._n:
            self._dp[row, col] = self._prize_board[row, col] + self._compute_opt(row, col + 1)       
        else:
            self._dp[row, col] = self._prize_board[row, col]

        return self._dp[row, col]

