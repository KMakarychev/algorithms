#ifndef _concise_h_
#define _concise_h_
#include <algorithm>

namespace alg{
//////////////

// Several convinient functions for sorting data.
// Staring with C++20, you can use std::ranges::sort
// instead of these functions.

//sort data in increasing order
template<class T>
void sort(T& data)
{
   std::sort(data.begin(), data.end());
}

//sort data using **cmp** compare function.
template<class T, class Compare>
void sort(T& data, Compare cmp)
{
   std::sort(data.begin(), data.end(), cmp);
}

// Helper class **order_by** for sorting data by a field.
// Examples: 
//   sort (jobs, order_by(&Job::finish));
//   sort (jobs, order_by(&Job::start).ascending());
//   sort (jobs, order_by(&Job::start).descending());
// This class can also be used along with std::sort;
//
// There is a small run-time overhead associated with using this class.
template<typename T>
class order_by
{
public:   
   order_by(T field): field_(field), bAscending_(true){}

   order_by<T>& ascending()
   {
      bAscending_ = true;
      return *this;
   }

   order_by<T>& descending()
   {
      bAscending_ = false;
      return *this;
   }

   template<class A, class B>
   bool operator() (A a, B b)
   {
      return (bAscending_) ? (a.*field_ < b.*field_) :  (a.*field_ > b.*field_);
   }
private:
   T field_;
   bool bAscending_;
};

// Function **sort_by** sorts data by field.
// Examples: 
//   // sort intervals by their start time
//   alg::sort_by<&Job::start>(intervals);
// There is almost no run-time overhead associated with using this function.

template<auto Field, class T>
void sort_by(T& data, bool bAscending = true)
{  
   if (bAscending)
   {
      alg::sort(data, [](auto a, auto b){return (a.*Field < b.*Field);});
   }
   else
   {
      alg::sort(data, [](auto a, auto b){return (a.*Field > b.*Field);});
   }
}

template<auto Field, class Iter>
void sort_by(Iter begin, Iter end, bool bAscending = true)
{  
   if (bAscending)
   {
      std::sort(begin, end, [](auto a, auto b){return (a.*Field < b.*Field);});
   }
   else
   {
      std::sort(begin, end, [](auto a, auto b){return (a.*Field > b.*Field);});
   }
}

// Helper class for sorting data by an expression.
// The parameter **expr** may be a lambda function.
// Examples: 
//   # sort by finish time
//   sort (jobs, order_by_expr([](Job j){return j.finish;}));
//   # sort by processing time
//   sort (jobs, order_by_expr([](Job j){return j.finish - j.start;}));
// This class can also be used along with std::sort;

template<typename T>
class order_by_expr
{
public:   
   order_by_expr(T expr): expr_(expr), bAscending_(true){}

   order_by_expr<T>& ascending()
   {
      bAscending_ = true;
      return *this;
   }

   order_by_expr<T>& descending()
   {
      bAscending_ = false;
      return *this;
   }

   template<class A, class B>
   bool operator() (A a, B b)
   {
      return (bAscending_) ? (expr_(a) < expr_(b)) :  (expr_(a) > expr_(b));
   }
private:
   T expr_;
   bool bAscending_;
};

// Function **create_table** is used for creating 
// multidimensional tables/arrays with default 
// value **value**. This function is helpful, when you 
// create a table for dynamic programming algorithms.
//
// Remark: the type of entries is determined by the type
// of the initial values. Make sure that you use the correct
// type.
//
// Example:
//   // create a 5x10 matrix with integer entries -1
//   auto matrix = create_matrix(5, 10, -1);
template<typename T>
auto create_table(int size, T value)
{   
   std::vector<T> table(size, value);
   return table;
}

template<typename... Targs>
auto create_table(int size, Targs... args)
{ 
   auto slice = create_table(args...);
   std::vector<decltype(slice)> table(size, slice);
   return table;
}

//end of the namespace alg
}
#endif //_concise_h_