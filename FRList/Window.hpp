#ifndef WINDOW_H
#define WINDOW_H

#include "FRNode.hpp"

template <class T>
class Window
{
	public:
		FRNode<T>* pred;
		FRNode<T>* curr;

		Window (FRNode<T>* _pred, FRNode<T>* _curr)
		{
			pred = _pred;
			curr = _curr;
		}
};

#endif