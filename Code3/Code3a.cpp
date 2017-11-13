// Code3a.cpp : Defines the entry point for the console application.
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
#include<vector>

using namespace std;


class SingleList
{
private:
	struct Node
	{
		int data;
		Node* prev;
	};

	atomic<Node*> tail{ nullptr }; //Mutable shared data where atomic is needed

public:
	SingleList()
	{
		tail = new Node;
		Node* t = tail;
		t->data = 0;
		t->prev = NULL;
	}

	~SingleList()
	{
		Node* last = tail.load();
		while (last->data != NULL)
		{
			Node* unlinked = last;
			last = last->prev;
			delete unlinked;
		}
	}

	void addnode(int value)
	{
		Node *temp = new Node;
		temp->data = value;
		temp->prev = tail;

		//transaction of temp data to shared tail if next node is as expected
		while (!tail.compare_exchange_weak(temp->prev, temp)) 
		{};
	}

	void showsinglelistfromtail()
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

void addsinglelistitems(SingleList& SingleList, int s)
{
	for (int i = s; i < (s + 100); i++)
	{
		SingleList.addnode(i);
	}
}

int main()
{
	SingleList singlelist1;

	std::thread t1(addsinglelistitems, std::ref(singlelist1), 101); // t1 starts running
	std::thread t2(addsinglelistitems, std::ref(singlelist1), 201); // t2 starts running

	try
	{
		for (int i = 1; i < 100; i++)
		{
			singlelist1.addnode(i);
		}
	}
	catch (...)
	{
		t1.join();
		t2.join();
		throw;
	}

	t1.join();
	t2.join();
	singlelist1.showsinglelistfromtail();

	return 0;
}

