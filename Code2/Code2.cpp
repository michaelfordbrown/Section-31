// Code2.cpp : Defines the entry point for the console application.

/*
 * Make the list in challenge 2 doubly linked and implement node removal.
  * Again; make it thread safe.
  */

#include "stdafx.h"

#include<iostream>
#include<fstream>
#include <thread>
#include<string>
#include<mutex>

using namespace std;


class List
{
private:
	struct Node
	{
		int data;
		Node* prev;
		Node* next;
	};

	Node* tail;
	mutex _mut; //synchronization primitive that can be used to protect shared data from being simultaneously accessed by multiple threads

public:
	List()
	{
		tail = new Node;
		tail->data = 0;
		tail->prev = NULL;
		tail->next = NULL;
	}

	void addnode(int value)
	{
		/*
		* mutex wrapper that provides a convenient RAII-style mechanism for owning a mutex
		* for the duration of a scoped block.
		*/
		std::lock_guard<mutex> locker(_mut); //ENTER CRITICAL SECTION

		Node *temp = new Node;
		temp->data = value;
		temp->prev = tail;
		tail->next = temp;
		temp->next = NULL;
				
		tail = temp; //EXIT CRITICAL SECTION
	}

	void removenode(int value)
	{
		/*
		* mutex wrapper that provides a convenient RAII-style mechanism for owning a mutex
		* for the duration of a scoped block.
		*/
		std::lock_guard<mutex> locker(_mut); //ENTER CRITICAL SECTION

		bool lookfornode = true;

		Node *temp;
		temp = tail;

		do
		{
			if (temp->data == value)
			{
				cout << "Found: " << value << endl;
				if (temp->prev == NULL)
				{
					Node* n = temp->next;
					n->prev = NULL;
				}
				else if (temp == tail)
				{
					Node* p = temp->prev;
					p->next = NULL;
					tail = p;
				}
				else
				{
					Node* p = temp->prev;
					Node* n = temp->next;

					p->next = n;
					n->prev = p;
				}

				lookfornode = false;
			}

			temp = temp->prev;

		} while ((temp != NULL) && (lookfornode)); //EXIT CRITICAL SECTION
	}


	void showlistfromtail()
	{
		Node *temp = new Node;
		temp = tail;
		cout << "Addr\tData\tPrevAddr\tNextAddr\n";
		while (temp != NULL)
		{
			cout << temp << "\t" << temp->data << "\t" << temp->prev << "\t" << temp->next << "\n";
			temp = temp->prev;
		}
		cout << endl;
	}
};

void addlistitems(List& list, int s)
{
	for (int i = s; i < (s + 100); i++)
	{
		list.addnode(i);
	}
}

void removelistitems(List& list, int s)
{
	for (int i = s; i < 500; i += s)
	{
		list.removenode(i);
	}
}

int main()
{
	List list1;

	std::thread t1(addlistitems, std::ref(list1), 101); // t1 starts running
	std::thread t2(addlistitems, std::ref(list1), 201); // t2 starts running
	std::thread t3(removelistitems, std::ref(list1), 10); // t3 starts running

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
		*In case of event allow main() thread to terminate.
		*/
		t1.join();
		t2.join();
		t3.join();
		throw;
	}

	/* Allow main() thread to terminate*/
	t1.join();
	t2.join();
	t3.join();

	list1.showlistfromtail();

	return 0;
}



