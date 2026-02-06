// This example is based on this story: 
//    https://nee.lv/2021/02/28/How-I-cut-GTA-Online-loading-times-by-70/
//
// TL;DR: 
//   1. Do not recompute the length of the same string over an over again 
//      in a loop. In general, try to save results that you are planning 
//      to reuse multiple times (use _memoization_).
//   2. Know the time complexity (running time) of operations that you use
//      in your code.
//   3. Use std::string instead of C-style char* when possible (unless you 
//      have a specific reason not to do so).

// To compile type:
//   clang++ replace_spaces.cpp -O2 -o replace_spaces.out
//   g++ replace_spaces.cpp -O2 -o replace_spaces.out
//
// Try running this code with a regular text file and with a 
// file that contains really large strings (say, 1 million characters).

#include <cstring>
#include <fstream>
#include <iostream>

const int bufferSize = 10000000;

// ReplaceSpaces - Replaces spaces with dashes.
// Find a serious bug in this function.
//
// DO NOT USE THIS FUNCTION IN YOUR CODE!
void ReplaceSpaces(char* str)
{
   for (int i = 0; i < std::strlen(str); i++)
   {
      if (std::isspace(str[i]))
      {
         str[i] = '-';
      }
   }
}

int main(int argc, char *argv[])
{
   if (argc != 3)
   {
      std::cerr << "Please, specify the input and output file names." << std::endl;
      return 1;
   }

   std::ifstream in(argv[1]);
   std::ofstream out(argv[2]);

   if (!in)
   {
      std::cerr << "Cannot open the input file."<< std::endl;
      return 1;
   } 

   char* str = new char[bufferSize];
   while (!in.eof())
   {      
      in.getline(str, bufferSize);
      ReplaceSpaces(str);
      out << str << std::endl;
   }
   delete[] str;

   in.close();
   out.close();
}