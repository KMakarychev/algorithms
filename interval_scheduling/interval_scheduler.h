// struct for storing intervals
struct Job
{
   int start = 0;
   int finish = 0;
};

// FindMaxSchedule finds the maximum number of jobs from
// a collection **jobs** that can be scheduled on one machine.
int FindMaxSchedule (const std::vector<Job>& jobs);