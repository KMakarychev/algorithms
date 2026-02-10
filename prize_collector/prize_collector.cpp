///////////////////////////////////////////////////////////////////////////////
// DP algorithm for finding the best path for the king from the square (0,0)
// to the square (n-1,n-1)
// (c) Konstantin Makarychev
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cassert>
#include <vector>

#include "../common/concise.h"
#include "prize_collector.h"

int FindMaxPrizeCollectingPath(const std::vector<std::vector<int>>& prizeBoard)
{
    const int n = static_cast<int>(prizeBoard.size());

    if (n == 0)
    {
        return 0;
    }

    auto dpTable = alg::create_table(n, n, -1);

    // Fill DP table bottom-up from bottom-right
    for (int i = n - 1; i >= 0; --i)
    {
        assert(prizeBoard[i].size() == n);
        for (int j = n - 1; j >= 0; --j)
        {
            if (i == n - 1 && j == n - 1)
            {
                dpTable[i][j] = prizeBoard[i][j];
            }
            else if (j == n - 1)  // last column
            {
                dpTable[i][j] = prizeBoard[i][j] + dpTable[i + 1][j];
            }
            else if (i == n - 1)  // last row
            {
                dpTable[i][j] = prizeBoard[i][j] + dpTable[i][j + 1];
            }
            else
            {
                dpTable[i][j] = prizeBoard[i][j] +
                                std::max(dpTable[i + 1][j], dpTable[i][j + 1]);
            }
        }
    }

    return dpTable[0][0];
}
