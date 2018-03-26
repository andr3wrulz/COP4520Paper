#ifndef WINDOW_H
#define WINDOW_H

#include "Node.hpp"

template <class T>
class Window
{
	public:
		Node<T>* pred;
		Node<T>* curr;

		Window (Node<T>* _pred, Node<T>* _curr)
		{
			pred = _pred;
			curr = _curr;
		}
};

#endif