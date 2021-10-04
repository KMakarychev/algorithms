///////////////////////////////////////////////////////////////////////////////
// Greedy algorithm for scheduling jobs on one maching ("interval scheduling")
// (c) Konstantin Makarychev
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <vector> 

#include "../common/concise.h"

// struct for storing intervals
struct Job
{
   int start = 0;
   int finish = 0;
};

// FindMaxSchedule finds the maximum number of jobs from
// a collection **jobs** that can be scheduled on one machine.
int FindMaxSchedule (std::vector<Job> jobs)
{  
   //sort jobs by finish time
   //we use functions defined in "concise.h"
   alg::sort_by<&Job::finish>(jobs);
   int count = 0;
   int previousFinishTime = 0;

   //for every job j, check if we can schedule it;
   //if we cannot discard it
   for (auto j : jobs)
   {
      //check that the job does not intersect with the previous one
      if (j.start >= previousFinishTime)
      {
         count++;
         previousFinishTime = j.finish;
      }
   }

   return count;
}