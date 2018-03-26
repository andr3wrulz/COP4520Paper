#include <stdio.h>
#include <stdarg.h>
#include "Node.hpp"
#include "MarkableReference.hpp"
#include "Window.hpp"
#include "FRList.hpp"

class Results
{
	private:
		int numberOfTests;
		int numberOfFailures;

	public:
		Results ()
		{
			numberOfTests = 0;
			numberOfFailures = 0;
		}

		void PrintResults ()
		{
			printf ("\n");
			printf ("=========================\n");
			printf ("    Number of Tests: %d\n", numberOfTests);
			printf ("    Number of Fails: %d\n", numberOfFailures);
			printf ("=========================\n\n");
		}

		void Assert (bool condition, const char* format, ...)
		{
			numberOfTests++;

			if (condition)// If our condition passes, do nothing
				return;

			va_list args;
			va_start (args, format);
			vprintf (format, args);
			va_end (args);

			numberOfFailures++;
		}

		bool AllPasses ()
		{
			return numberOfFailures == 0;
		}
};

bool MarkableReferenceTests ()
{
	printf ("=============== Starting MarkableReference.hpp Unit Tests ==============\n");

	Results r;

	// Initialization
	Node<int> n (5);
	MarkableReference<int> mr (&n);// Markable Reference to n

	// Pointer resolution test no flags
	r.Assert ((mr.GetReference() == &n), "Pointer points to [%p] but should point to [%p] with no flags\n", mr.GetReference(), &n);

	// Deletion mark
	mr.Set (&n, false, true);
	r.Assert ((mr.IsMarkedForDeletion()), "Marked reference for deletion but IsMarkedForDeletion returned false\n");

	// Pointer resolution test marked for deletion
	r.Assert ((mr.GetReference() == &n), "Pointer points to [%p] but should point to [%p] with deletion flag\n", mr.GetReference(), &n);

	// Successor marked flag
	mr.Set (&n, true, false);
	r.Assert ((mr.IsSuccessorMarked()), "Marked successor flag but IsSuccessorMarked returned false\n");

	// Pointer resolution test successor marked
	r.Assert ((mr.GetReference() == &n), "Pointer points to [%p] but should point to [%p] with successor flag\n", mr.GetReference(), &n);

	// Both flags
	mr.Set (&n, true, true);
	r.Assert ((mr.IsSuccessorMarked() && mr.IsMarkedForDeletion()), "Marked both flags but IsSuccessorMarked: %s and IsMarkedForDeletion: %s\n", 
		(mr.IsSuccessorMarked() ? "true" : "false"), (mr.IsMarkedForDeletion() ? "true" : "false"));

	// Pointer resolution test both flags
	r.Assert ((mr.GetReference() == &n), "Pointer points to [%p] but should point to [%p] with both flag\n", mr.GetReference(), &n);

	// Compare and swap test
	Node<int> n2 (7);
	mr.CompareAndSet (&n, &n2, true, false, true, false);
	r.Assert ((mr.GetReference() == &n2 && !mr.IsSuccessorMarked() && !mr.IsMarkedForDeletion()),
		"Compare and swap failed. Pointer should be [%p] was [%p], successor flag should be false was %s, deletion mark should be false was %s\n",
		&n2, mr.GetReference(), (mr.IsSuccessorMarked() ? "true" : "false"), (mr.IsMarkedForDeletion() ? "true" : "false"));

	r.PrintResults ();

	return r.AllPasses ();
}

bool NodeTests ()
{
	printf ("===================== Starting Node.hpp Unit Tests =====================\n");

	Results r;

	// Initialization test
	Node<int> n (5);
	r.Assert ((n.data == 5), "Failed Node initialization test, %d != 5\n", n.data);

	// Backlink test
	Node<int> n2 (7);
	n.backlink.store(&n2);
	r.Assert ((n.backlink.load() == &n2), "Stored [%p] into the backlink but retrieved [%p]\n", &n2, n.backlink.load());

	r.PrintResults ();

	return r.AllPasses ();
}

bool WindowTests ()
{
	printf ("==================== Starting Window.hpp Unit Tests ====================\n");

	Results r;

	// Initialization test
	Node<int> n1 (5);
	Node<int> n2 (7);
	Window<int> w (&n1, &n2);
	r.Assert ((w.pred->data == 5 && w.curr->data == 7), "Failed initialization test pred.data should be 5 was %d and curr.data should be 7 was %d\n",
		w.pred->data, w.curr->data);

	r.PrintResults ();

	return r.AllPasses ();
}

bool FRListTests ()
{
	printf ("==================== Starting FRList.hpp Unit Tests ====================\n");

	Results r;

	FRList<int> list;

	// Contains empty list
	r.Assert ((!list.Contains(5)), "Called contains (5) on an empty list and got true\n");

	// Remove on empty list
	Node<int>* retVal = list.Remove (5);
	r.Assert ((retVal == NULL), "Called Remove (5) on an empty list and got [%p] instead of null\n", retVal);

	// Insert a node
	Node<int> n (7);
	list.Add (&n);
	r.Assert ((list.Contains(7)), "Inserted node (data %d, addr[%p]) into list but did not find it with contains\n", n.data, &n);

	// Remove the node
	retVal = list.Remove(7);
	r.Assert ((retVal == &n), "Tried Remove (7) and got address [%p] but should be [%p]\n", retVal, &n);

	r.PrintResults ();

	return r.AllPasses ();
}

int main ()
{
	bool anyFailures = false;
	anyFailures |= !MarkableReferenceTests ();
	anyFailures |= !NodeTests ();
	anyFailures |= !WindowTests ();
	anyFailures |= !FRListTests ();

	if (anyFailures)
		printf ("[ERROR] Test(s) did not complete successfully, please review!!!\n");
	else
		printf ("[SUCCESS] All tests completed successfully!!!\n");
}