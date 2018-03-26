/*
 * Andrew Geltz
 * University of Central Florida
 * Spring 2018
 *
 * Based on the 2004 paper, Lock-Free Linked Lists and Skip Lists
 * By Mikhail Fomitchev and Eric Ruppert at York University
 */

#ifndef FRList_H
#define FRList_H

#include <atomic>
#include <climits>
#include "MarkableReference.hpp"
#include "FRNode.hpp"
#include "Window.hpp"

#define FRL_DEBUG false

#define EPSILON 1

#define CAS(exp, succ) compare_exchange_weak(exp, succ)

template <class T>
class FRList
{
	private:
		FRNode<T>* head;
		FRNode<T>* tail;

	void PrintList ()
	{
		FRNode<T>* curr = head;

		printf ("========== Printing List ==========\n");
		while (curr != NULL)
		{
			printf ("\t(data %d, addr[%p], next[%p], succ %d, mark %d)\n", curr->data, curr, curr->next.GetReference(), curr->next.IsSuccessorMarked(), curr->next.IsMarkedForDeletion());
			curr = curr->next.GetReference ();
		}
	}

	void HelpMarkedForDeletion (FRNode<T>* prev, FRNode<T>* del)
	{
		if (FRL_DEBUG)
			printf ("Called HelpMarkedForDeletion (prev [%p], del [%p])\n", prev, del);

		// Attempt to pysically delete the marked FRNode and unflag prev
		FRNode<T>* next = del->next.GetReference();

		if (FRL_DEBUG)
			printf ("Attempting CAS (exp[%p], success[%p], expSucc %d, successSucc %d, expDel %d, successDel %d\n",
				del, next, true, false, false, false);

		// Expect successor flag and set it to false
		prev->next.CompareAndSet (del, next, true, false, false, false);
	}

	Window<T> SearchFrom (T data, FRNode<T>* from)
	{
		if (FRL_DEBUG)
			printf ("Called SearchFrom (%d, [%p])\n", data, from);

		// Find two consecutive FRNode such that n1.key <= t.key < n2
		FRNode<T>* curr = from;
		FRNode<T>* next = from->next.GetReference ();
		while (next->data <= data)
		{
			if (FRL_DEBUG)
			{
				printf ("\tSearchFrom Loop - curr (%d)[%p][%p], next (%d)[%p][%p]\n", curr->data, curr, curr->next.GetReference(), next->data, next, next->next.GetReference());
			}
			while (next->next.IsMarkedForDeletion () &&
					(!curr->next.IsMarkedForDeletion () || curr->next.GetReference () != next))
			{
				if (curr->next.GetReference () == next)
					HelpMarkedForDeletion (curr, next);
				next = curr->next.GetReference ();
			}
			if (next->data <= data)// Move down list
			{
				curr = next;
				next = curr->next.GetReference ();
			}
		}

		Window<T> w (curr, next);

		return w;
	}

	void HelpSuccessorFlagged (FRNode<T>* prev, FRNode<T>* del)
	{
		if (FRL_DEBUG)
			printf ("Called HelpSuccessorFlagged ([%p], [%p])\n", prev, del);

		// Attempt to mark and physically delete del since prev is flagged
		del->backlink.store(prev);

		if (!del->next.IsMarkedForDeletion())
			TryMarkForDeletion (del);
		HelpMarkedForDeletion (prev, del);
	}

	void TryMarkForDeletion (FRNode<T>* n)
	{
		if (FRL_DEBUG)
			printf ("Called TryMarkForDeletion (data %d, addr[%p], next[%p], succ %d, del %d)\n",
				n->data, n, n->next.GetReference(), n->next.IsSuccessorMarked(), n->next.IsMarkedForDeletion());

		do
		{
			// We need the next FRNode so we can update the flag
			FRNode<T>* next = n->next.GetReference ();

			if (FRL_DEBUG)
				printf ("Trying to replace ([%p], %d, %d) with ([%p], %d, %d)\n",
					next, next->next.IsSuccessorMarked(), next->next.IsMarkedForDeletion(), next, 0, 1);

			// If our CAS fails due to n successor being marked for deletion, help it and try again
			if (n->next.CompareAndSet (next, next, false, false, false, true))
				if (n->next.IsSuccessorMarked())
					HelpSuccessorFlagged (n, next);
		} while (!n->next.IsMarkedForDeletion ());

		if (FRL_DEBUG)
			printf ("Marked (data %d, [%p]) for deletion\n", n->data, n);
	}

	bool TryFlagSuccessor (FRNode<T>* _prev, FRNode<T>* _target)
	{
		if (FRL_DEBUG)
			printf ("Called TryFlagSuccessor (prev[%p], target[%p])\n", _prev, _target);

		FRNode<T>* prev = _prev;
		FRNode<T>* target = _target;
		while (true)
		{
			if (prev->next.IsSuccessorMarked())// If the FRNode already has successor flag
			{
				if (FRL_DEBUG)
					printf ("Target FRNode already had successor flag\n");
				return false;
			}

			if (prev->next.CompareAndSet(target, target, false, true, false, false))// Attempt to assert the successor flag
			{
				if (FRL_DEBUG)
					printf ("Was able to set successor flag on prev FRNode [%p] for target [%p]\n", prev, target);
				return true;// We were successful
			}

			while (prev->next.IsMarkedForDeletion ())// If the CAS failed because previous FRNode is marked for deletion
			{
				if (FRL_DEBUG)
					printf ("CAS failed because prev was marked for deletion, backtracking\n");
				prev = prev->backlink;// Go back up the chain one step
			}

			// Try to reaquire FRNodes if something moved
			Window<T> w = SearchFrom(target->data, prev);

			if (FRL_DEBUG)
			{
				PrintList ();
				printf ("Got window:\n");
				printf ("\tpred (data %d, addr[%p], next[%p], succ %d, mark %d)\n",
					w.pred->data, w.pred, w.pred->next.GetReference(), w.pred->next.IsSuccessorMarked(), w.pred->next.IsMarkedForDeletion());
				printf ("\tcurr (data %d, addr[%p], next[%p], succ %d, mark %d)\n",
					w.curr->data, w.curr, w.curr->next.GetReference(), w.curr->next.IsSuccessorMarked(), w.curr->next.IsMarkedForDeletion());
			}

			prev = w.pred;

			if (w.curr != target)// We didn't find our FRNode
			{
				if (FRL_DEBUG)
					printf ("Lost target FRNode after backtracking, maybe another FRNode removed it\n");
				return false;
			}
		}
	}

