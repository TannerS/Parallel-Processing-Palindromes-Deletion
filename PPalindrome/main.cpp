#include <fstream>
#include <string>
#include <iostream>
#include "mpi.h"
#include <stack>
#include <vector>


// argc = cpu count, argv = file.cpp
int main(int argc, char *argv[])
{
	int namelen = 0;
	int myid, numprocs = 0;
	// processor name
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	//initialize MPI execution environment
	MPI_Init(&argc, &argv);
	//each process get total # of processes
	//the total # of processes specified in mpirun �np n
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	//each process gets its own id
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	// Gets the name of the processor
	MPI_Get_processor_name(processor_name, &namelen);
	// number of processes
	int n = 0;
	// display info
	fprintf(stderr, "process %d on %s\n", myid, processor_name);
	fflush(stderr);
	// declare array to hold char from words plus \0
	char* arr;
	// list to keep track of length of each word
	short* list;
	// size of entire array
	int arr_size = 0;
	// size of the list
	int list_size = 0;


	if (myid == 0)
	{
		// stream to open file
		std::fstream in;
		// vector to dynamically grow as we add strings to it
		// this makes it so we don't need to open file twice since 
		// we would normally open file and count number of words
		// then reopen it to get the actually words to put in an array
		// we just declared based off the size we got the first time
		std::vector<std::string>* words = new std::vector<std::string>();
		// open file as instream
		in.open("Palindromes.txt", std::ios::in);
		// if error opening file
		if (in.fail())
		{
			// display message and close
			std::cout << "Error Opening File" << std::endl;
			return 0;
		}
		// no error while opening file
		else
		{
			// temp string to hold each word
			std::string temp;
			// grab each word from each line
			while (getline(in, temp))
			{
				// put word into vector
				words->push_back(temp);
				// loop each string (word) and get it's length
				for (int i = 0; i < temp.size(); i++)
					//increment size
					arr_size++;
				// increment one last time since we will be adding a 
				// \0 for each word
				arr_size++;
			}
			// done, close file
			in.close();
		}
		// set size depending on word size
		list_size = words->size();
		// allocate list, list should be number of \0
		// since there is one per word, it should be the number of words
		list = new short[list_size];
		// allocate array
		arr = new char[arr_size];
		// put the values into array
		// using a counter
		int counter = 0;
		// loop entier array, while looping each word
		// and put them sequentially into array
		// with null terminator ending each word
		for (int i = 0; i < list_size; i++)
		{
			//mark start of word
			arr[counter] = '\0';
			// put null terminator index into list
			list[i] = counter;
			// incremenet counter
			counter++;
			// loop to get count of the next word
			for (int j = 0; j < words->at(i).size(); j++)
			{
				// get word from vector at i (string is returned)
				// get char at j from string
				arr[counter++] = words->at(i).at(j);
			}
		}
		// free up memory, this object is no longer used
		delete words;
	}


	MPI_Barrier(MPI_COMM_WORLD);
	// broadcast the size of char array and list to other processes
	// they will be used to allocate the needed space per process
	MPI_Bcast(&arr_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&list_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// array and list where both allocated as root, 
	// this time only allocate for the other processes
	if (myid != 0)
	{
		std::cout << "DEBUG 1" << std::endl;
		// allocate list, list should be number of \0
		// since there is one per word, it should be the number of words
		list = new short[list_size];
		// allocate array
		arr = new char[arr_size];
		std::cout << "DEBUG 2" << std::endl;
	}
	// broadcast array of char (basically all the words
	// in a char array where each word ends in \0)
	// also broadcast list of word indexes

	//*************NEEDED OR ELSE SEGMENTATION FAULT
	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Bcast(list, list_size, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(arr, arr_size, MPI_CHAR, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);

	

	/*
	if (myid == 0)
	{
		// ****************************DEBUG******************************
		std::cout << "ROOT" << std::endl;
		for (int i = 0, j = 0; i < arr_size; i++)
		{
			if (list[j] == i)
			{
				//std::cout << std::endl;
				j++;
			}
			//else
				//std::cout << arr[i];
		}
		//std::cout << std::endl;

	}*/

	/*
	if (myid != 0)
	{
		std::cout << "DEBUG 4" << std::endl;
		std::cout << "ARR SIZE : " << arr_size << " LIST SIZE: " << list_size << std::endl;
		std::cout << "DEBUG 5" << std::endl;

		std::cout << "PROCESS OTHER" << std::endl;
		for (int i = 0, j = 0; i < arr_size; i++)
		{
			if (list[j] == i)
			{
				std::cout << std::endl;
				j++;
			}
			else
				std::cout << arr[i];
		}
		std::cout << std::endl;
	}
	*/

	/*
	if (myid != 0)
	{
		std::cout << "PROCESS OTHER" << std::endl;
		for (int i = 0, j = 0; i < list_size; i++)
		{
			std::cout << list[i];
		}
		std::cout <<std::endl;
	}
	*/
	
	MPI_Barrier(MPI_COMM_WORLD);

	markParalindromes(myid, arr_size, list_size, arr, list, numprocs);
	



	if (myid == 0)
	{
		delete[] arr;
		delete[] list;
	}
	else
	{
		// ********************segmentation fault***********************
		//lete[] arr;
		//lete[] list;
	}



	MPI_Finalize();
}


bool markParalindromes(int index, int array_size, int list_size, char* words, short* word_indexes, int num_of_processes)
{
	/*
	[0,6,12,17,22]

	p0p1p0p1p0p1

	\0 h e l l o   ->> 1-5
	\0 w o r l d   ->> 7-11
	\0 t h i s     ->> 13-16
	\0 i s         ->> 18-19
	\0 a           ->> 21-21
	\0 t e s t     ->> 23-26 -end

	*/
	// start of word marker
	int start = -1;
	// end of word / start of next work markers
	int end = -1;


	for (int i = 0; i < list_size; i += num_of_processes)
	{
		// get start of word
		start = word_indexes[i];
		//end of word is start of next word NOT INCLUDING this element
		end = word_indexes[i + 1];
		// check if current word based off index is a plaindrome or not
		// this if statement means it is a pliandrome
		if (checkpalindrome(start, end, words))
		{
			// is palinedrome


		}
		else
		{
			// is not palinedrome

		}

		
	}
}

bool checkpalindrome(int start, int end, char* words)
{
	// for example this would mean start = 0
	// and end = 6
	// word is 1-5 however
	// this will start at 0 but we want the enxt one so start+1
	// since it is < end, the end number, 6 in our exmaple is not included
	// if using cyclic, we do j += processes
	// MAY NEEDMORE TESTE FOR CYCLIC
	for (int j = start + 1, k = end - 1; j < end; j++, k--)
	{
		//  0, 1, 2, 3, 4, 5
		// 0\, h, e, l, l, o
		// 0\ = 0
		// o = 5
		// start = 0
		// start + 1 = 1
		// end = 6
		/*
		// first example
		arr[start] == arr[end-1]
		*/
		if (words[j] != words[k])
			// if any single char does not match, it is not a plaindrome
			return false;
	}
	// if loop completes, it means it IS a plaindrome
	// return true;
	return true;
}












































