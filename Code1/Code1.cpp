// Code1.cpp : Defines the entry point for the console application.
//

/*
 * Code1
 * Implement a singly linked list of nodes where a node is
 *
 * struct Node
 * {
 *		int data;
 *		Node* prev;
 *	}
 *
 * so that you can add elements to the list (but not remove them). The list starts at a "tail" node, initially set to 0;
 *
 * Node* tail
 *
 * Adding elements must be thread safe.
 */

#include "stdafx.h"
#include<iostream>
#include<fstream>
#include <thread>
#include<string>
#include<mutex>
#include<vector>

using namespace std;

class List
{
	private: 
		struct Node
		{
			int data;
			Node* prev;
		};

		mutex _mut; //synchronization primitive that can be used to protect shared data from being simultaneously accessed by multiple threads
		Node* tail;


	public:
	List()
	{
		tail = new Node;
		tail->data = 0;
		tail->prev = NULL;
	}

	void addnode(int value)
	{
		/*
		 * mutex wrapper that provides a convenient RAII-style mechanism for owning a mutex 
		 * for the duration of a scoped block. 
		 */

		/*
		 * Resource acquisition is initialization (RAII), binds the life cycle of a resource that 
		 * must be acquired before use (allocated heap memory, thread of execution, 
		 * open socket, open file, locked mutex, disk space, database connection—anything 
	     * that exists in limited supply) to the lifetime of an object. 
		 */
		std::lock_guard<mutex> locker(_mut); // ENTER CRITICAL SECTION

		Node *temp = new Node;
		temp->data = value;
		temp->prev = tail;

		tail = temp; // EXIT CRITICAL SECION
	}

	void showlistfromtail()
	{
		Node *temp = new Node;
		temp = tail;
		cout << "Addr\tData\tPrevAddr\n";
		while (temp != NULL)
		{
			cout << temp << "\t" << temp->data << "\t" << temp->prev << "\n";
			temp = temp->prev;
		}
		cout << endl;
	}

};

void addlistitems(List& list, int s)
{
	for (int i = s; i < (s+100); i++)
	{
		list.addnode(i);
	}
}

int main()
{
	List list1;

	/*
	* Threads of execution allowing multiple functions to run concurrently
	*
	* Parameters are reference to the queue and execution iterator
	*/
	std::thread t1(addlistitems, std::ref(list1), 101); // t1 starts running
	std::thread t2(addlistitems, std::ref(list1), 201); // t2 starts running

	try
	{
		for (int i = 1; i < 100; i++)
		{
			list1.addnode(i);
		}
	}
	catch (...)
	{
		/*
		* In case of event allow main() thread to terminate.
		*/
		t1.join();
		t2.join();
		throw;
	}

	/* Allow main() thread to terminate*/
	t1.join();
	t2.join();
	list1.showlistfromtail();

    return 0;
}

