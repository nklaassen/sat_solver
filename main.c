#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

static long long backtracks = 0;
pthread_mutex_t bt_lock;

void *printBacktracks(void *arg)
{
	(void)arg;
	while (true) {
		sleep(2);
		printf("backtracks: %lld\n", backtracks);
	}
	return NULL;
}

bool checkClause(int const numVars, bool solution[numVars], int const *clause)
{
	// if one variable is satisfied, return true
	for (int i = 0; i < numVars; i++) {
		int var = *(clause + i);
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
		int *clauses)
{
	// all clauses need to be satisfied to return true
	for (int i = 0; i < numClauses; i++) {
		if (!checkClause(numVars, solution, clauses + i*numVars)) {
			return false;
		}
	}
	return true;
}

// Global flag, set if/when a solution is found
int solved = 0;

// Recursive function which naively tries all possible truth assignments and tests them
// returns: true if satisfiable, solution contains the satisfying truth assignment
//          false if unsatisfiable
// the "index" argument indicates the depth of recursion, and also the index into the 
// solution that this call is responsible for flipping
bool solve(int const numClauses,
		int const numVars,
		bool solution[numVars],
		int *clauses,
		int const index)
{
	// Check if another thread already found a solution
	if(__sync_fetch_and_add(&solved, 0)) {
		return false;
	}
	if (index == numVars) {
		return checkClauses(numClauses, numVars, solution, clauses);
	}
	solution[index] = true;
	if (solve(numClauses, numVars, solution, clauses, index + 1)) {
		return true;
	}
	pthread_mutex_lock(&bt_lock);
	backtracks++;
	pthread_mutex_unlock(&bt_lock);
	solution[index] = !solution[index];
	return solve(numClauses, numVars, solution, clauses, index + 1);
}

// Struct for passing all the info needed into a thread
typedef struct threadArg 
{
	unsigned threadPow;
	unsigned threadNum;
	int numVars;
	int numClauses;
	int *clauses;
	bool *solutionDone;
} threadArg;

// Start a search on a subset of the solution space
void* solveThread(void* arg)
{
	threadArg info = *(threadArg*)arg;
	bool solution[info.numVars];

	// Set the first few places of the solution
	for(int i=0; i<info.threadPow; i++) {
		solution[i] = (info.threadNum >> i) & 1;
	}

	// Check recursively. If a solution is found, set the global flag and write
	// the solution to solutionDone. 
	if(solve(info.numClauses, info.numVars, solution, info.clauses, info.threadPow)) {
		if(__sync_fetch_and_add(&solved, 1)) {
			for(int i=0; i<info.numVars; i++) {
				info.solutionDone[i] = solution[i];
			}
		}
	}
}

int main()
{
	int numVars;
	int numClauses;
	char junk[128];

	scanf("%s %s %d %d\n", junk, junk, &numVars, &numClauses);

	// Array of clauses. Each clause is an array of integers.
	int* clauses = (int*)malloc(numVars * numClauses * sizeof(int));

	// A potential solution is an array of bools, where the index is the "name" of the variable - 1
	bool solutionDone[numVars];

	// Read all of the clauses
	for (int clause = 0; clause < numClauses; clause++) {
		int var;
		scanf("%d", &var);
		for (int i = 0; i < numVars; i++) {
			*(clauses + clause*numVars + i) = var;
			if (var == 0) {
				break;
			}
			scanf("%d", &var);
		}
	}

	pthread_t bt_tid;
	pthread_create(&bt_tid, NULL, &printBacktracks, NULL);

	// Number of threads must be a power of 2
	unsigned threadPow = 2;
	unsigned numThreads = 1 << threadPow;
	pthread_t tid[numThreads];
	threadArg args[numThreads];

	for(int i=0; i<numThreads; i++)
	{
		// Thread only gets a pointer to the arguments, so we need to 
		// populate a different struct for each thread
		args[i].threadPow = threadPow;
		args[i].threadNum = i;
		args[i].numVars = numVars;
		args[i].numClauses = numClauses;
		args[i].clauses = clauses;
		args[i].solutionDone = solutionDone;
		pthread_create(&tid[i], NULL, &solveThread, &args[i]);
	}

	for(int i=0; i<numThreads; i++)
	{
		pthread_join(tid[i], NULL);
	}

	if (solved) {
		printf("s SATISFIABLE\nv");
		for (int i = 0; i < numVars; i++) {
			printf(" %d", solutionDone[i] ? i + 1 : -1 * (i + 1));
		}
		printf(" 0\n");
	} else {
		printf("s UNSATISFIABLE\n");
	}

	return 0;
}
