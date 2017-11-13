// Code3b.cpp : Defines the entry point for the console application.
// 

/*
* Code 3 (bonus)
* Solve challenge 1 and challenge 2, but this time (if you haven't already done so) make it thread safe without using a global locks.
* If you find a solution to this, think about what some issues with it may be, and edge cases that could be very difficult to handle.
* If you handle those edge cases then that's great, but we don't expect that.
*/

#include "stdafx.h"

#include<iostream>
#include<fstream>
#include <thread>
#include<string>
#include<atomic>

using namespace std;

class DoubleList
{
private:

	struct DoubleNode
	{
		int data;
		DoubleNode* prev;
		DoubleNode* next;
	};

	atomic<DoubleNode*> tail{ nullptr }; //Shared variable made atomic

public:
	DoubleList()
	{
		tail.store(new DoubleNode);
		DoubleNode* t = tail.load();
		t->data = 0;
		t->prev = NULL;
		t->next = NULL;
	}

	~DoubleList()
	{
		DoubleNode* last = tail.load();
		while (last->data != NULL)
		{
			DoubleNode* unlinked = last;
			last = last->prev;
			delete unlinked;
		}
	}

	void addnode(int value)
	{
		DoubleNode *temp = new DoubleNode;
		DoubleNode* p = tail.load();

		temp->data = value;
		temp->prev = tail.load();
		temp->next = NULL;
		p->next = temp;

		//transaction of temp data to shared tail if next node is as expected
		while (!tail.compare_exchange_weak(temp->prev, temp))
		{
		};


	}

	void removenode(int value)
	{

		bool lookfornode = true;

		DoubleNode *temp;
		temp = tail.load();

		do
		{
			if (temp->data == value)
			{
				cout << "Found: " << value << endl;
				if (temp->prev == NULL)
				{
					DoubleNode* n = temp->next;
					n->prev = NULL;
				}
				else if (temp == tail.load())
				{
					DoubleNode* p = temp->prev;
					p->next = NULL;

					//transaction of next pointed element of tail to temp if pointer is as expected
					while (!tail.compare_exchange_weak(temp->prev, p))
					{
					};
				}
				else
				{
					DoubleNode* p = temp->prev;
					DoubleNode* n = temp->next;

					p->next = n;
					n->prev = p;
				}

				lookfornode = false;
			}

			temp = temp->prev;

		} while ((temp != NULL) && (lookfornode));
	}


	void showlistfromtail()
	{
		DoubleNode *temp = new DoubleNode;
		temp = tail.load();
		cout << "Addr\tData\tPrevAddr\tNextAddr\n";
		while (temp != NULL)
		{
			cout << temp << "\t" << temp->data << "\t" << temp->prev << "\t" << temp->next << "\n";
			temp = temp->prev;
		}
		cout << endl;
	}
};

void adddoublelistitems(DoubleList& list, int s)
{
	for (int i = s; i < (s + 100); i++)
	{
		list.addnode(i);
	}
}

void removedoublelistitems(DoubleList& list, int s)
{
	for (int i = s; i < 500; i += s)
	{
		list.removenode(i);
	}
}

int main()
{
	DoubleList doublelist1;

	std::thread t1(adddoublelistitems, std::ref(doublelist1), 101); // t1 starts running
	std::thread t2(adddoublelistitems, std::ref(doublelist1), 201); // t2 starts running
	std::thread t3(removedoublelistitems, std::ref(doublelist1), 10); // t3 starts running

	try
	{
		for (int i = 1; i < 100; i++)
		{
			doublelist1.addnode(i);
		}
	}
	catch (...)
	{
		t1.join();
		t2.join();
		t3.join();
		throw;
	}

	t1.join();
	t2.join();
	t3.join();

	doublelist1.showlistfromtail();

	return 0;
}



