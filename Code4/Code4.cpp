// Code4.cpp : Defines the entry point for the console application.
//
/*
 * Code 4 (Very Big Bonus)
 * A queue, as above
 */

#include "stdafx.h"

#include<iostream>
#include<fstream>
#include <thread>
#include<string>
#include<array>
#include<atomic>

#include<chrono>

#define MAXITEMQUEUE 25

using namespace std;

class ExecutionTimer
{
public:
	using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
		std::chrono::high_resolution_clock,
		std::chrono::steady_clock>;

	ExecutionTimer() = default;

	~ExecutionTimer()
	{
		std::cout
			<< "\nElapsed: "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - mStart).count()
			<< " ms"
			<< std::endl;
	}

private:
	Clock::time_point mStart = Clock::now();
};

struct  DataItem{
	int data;
};

class ItemQueue
{
private:
	std::array<DataItem, MAXITEMQUEUE> items;

	// Shared pointers need to made atomic
	std::atomic<size_t> readpointer = { 0 }, writepointer = { 0 };
	bool queuefull;
	bool queueempty;

public:
	ItemQueue()
	{
		queuefull = false;
		queueempty = true;
	}

	inline bool IsQueueEmpty() {
		return queueempty;
	}

	inline bool IsQueueFull() {
		return queuefull;
	}

	/* push - Insert element (public member function ) */
	bool push(const DataItem& element)
	{
		auto oldwritepointer = writepointer.load();
		auto newwritepointer = (oldwritepointer + 1) % MAXITEMQUEUE;
		auto oldreadpointer = readpointer.load();

		if (!(newwritepointer == oldreadpointer))
		{
			items[oldwritepointer] = element;
			
			// Safely update write pointer
			if (!(writepointer.compare_exchange_strong(oldwritepointer, newwritepointer)))
				return false;

			return true;
		}
		else
		{
			return false;
		}
	}

	/* pop - Remove next element (public member function ) */
	bool pop(DataItem& element)
	{
		auto oldreadpointer = readpointer.load();
		auto oldwritepointer = writepointer.load();
		auto newreadpointer = (oldreadpointer + 1) % MAXITEMQUEUE;

		if (!(oldreadpointer == oldwritepointer))
		{
			element = std::move(items[oldreadpointer]);
			while (true)
			{
				if (readpointer.compare_exchange_strong(oldreadpointer, newreadpointer))
				{
					return true;
				}
			}
		}
		else
		{
			return false;
		}
	}
};

ItemQueue testqueue;

void Source_Function(int DataSize)
{
	DataItem di;

	for (int i = 0; i < (DataSize); i++)
	{

		di.data = i;

		while (!testqueue.push(di))
		{
			//this_thread::sleep_for(chrono::milliseconds(50));
		}

		cout << "\nWrite[" << i << "] :" << di.data << endl;

		if ((i % 2) == 0)
		{
			this_thread::sleep_for(chrono::milliseconds(50));
		}
	}
}

void Sink_Function(int DataSize)
{
	DataItem di;

	for (int i = 0; i < DataSize; i++)
	{
		while (!testqueue.pop(di))
		{
			//this_thread::sleep_for(chrono::milliseconds(50));
		}

		cout << "\nData Read[" << i << "] :" << di.data << endl;

		if ((i % 10) == 0)
		{
			this_thread::sleep_for(chrono::milliseconds(5000));
		}
	}
}

int main()
{
	ExecutionTimer timer;

	thread t1(Source_Function, 10000); // t1 starts running
		
	Sink_Function(10000);

	t1.join();



	return 0;
}



