#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

static long long backtracks = 0;
void *printBacktracks(void *arg)
{
	(void)arg;
	while (true) {
		sleep(2);
		printf("backtracks: %lld\n", backtracks);
	}
	return NULL;
}

bool checkClause(int const numVars, bool solution[numVars], int const clause[numVars])
{
	// if one variable is satisfied, return true
	for (int i = 0; i < numVars; i++) {
		int var = clause[i];
		if (var == 0) {
			break;
		}
		if (solution[abs(var) - 1] == (var > 0)) {
			return true;
		}
	}
	return false;
}

bool checkClauses(int const numClauses,
		int const numVars,
		bool solution[numVars],
		int const clauses[numClauses][numVars])
{
	// all clauses need to be satisfied to return true
	for (int i = 0; i < numClauses; i++) {
		if (!checkClause(numVars, solution, clauses[i])) {
			return false;
		}
	}
	return true;
}

// Recursive function which naively tries all possible truth assignments and tests them
// returns: true if satisfiable, solution contains the satisfying truth assignment
//          false if unsatisfiable
// the "index" argument indicates the depth of recursion, and also the index into the 
// solution that this call is responsible for flipping
bool solve(int const numClauses,
		int const numVars,
		bool solution[numVars],
		int const clauses[numClauses][numVars],
		int const index)
{
	if (index == numVars) {
		return checkClauses(numClauses, numVars, solution, clauses);
	}
	solution[index] = true;
	if (solve(numClauses, numVars, solution, clauses, index + 1)) {
		return true;
	}
	backtracks++;
	solution[index] = !solution[index];
	return solve(numClauses, numVars, solution, clauses, index + 1);
}

int main()
{
	int numVars;
	int numClauses;
	char junk[128];

	scanf("%s %s %d %d\n", junk, junk, &numVars, &numClauses);

	// Array of clauses. Each clause is an array of integers.
	int clauses[numClauses][numVars];
	// A potential solution is an array of bools, where the index is the "name" of the variable - 1
	bool solution[numVars];

	// Read all of the clauses
	for (int clause = 0; clause < numClauses; clause++) {
		int var;
		scanf("%d", &var);
		for (int i = 0; i < numVars; i++) {
			clauses[clause][i] = var;
			if (var == 0) {
				break;
			}
			scanf("%d", &var);
		}
	}

	pthread_t tid;
	pthread_create(&tid, NULL, &printBacktracks, NULL);

	if (solve(numClauses, numVars, solution, clauses, 0)) {
		printf("s SATISFIABLE\nv");
		for (int i = 0; i < numVars; i++) {
			printf(" %d", solution[i] ? i + 1 : -1 * (i + 1));
		}
		printf(" 0\n");
	} else {
		printf("s UNSATISFIABLE\n");
	}

	return 0;
}
