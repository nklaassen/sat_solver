#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <omp.h>

// Number of variables that will be set before each thread tries to find a solution
#define PREFIX 3
#define MAX_THREADS 1 << PREFIX

// In general, don't use volatile for thread safety, it doesn't work.
// Since all we are going to do is set this variable to true once, and all the threads *eventually* need to
// know about it, volatile is probably enough so that the compiler won't completely optimize it out.
// Using a lock here would actually be a significant performance cost.
static volatile bool done = false;

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
	if (done) {
		return true;
	}
	if (index >= numVars) {
		return checkClauses(numClauses, numVars, solution, clauses);
	}
	solution[index] = true;
	if (solve(numClauses, numVars, solution, clauses, index + 1)) {
		return true;
	}
	__sync_fetch_and_add(&backtracks, 1);
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

	bool satisfiable = false;
	bool solution[numVars];

#pragma omp parallel for
	for (int i = 0; i < MAX_THREADS; i++) {
		if (!satisfiable) {
			// A potential solution is an array of bools, where the index is the "name" of the variable - 1
			bool guess[numVars];
			for (int bits = i, j = 0; j < PREFIX && j < numVars; bits >>= 1, j++) {
				guess[j] = (bits & 1) != 0;
			}
			bool sat = solve(numClauses, numVars, guess, clauses, PREFIX);
#pragma omp critical
			if (sat && !satisfiable) {
				done = true;
				satisfiable = true;
				memcpy(solution, guess, sizeof(solution));
			}
		}
	}

	if (satisfiable) {
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