	public:
		FRList ()
		{
			head = new FRNode<T> (INT_MIN);
			tail = new FRNode<T> (INT_MAX);
			head->next.Set(tail, false, false);
			tail->next.Set(NULL, false, false);

			if (FRL_DEBUG)
			{
				printf ("Just created list\n");
				PrintList ();
			}
		}

		void Add (FRNode<T>* n)
		{
			if (FRL_DEBUG)
				printf ("Called Add (data %d, addr [%p])\n", n->data, n);

			FRNode<T>* prev;
			FRNode<T>* next;

			// Look for the placement of our new FRNode
			Window<T> w = SearchFrom (n->data, head);
			prev = w.pred;
			next = w.curr;

			if (FRL_DEBUG)
			{
				PrintList ();
				printf ("Got window:\n");
				printf ("\tpred (data %d, addr[%p], next[%p], succ %d, mark %d)\n",
					w.pred->data, w.pred, w.pred->next.GetReference(), w.pred->next.IsSuccessorMarked(), w.pred->next.IsMarkedForDeletion());
				printf ("\tcurr (data %d, addr[%p], next[%p], succ %d, mark %d)\n",
					w.curr->data, w.curr, w.curr->next.GetReference(), w.curr->next.IsSuccessorMarked(), w.curr->next.IsMarkedForDeletion());
			}

			// If we find that FRNode already in the list, return
			if (prev->data == n->data)
			{
				if (FRL_DEBUG)
					printf ("Cannot insert %d into list because it already exists\n", n->data);
				return;
			}

			while (true)
			{
				if (prev->next.GetReference()->next.IsSuccessorMarked())// If pred is flagged, help
				{
					HelpSuccessorFlagged (prev, prev->next.GetReference());
				} else {
					// Set the next pointer for the new FRNode to the next FRNode in the list
					n->next.Set(next, false, false);
					
					// Point prev to our new FRNode instead of next
					if (prev->next.CompareAndSet(next, n, false, false, false, false))
					{
						if (FRL_DEBUG)
						{
							printf ("Successfully added FRNode (data %d, [%p]) into the list\n", n->data, n);
							PrintList ();
						}
						return;
					} else {
						if (prev->next.IsSuccessorMarked())// If we failed becuase prev's successor is flagged
							HelpSuccessorFlagged (prev, prev->next.GetReference());
						
						while (prev->next.IsMarkedForDeletion())// If we failed becuase prev is marked
							prev = prev->backlink;
					}

				
				}
			}
		}

		FRNode<T>* Remove (T data)
		{
			if (FRL_DEBUG)
				printf ("Called Remove(%d)\n", data);
			
			// Find FRNode we are looking to delete
			Window<T> w = SearchFrom (data - EPSILON, head);// Search for (prev, target) by undershooting

			if (FRL_DEBUG)
			{
				PrintList ();
				printf ("Got window:\n");
				printf ("\tpred (data %d, addr[%p], next[%p], succ %d, mark %d)\n",
					w.pred->data, w.pred, w.pred->next.GetReference(), w.pred->next.IsSuccessorMarked(), w.pred->next.IsMarkedForDeletion());
				printf ("\tcurr (data %d, addr[%p], next[%p], succ %d, mark %d)\n",
					w.curr->data, w.curr, w.curr->next.GetReference(), w.curr->next.IsSuccessorMarked(), w.curr->next.IsMarkedForDeletion());
			}

			if (w.curr->data != data)// FRNode was not found in list
			{
				if (FRL_DEBUG)
					printf ("Couldn't find %d in list to remove\n", data);
				return NULL;
			}

			bool result = TryFlagSuccessor (w.pred, w.curr);

			if (w.pred != NULL)
				HelpSuccessorFlagged (w.pred, w.curr);
			
			if (!result)
			{
				if (FRL_DEBUG)
					printf ("Couldn't flag successor so not removing anything\n");
				return NULL;
			}

			if (FRL_DEBUG)
				printf ("Successfully removed (data %d, [%p])\n", w.curr->data, w.curr);

			return w.curr;
		}

		bool Contains (T data)
		{
			if (FRL_DEBUG)
				printf ("Called Contains (%d)\n", data);

			Window<T> w = SearchFrom (data, head);

			if (FRL_DEBUG)
			{
				PrintList ();
				printf ("Got window:\n");
				printf ("\tpred (data %d, addr[%p], next[%p], succ %d, mark %d)\n",
					w.pred->data, w.pred, w.pred->next.GetReference(), w.pred->next.IsSuccessorMarked(), w.pred->next.IsMarkedForDeletion());
				printf ("\tcurr (data %d, addr[%p], next[%p], succ %d, mark %d)\n",
					w.curr->data, w.curr, w.curr->next.GetReference(), w.curr->next.IsSuccessorMarked(), w.curr->next.IsMarkedForDeletion());
			}

			return (w.pred->data == data);
		}
};

#endif