///////////////////////////////////////////////////////////////////////////////
// Example of a dynamic programming algorithm
// Written by Konstantin Makarychev.

// An example of a dynamic programming algorithm for finding the maximum 
// independent set on a line. We are given a set of non-negative weights 
//             weight[0],..., weight[n-1]
// The goal is to find a subset of indices S that maximizes
//             sum_{i in S} weight[i]
// such that S is an independent set on a line. Namely, S must satisfy
// the following condition: if i is in S, then (i+1) is not in S.


// To compile, use one of the following commands:
//  1. g++ maximum_independent_set.cpp -o maximum_independent_set
//  2. clang++ maximum_independent_set.cpp -o maximum_independent_set
//  3. cl maximum_independent_set.cpp

// include standard libraries
#include <algorithm>
#include <cassert> 
#include <iostream>
#include <string>
#include <vector>


// The first function **FindIndependentSet_BottomUp** uses a bottom-up 
// approach.

int FindIndependentSet_BottomUp (const std::vector<int>& weights)
{  
   // Handle a special case when the array is empty.
   if (weights.empty()) return 0;

   // From now on, the size of the array is at least 1.
   // Get the number of elements in the array.
   size_t nSize = weights.size();
   assert(nSize > 0);

   // Allocate a DP table. 
   // optValues[i] is the weight of the maximum independent 
   // set for elements {0,...,i}.
   // Тhe size of the DP table in nSize.
   // Question: Can we use less memory in this particular case?
   std::vector<int> optValues (nSize, 0);

   // Initialize the array (the base case of DP).
   optValues[0] = weights[0];

   // Fill in the DP table starting with the smallest subproblem.
   if (nSize > 1)
   {
      optValues[1] = std::max(optValues[0], weights[1]);

      for (size_t i = 1; i < nSize - 1; ++i)
      {         
         optValues[i+1] = std::max(optValues[i], optValues[i - 1] + weights[i+1]);
      }
   }

   //return the last element in the DP table
   return optValues[nSize - 1];
}

// An example of a dynamic programming algorithm for finding 
// the maximum independent set. This algorithm uses a top-down 
// approach.

const int notValue = -1;

// A helper function: Find the maximum independent set for 
// the subarray [0..k] of the array weights. 
// * weights is the array of weights;
// * dpTable is the array of cached/memoized values;
// * k is the index of the last element in the subarray.

// Note:
//   1. We pass the main array "weights" by const reference.
//   2. We pass the dynamic table dpTable by reference.

int FindIndependentSetRecursively (const std::vector<int>& weights, 
                                   std::vector<int>& dpTable, 
                                   size_t k)
{
   // Make sure this function receives a valid input.
   assert (k < weights.size());
   assert (dpTable.size() == weights.size());

   // If we know the result, return it right away.
   if (dpTable[k] != notValue) return dpTable[k];

   int result = notValue;

   // Handle the base cases first.
   if (k == 0)
   {
      result = weights[0];
   } 
   else if (k == 1)
   {
      result = std::max(weights[0], weights[1]);
   }
   else 
   {
      //Main case, k >= 2.

      //Option A - we do not include k in the independent set;
      //Option B - we include k in the independent set.
      int optionA = FindIndependentSetRecursively(weights, dpTable, k - 1);
      int optionB = FindIndependentSetRecursively(weights, dpTable, k - 2) + weights[k];
      result = std::max (optionA, optionB);
   }
   
   //Store the result in the DP table.
   dpTable[k] = result;

   assert(result >= 0);

   return result;
}

int FindIndependentSet_TopDown (const std::vector<int>& weights)
{
   // Handle a special case when the array is empty.
   if (weights.empty()) return 0;

   // From now on, the size of the array is at least 1.
   // Get the number of elements in the array.
   size_t nSize = weights.size();

   assert(nSize > 0);

   // Allocate a DP table. Set a special value to all entries (notValue).
   // This value indicates that the table cells are not yet initialized. 
   std::vector<int> dpTable (nSize, notValue);

   return FindIndependentSetRecursively(weights, dpTable, nSize - 1);
}

