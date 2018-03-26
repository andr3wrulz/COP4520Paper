#ifndef NODE_H
#define NODE_H

#include "MarkableReference.hpp"

// Forward declaration required to avoid circular dependency
template <class T>
class MarkableReference;

template <class T>
class Node
{
	public:
		T data;
		std::atomic<Node<T>*> backlink;
		MarkableReference<T> next;

		Node () {}

		Node (T _data) {
			data = _data;
		}
};

#endif