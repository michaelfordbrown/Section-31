// SourceSinkConVar.cpp : Defines the entry point for the console application.
//
/*
* Given a source producing data at a given rate, probably irregular, sometimes blocking and a sink consuming the data,
* i.e. doing processing on the data.
*
* Given that the source and sink run at different rates,
* i.e. the data coming in can come in large or small bursts, there might be periods of little or no data,
* of long runs of data,
*
* And the sink can process the data at different rates (some data takes longer),
*
* and we might want parallelism because we can vectorise the processing.
*
* How could you create a solution that minimizes coupling and waiting between the source and the sink?
* i.e. if the source blocks on a socket waiting for input the sink should be able to process any outstanding data,
* and conversely the source should be able to read data from the socket whenever available without being held
* up by the sink (processing might be lengthy, for example).
*/
#include "stdafx.h"

#include<iostream>
#include<fstream>
#include <thread>
#include<string>
#include<mutex>
#include<vector>

#define MAXITEMQUEUE 25

using namespace std;

mutex mu;
condition_variable cond;

struct DataItem {
	int data;
};

class ItemQueue
{
private:
	DataItem dataitems[MAXITEMQUEUE];
	int sinkreadpointer;
	int sourcewritepointer;
	int sourcesinkreadpointer;
	bool queuefull;
	bool queueempty;

public:
	ItemQueue()
	{
		sinkreadpointer = 0;
		sourcewritepointer = 0;
		sourcesinkreadpointer = 0;
		queuefull = false;
		queueempty = true;
	};

	inline bool IsQueueEmpty() {
		return queueempty;
	}

	inline bool IsQueueFull() {
		return queuefull;
	}

	bool WriteData(DataItem data)
	{
		if (!queuefull)
		{
			dataitems[sourcewritepointer] = data;
			if (IsQueueEmpty())
			{
				queueempty = false;
			}

			sourcewritepointer = (sourcewritepointer + 1) % MAXITEMQUEUE;
			if (sourcewritepointer == sinkreadpointer)
			{
				queuefull = true;
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	bool ReadData(DataItem& data)
	{
		if (!queueempty)
		{
			data = dataitems[sinkreadpointer];

			if (IsQueueFull())
			{
				queuefull = false;
			}

			sinkreadpointer = (sinkreadpointer + 1) % MAXITEMQUEUE;
			if (sinkreadpointer == sourcewritepointer)
			{
				queueempty = true;
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	int  SourceRead()
	{
		int data;
		data = dataitems[sourcesinkreadpointer].data;
		sourcesinkreadpointer = (sourcesinkreadpointer + 1) % MAXITEMQUEUE;
		return data;
	}

};

ItemQueue items;

void Source_Function(int DataSize)
{
	DataItem di;
	cout << "\nSource: Thread Started\n";
	/*
	* loop for DataSize, writing items from queue
	*/

	for (int i = 0; i < (DataSize); i++)
	{
		di.data = i;

		// If the queue is full wait Sink to read data. 
		// Meanwhile complete own data processing
		while (items.IsQueueFull())
		{
			cout << "\nSource Data Read " << items.SourceRead() << endl << endl;
			this_thread::sleep_for(chrono::milliseconds(500));
		}

		unique_lock<mutex> locker(mu);  // ENTER CRITICAL SECTION, Global Lock
		if (!items.WriteData(di)) // Write data item to queue and update write pointer and flags
		{
			cout << "\nSource WriteData Warning!\n";
		}
		else
		{
			cout << "Writing Item No. " << i << "\t";
		}
		locker.unlock(); // EXIT CRITICAL SECTION, Global Unlock
	}
	cout << "\nSource: Thread Completed\n\n";
}

void Sink_Function(int DataSize)
{
	DataItem di;

	cout << "\nSink: Thread Started\n";

	/*
	* loop for DataSize, reading items from queue
	*/
	for (int i = 0; i < DataSize; i++)
	{
		while (items.IsQueueEmpty())
		{
			cout << "\nSink: Processing Data\n\n";

			this_thread::sleep_for(chrono::milliseconds(500));
		}

		unique_lock<mutex> locker(mu); // ENTER CRITICAL SECTION, Global Lock
		if (items.ReadData(di)) // Read data item from queue and update read pointer and flags
		{
			cout << "Reading Item: " << di.data << "\t";
		}
		else
		{
			cout << "\nSink Warning\n";
		}

		locker.unlock(); // EXIT CRITICAL SECTION, Global Unlock
	}
	cout << "\nSink: Thread Completed\n\n";
}

int main()
{
	/*
	* Example multi-thread code that transfers integer data from Source to Sink.
	*/

	thread t1(Source_Function, 1000); // Start second thread (main first) to write data into queue

	Sink_Function(1000); // Read data from the queue in multi-thread fashion

	t1.join(); // Force main thread to wait until t1 thread is completed

	return 0;
}

