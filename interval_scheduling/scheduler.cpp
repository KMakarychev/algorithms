////////////////////////////////////////////////////////////////////////////
// DO NOT EDIT THIS FILE
//
// To compile with **clang++** or **g++** type:
//   clang++ -std=c++17 -pedantic -Wall scheduler.cpp -O3 -o scheduler.out
//   g++ -std=c++17 -pedantic -Wall scheduler.cpp -O3 -o scheduler.out


#include <string>
#include <vector>

#include "interval_scheduling.h"
#include "../common/test_framework.h"

const char* inputFilename = "data/intervals.in";

constexpr int kTestFrameworkVersion = 107;
constexpr int problem_set_id = 1005230;

struct IntervalSchedulingProblem : public TestFramework::BasicProblem
{
   std::vector<int> left;
   std::vector<int> right;
};

int FindMaxScheduleHelper (const std::vector<int>& left,
                           const std::vector<int>& right)
{
   TestFramework::ExitIfConditionFails (left.size() == right.size(), 
      "Invalid data. Arrays of the left and right endpoints have different sizes.");

   size_t size = left.size();
   std::vector<Job> jobs;
   jobs.reserve (size);

   for (size_t i = 0; i < size; i++)
   {
      TestFramework::ExitIfConditionFails (right[i] >= left[i], 
               "Left endpoint is greater that the right endpoint. "
               "Please, check the input file.");

      jobs.push_back ({left[i],right[i]});
   }
   
   return FindMaxSchedule (jobs);
}


int main(int argc, char *argv[])
{
   using namespace TestFramework;
   
   static_assert (GetTestFrameworkVersion () == kTestFrameworkVersion, 
                  "Incorrect version of Test Framework."
                  "Please, update test_framework.h.");

   ProblemSetHeader header;
   RecordAdapter<ProblemSetHeader> psAdapter(header); 
   AddDefaultProblemSetColumns(psAdapter);

   std::vector<IntervalSchedulingProblem> problems;
   TableAdapter<IntervalSchedulingProblem> prAdapter(problems);
   AddDefaultProblemColumns (prAdapter);
 
   AddColumn<IntervalSchedulingProblem>(prAdapter, "left", 
                              &IntervalSchedulingProblem::left);

   AddColumn<IntervalSchedulingProblem>(prAdapter, "right", 
                              &IntervalSchedulingProblem::right);
        
   BasicYamlParser parser(dynamic_cast<ITable*>(&psAdapter), 
                          dynamic_cast<ITable*>(&prAdapter));

   parser.ParseFile(inputFilename, true);

   PreprocessProblemSet(problem_set_id, problems, header);

   for (int i = 0; i < header.problem_count; i++)
   {
      IntervalSchedulingProblem& theProblem = problems[i];
      
      theProblem.student_answer = 
               FindMaxScheduleHelper (theProblem.left, theProblem.right);
   }

   std::cout << std::endl;
   ProcessResults(problems, header);
   std::cout << "Running time: " << header.time << "ms."
             << std::endl << std::endl;
}