///////////////////////////////////////////////////////////////////////////////
//The main function runs several tests and outputs the results.

int main (int argc, char *argv[])
{
   //some hard-coded examples.
   std::vector<int> example1; //empty

   std::vector<int> example2 = {11, 10};

   std::vector<int> example3 = {100, 120, 21};

   std::vector<int> example4 = {53,77,102,76,65,1,1,1};

   std::vector<int> example5 = {74,1,1,63,61,63,1,1,93,94,95,98,1,121,93,1,97,1,1,1,135,1,107,1,135,1,1,1,110,160,149,1,116,1,1,1,125,138,142,165,2,166,170,157,2,2,190,2,2,2,172,165,160,180,207,198,2,174,2,204,209,211,3,224,211,226,185,187,3,3,3,244,227,248,208,3,3,3,228,255,4,222,217,4,231,252,4,235,239,4,4,4,4,242,4,4,276,248,289,5,295,5,5,5,5,266,288,5,5,315,284,281,312,287,324,327,5,5,289,6,6,6,343,327,307,322,331,312,349,323,6,325,6,346,6,6,332,6,370,359,7,353,379,351,377,7,371,385,377,7,7,7,7,370,7,411,413,7,385,8,387,8,376,8,8,429,8,8,429,391,398,407,433,402,8,444,8,407,452,9,447,9,457,455,9,9,9,9,453,9,437,9,9,9,9,9,9,450,489,486,498,10,503,473,483,10,488,10,10,506,499,10,490,10,528,10,501,525,10,11,11,509,11,11,11,509,11,547,11,11,11,11,519,546,528,11,564,11,534,12,12,567,12,561,562,12,548,12,12,12,584,563,592,12,572,586,606,598,589,13,13,583,13,606,13,595,625,13,13,13,636,630,599,13,622,13,13,647,13,14,14,14,627,662,636,665,14,14,643,14,678,657,681,654,14,690,648,14,664,15,691,680,15,659,15,679,664,683,15,15,15,15,15,697,15,15,733,708,705,735,16,16,696,16,16,16,16,709,16,16,743,714,755,761,16,759,16,16,738,742,744,738,17,17,17,785,17,782,17,763,17,17,778,806,773,17,17,786,17,797,18,805,801,18,18,812,796,18,795,18,795,833,797,815,801,804,18,809,849,843,19,19,834,19,846,19,19,19,838,844,19,19,840,19,19,19,19,19,19,20};

   std::cout << "First example:" << std::endl;
   std::cout << "  Bottom-up approach: " << FindIndependentSet_BottomUp(example1) << std::endl;
   std::cout << "  Top-down approach: "  << FindIndependentSet_TopDown (example1) << std::endl;
   std::cout << std::endl;

   std::cout << "Second example:" << std::endl;
   std::cout << "  Bottom-up approach: " << FindIndependentSet_BottomUp(example2) << std::endl;
   std::cout << "  Top-down approach: "  << FindIndependentSet_TopDown (example2) << std::endl;
   std::cout << std::endl;

   std::cout << "Third example:" << std::endl;
   std::cout << "  Bottom-up approach: " << FindIndependentSet_BottomUp(example3) << std::endl;
   std::cout << "  Top-down approach: "  << FindIndependentSet_TopDown (example3) << std::endl;
   std::cout << std::endl;

   std::cout << "Fourth example:" << std::endl;
   std::cout << "  Bottom-up approach: " << FindIndependentSet_BottomUp(example4) << std::endl;
   std::cout << "  Top-down approach: "  << FindIndependentSet_TopDown (example4) << std::endl;
   std::cout << std::endl;

   std::cout << "Fifth example:" << std::endl;
   std::cout << "  Bottom-up approach: " << FindIndependentSet_BottomUp(example5) << std::endl;
   std::cout << "  Top-down approach: "  << FindIndependentSet_TopDown (example5) << std::endl;
   std::cout << std::endl;
   return 0;
}
