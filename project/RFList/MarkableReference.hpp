#ifndef MARKABLE_REFERENCE_H
#define MARKABLE_REFERENCE_H

// Toggle for debug printing
#define MR_DEBUG_FLAG false

#include "Node.hpp"
#include <atomic>

#include <stdio.h>

// 2^0 bit is the marked for deletion flag
#define MARKED_FOR_DELETION_BIT 0x01
// 2^1 bit is the successor mark flag
#define SUCCESSOR_BIT 0x02
#define BOTH_BITS 0x03

#define CAS(exp, succ) compare_exchange_weak(exp, succ)

// Forward declaration required to avoid circular dependency
template <class T>
class Node;

template <class T>
class MarkableReference
{
	public:
		std::atomic<Node<T>*> ptr;

		MarkableReference () {}

		MarkableReference (Node<T>* _ptr)
		{
			ptr.store(_ptr);
		}

		// So it points to a byte aligned address, we need to remove the last two bits
		Node<T>* GetReference ()
		{
			return (Node<T>*)((uintptr_t)(ptr.load()) & ~BOTH_BITS);
		}

		bool IsMarkedForDeletion ()
		{
			return (uintptr_t)(ptr.load()) & MARKED_FOR_DELETION_BIT;
		}

		bool IsSuccessorMarked ()
		{
			return (uintptr_t)(ptr.load()) & SUCCESSOR_BIT;
		}

		bool Set (Node<T>* n, bool successorMarked, bool deletionMark)
		{
			if (MR_DEBUG_FLAG)
			{
				printf ("Called MR.Set([%p], %s, %s)\n", n, (successorMarked ? "true" : "false"), (deletionMark ? "true" : "false"));
				printf ("\tPointer [%p] | Successor Bit(%p) | Marked Bit (%p)\n", (uintptr_t)(n), ((successorMarked) ? SUCCESSOR_BIT : 0x0), ((deletionMark) ? MARKED_FOR_DELETION_BIT : 0x0));
			}

			Node<T>* newValue = (Node<T>*)(
				(uintptr_t)(n) |
				((successorMarked) ? SUCCESSOR_BIT : 0x0) |
				((deletionMark) ? MARKED_FOR_DELETION_BIT : 0x0));

			if (MR_DEBUG_FLAG)
				printf ("\tSetting pointer to [%p]\n", newValue);

			ptr.store (newValue);
		}

		bool CompareAndSet (Node<T>* expected, Node<T>* success, bool expectedSuccessor, bool successSuccessor,
							bool expectedDeletionMark, bool successDeletionMark)
		{
			if (MR_DEBUG_FLAG)
				printf ("Called MR.CAS(exp [%p], success [%p], expSuccesor %s, successSuccessor %s, expDel %s, successDel %s\n",
					expected, success, (expectedSuccessor ? "true" : "false"), (successSuccessor ? "true" : "false"),
					(expectedDeletionMark ? "true" : "false"), (successDeletionMark ? "true" : "false"));

			// Need to apply the indicated flags for the CAS to work
			Node<T>* modifiedExpected = (Node<T>*)(
				(uintptr_t)(expected) |
				((expectedSuccessor) ? SUCCESSOR_BIT : 0x0) |
				((expectedDeletionMark) ? MARKED_FOR_DELETION_BIT : 0x0));

			if (MR_DEBUG_FLAG)
				printf ("\tExpected = [%p] | (%p) | (%p)\n",
					(uintptr_t)(expected), ((expectedSuccessor) ? SUCCESSOR_BIT : 0x0), ((expectedDeletionMark) ? MARKED_FOR_DELETION_BIT : 0x0));

			Node<T>* modifiedSuccess = (Node<T>*)(
				(uintptr_t)(success) |
				((successSuccessor) ? SUCCESSOR_BIT : 0x0) |
				((successDeletionMark) ? MARKED_FOR_DELETION_BIT : 0x0));
			
			if (MR_DEBUG_FLAG)
				printf ("\tSuccess = [%p] | (%p) | (%p)\n",
					(uintptr_t)(success), ((successSuccessor) ? SUCCESSOR_BIT : 0x0), ((successDeletionMark) ? MARKED_FOR_DELETION_BIT : 0x0));

			return ptr.CAS(modifiedExpected, modifiedSuccess);
		}
};

#endif