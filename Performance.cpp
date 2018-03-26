#include <stdio.h>
#define HAVE_STRUCT_TIMESPEC // Needed so pthreadwin32 does not overwrite struct timespec
#include <pthread.h>
#include <ctime>
#include <cstdlib>
#include <vector>

#include "FRList\FRList.hpp"
#include "FRList\FRNode.hpp"

#define OPS_PER_THREAD 5

struct FRThreadData
{
	int threadId;
	FRList<int>* list;
	std::vector<FRNode<int>>* nodeArray;
	int addChance;
	int removeChance;
	int containsChance;
};

void* FRThreadLogic (void* threadArgs)
{
	/*// Cast pointer to data struct so we can use it
	struct FRThreadData* data;
	data = (struct FRThreadData*) threadArgs;

	// Do all the operations
	for (int ops = 0; ops < OPS_PER_THREAD; ops++)
	{
		int random = rand () % 1000 + 1;
		if (random < data->addChance)// Add
		{
			// Grab node from list
			FRNode<int>* n = &data->nodeArray[data->arrayIndex++];
			n->data = random;// Data will be range [1, addChance)

			// Add the node
			data->list->Add (n);
		}
		else if (random < data->addChance + data->removeChance)// Remove
		{
			data->list->Remove (random - data->removeChance);// Remove something in range [1, addChance)
		}
		else // Contains
		{
			data->list->Contains (random - data->removeChance - data->containsChance);// Look for something in range [1, addChance)
		}
	}*/
}

double* FRDoTest (double* times, int addChance, int removeChance, int containsChance)
{
	if (addChance + removeChance + containsChance != 1000)
		printf ("[ERROR] FRDoTest called with invalid parameters\n\tAdd (%d) + Remove (%d) + Contains (%d) != 100!\n", addChance, removeChance, containsChance);

	times = new double [4];

	printf ("\n===== Starting FRList Test - %d Add, %d Remove, %d Contains =====\n", addChance, removeChance, containsChance);

	int threadCounts [] = {1, 2, 4, 8};
	for (int threadCountIndex = 0; threadCountIndex < 4; threadCountIndex++)
	{
		int numThreads = threadCounts[threadCountIndex];
		printf ("Testing with %d threads \n", numThreads);

		FRList<int> list;

		printf ("Allocating nodes...\n");
		// Create OPS_PER_THREAD nodes for each thread so they can't run out
		
		/*
		printf ("Creating thread data...\n");
		struct FRThreadData* threadData = new struct FRThreadData [numThreads];
		for (int i = 0; i < numThreads; i++)
		{
			threadData[i].threadId = i;
			threadData[i].list = &list;
			threadData[i].nodeArray = preallocatedNodes[i];
			threadData[i].arrayIndex = 0;
			threadData[i].addChance = addChance;
			threadData[i].removeChance = removeChance;
			threadData[i].containsChance = containsChance;
		}*/

		/* pthread_t* threads = new pthread_t [numThreads];

		printf ("Starting evaluation...\n");
		clock_t begin = clock ();
		// Spin off threads
		for (int i = 0; i < numThreads; i++)
			pthread_create (&threads[i], NULL, FRThreadLogic, (void*)&threadData[i]);

		// Join them back together
		for (int i = 0; i < numThreads; i++)
			pthread_join (threads[i], NULL);

		clock_t end = clock (); 

		times [threadCountIndex] = double (begin-end) / CLOCKS_PER_SEC;*/
	}

	return times;
}

void FRTests ()
{
	double* results = new double [3];
	// Test 1 - 34% Add, 33% Remove, 33% Contains
	FRDoTest (&results[0], 340, 330, 330);

	// Test 2 - 50% Add, 50% Remove, 0% Contains
	FRDoTest (&results[1], 500, 500, 0);

	// Test 3 - 25% Add, 25% Remove, 50% Contains
	FRDoTest (&results[2], 250, 250, 500);

	printf ("Test 1: %lf\nTest 2: %ld\nTest 3: %lf\n", results[0], results[1], results[2]);
}

int main ()
{
	printf (".\n");
	srand (time(NULL));

	printf ("Starting FRList Tests\n");
	FRTests ();

	return 0;
}