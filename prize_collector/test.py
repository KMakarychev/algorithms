import yaml
import numpy as np
from find_best_path import MaxPrizeSolver

DATA_FILE = "data/problem_set.in"

def call_solver(prizes):        
    total = len(prizes)
    n = int(np.sqrt(total))

    if n * n != total:
        raise ValueError(f"Prize count {total} is not a perfect square")

    prize_board = np.array(prizes, dtype=int).reshape((n, n))
    solver = MaxPrizeSolver(prize_board)
    return solver.solve()


def run_tests(yaml_file):

    print("Loading data...")
    with open(yaml_file, "r") as f:
        data = yaml.safe_load(f)

    total_problems = data["problems"]
    problems = data["data"]

    print("Running tests...")
    num_solved_correctly = 0
    error_list = []
    mistakes = []

    for entry in problems:
        problem_id = entry["problem"]
        expected = entry["correct_answer"]
        prizes = entry["prizes"]

        try:
            answer = call_solver(prizes)
        except Exception as e:
            error_list.append((problem_id, expected, str(e)))
            continue

        if answer == expected:
            num_solved_correctly += 1
        else:
            mistakes.append((problem_id, expected, answer))

    # ---- Report ----
    print(f"\nTest Results")
    print(f"=======================")
    print(f"Total problems:   {total_problems}")
    print(f"Solved correctly: {num_solved_correctly}")
    print(f"Wrong answers:    {len(mistakes)}")

    if error_list:
        print("\nErrors:")
        for problem_id, expected, error in sorted(error_list):
            print(
                f"  Problem {problem_id}: ERROR ({error})"
            )

    if mistakes:
        print("\nMistakes:")
        for problem_id, expected, answer in sorted(mistakes):
            print(
                f"  Problem {problem_id}: "
                f"expected={expected}, got={answer}"
            )


if __name__ == "__main__":
    run_tests(DATA_FILE)
