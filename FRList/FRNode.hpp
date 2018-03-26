#ifndef FRNode_H
#define FRNode_H

#include "MarkableReference.hpp"

// Forward declaration required to avoid circular dependency
template <class T>
class MarkableReference;

template <class T>
class FRNode
{
	public:
		T data;
		std::atomic<FRNode<T>*> backlink;
		MarkableReference<T> next;

		FRNode () {}

		FRNode (T _data) {
			data = _data;
		}
};

#endif