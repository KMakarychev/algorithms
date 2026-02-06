///////////////////////////////////////////////////////////////////////////////
// Greedy algorithm for scheduling jobs on one machine ("interval scheduling")
// (c) Konstantin Makarychev
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <vector>

#include "../common/concise.h"
#include "interval_scheduler.h"

int FindMaxSchedule(const std::vector<Job>& jobs)
{
    std::vector<Job> sortedJobs = jobs;
    alg::sort_by<&Job::finish>(sortedJobs);

    int count = 0;
    int previousFinishTime = 0;

    for (const Job& j: sortedJobs)
    {
        if (j.start >= previousFinishTime)
        {
            ++count;
            previousFinishTime = j.finish;
        }
    }

    return count;
}
