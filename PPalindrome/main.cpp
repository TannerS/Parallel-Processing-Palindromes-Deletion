#include <fstream>
#include <string>
#include <iostream>
#include "mpi.h"
#include <stack>
#include <vector>
#include <locale> 
#include <cstdlib>
// #include <ctype.h>
//#include <ctype>

bool checkpalindrome(int, int, char*);
char* markParalindromes(int, int, int, char*, short*, int, int&);

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
	// new list of words that are not palindromes
	char* new_words;
	// size of new array of words eahc process will
	// have inorder to send back to root after finding
	// all none plaindrome words
	int new_size = 0;
	// this will be the total size of non-palidrome words
	// which will be recieved from each process
	int total_size = 0;
	// temp vector to hold arrays in file
	std::vector<std::string>* words;
	// root does
	if (myid == 0)
	{
		// stream to open file
		std::fstream in;
		// vector to dynamically grow as we add strings to it
		// this makes it so we don't need to open file twice since 
		// we would normally open file and count number of words
		// then reopen it to get the actually words to put in an array
		// we just declared based off the size we got the first time
		words = new std::vector<std::string>();
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
		// we added one since later on in the program
		// we use the next index to mark where the loop stops
		// without one at end, there is no way to mark the end
		// and last word never gets processed
		list_size++;
	}
	// barrier
	MPI_Barrier(MPI_COMM_WORLD);
	// broadcast the size of char array and list to other processes
	// they will be used to allocate the needed space per process
	MPI_Bcast(&arr_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// barrier
	MPI_Barrier(MPI_COMM_WORLD);
	// broadcats list size
	MPI_Bcast(&list_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// barrier
	MPI_Barrier(MPI_COMM_WORLD);
	// allocate list, list should be number of \0
	// since there is one per word, it should be the number of words
	list = new short[list_size];
	// barrier
	MPI_Barrier(MPI_COMM_WORLD);
	// allocate array
	arr = new char[arr_size];
	// barrier
	MPI_Barrier(MPI_COMM_WORLD);
	// root does this
	if(myid ==0)
	{
		// put the values into array
		// using a counter
		int counter = 0;
		// loop entier array, while looping each word
		// and put them sequentially into array
		// with null terminator ending each word
		// we do list_size-1 since list_size is increased by 1
		// to fix an earlier problem where we need to mark
		// last element in list to be able to end it
		// without it, it crashes, not sure why
		for (int i = 0; i < list_size - 1; i++)
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
		// make last element to stop loops later in program
		list[list_size - 1] = counter;
		// free up memory, this object is no longer used
		delete words;
	}
	// broadcast array of char (basically all the words
	// in a char array where each word ends in \0)
	// also broadcast list of word indexes
	MPI_Barrier(MPI_COMM_WORLD);
	// send list of indexes to all processes
	MPI_Bcast(list, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// send array ofwords to processes
	MPI_Bcast(arr, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
	// run function for each process to create a new list of non-palindromes
	// this is using cyclic partiioning


/*
	new_words = markParalindromes(myid, arr_size, list_size, arr, list, numprocs, new_size);
	// get number of non-palindromes from each process and sum it up into new var
	// called total_size, this will be used as a way to keep track oh many chars
	// words we will be displaying in root
	MPI_Reduce(&new_size, &total_size, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);


	std::cout << "TOTAL SIZE: " << total_size << std::endl;



	char* temp;
	int* size_arr;
	int temp_size;



	// have root allocate array for final results
	if (myid == 0)
	{
		temp = new char[total_size];
		size_arr = new int[numprocs];
	}
	// barrier
	MPI_Barrier(MPI_COMM_WORLD);
	// all non root processes do this
	if(myid != 0)
	{
		// have all processes send thier arra y size
		MPI_Send(&new_size, 1, MPI_INT, 0, myid, MPI_COMM_WORLD);
	}
	else
	{
		// put root size into arry
		size_arr[0] = new_size;
		// loop other processes sizes
		for (int i = 1; i < numprocs; i++)
		{
			// receive sizes from process i
			MPI_Recv(&temp_size, 1, MPI_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// put size into array
			size_arr[i] = temp_size;
		}
	}
	// barrier
	MPI_Barrier(MPI_COMM_WORLD);
	// all non root processes do this
	if (myid != 0)
	{
		// have all processes send thier arra y size
		MPI_Send(new_words, new_size, MPI_CHAR, 0, myid, MPI_COMM_WORLD);
	}
	else
	{
		// temp counter to keep track of results array
		int counter = 0;
		// loop and put current words into result array
		for (int i = 0; i < new_size; i++)
			temp[counter++] = new_words[i];
		// loop other processes sizes
		for (int i = 1; i < numprocs; i++)
		{
			// receive sizes from process i
			MPI_Recv(&arr, new_size, MPI_CHAR, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// words into result array
			for (int j = 0; j < size_arr[i]; j++)
			{
				temp[counter++] = arr[j];
			}
		}
	}
	// barrier
	MPI_Barrier(MPI_COMM_WORLD);
	// clean up and display results


	*/


	if (myid == 0)
	{
		
		// display results
		for (int i = 0; i < total_size; i++)
		{
			//if (temp[i] == '\0')
				//std::cout << std::endl;
			//else
				//std::cout << temp[i];
		}
		
		// clean up
		if(arr != NULL)
			delete[] arr;
		if (list != NULL)
			delete[] list;
		//if (new_words != NULL)
		//	delete[] new_words;
		//if (temp != NULL)
			//delete[] temp;
	}


	
	MPI_Finalize();
}

// es(myid, arr_size, list_size, arr, list, numprocs, new_size);

char* markParalindromes(int index, int array_size, int list_size, char* words, short* word_indexes, int num_of_processes, int& new_size)
{

	std::cout << "*************PROCESS: " << index << "*******************" << std::endl;
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

	// create array of max amount of words
	// this is worst case where it assumes 
	// all words are NOT palindrome
	char* new_words = new char[array_size];
	// start of word marker
	int start = -1;
	// end of word/start of next work markers
	int end = -1;
	// loop list of when each word starts using cyclic partioning
	// loop will start at the value from myid and increment by the number 
	// of processors doing the job
	for (int i = index, k = 0; i < list_size; i += num_of_processes)
	{
		// at index of list, getstart of word
		start = word_indexes[i];
		// end of word is start of next word NOT INCLUDING this element
		end = word_indexes[i + 1];
		// check if current word based off index is a plaindrome or not
		// this is for it not being a palindrome

		//

		if (!checkpalindrome(start, end, words))
		{
			// loop this range of the word and add it to new array
			for (int j = start; j < end; j++)
			{
				// increase the number of words
				new_size++;
				// put char at pos j into new array at pos k
				new_words[k++] = words[j];
				//****assume \0 also gets copied*****
			}
		}
	}

	return new_words;
}

bool checkpalindrome(int start, int end, char* words)
{

	/*
	for (int i = start; i < end; i++)
	{
		if (words[i] == '\0')
			std::cout << std::endl;
		else
			std::cout << words[i];


	}
	
	*/
	
	
	
	// for example this would mean start = 0
	// and end = 6
	// word is 1-5 however
	// this will start at 0 but we want the enxt one so start+1
	// since it is < end, the end number, 6 in our exmaple is not included
	// if using cyclic, we do j += processes
	for (int j = start + 1, k = end - 1; j < end; j++, k--)
	{
		// if there is a space at j, increase k
		// so next loop you can have same letters, except
		// skipped over the space
		if (isspace(words[j]))
			k++;
		// same as above but opposite
		else if (isspace(words[k]))
			j--;
		// if no spaces
		else
		{
			// check each lowercase of each char to see if they match
			if ((char)tolower(words[j]) != (char)tolower(words[k]))
				// if any single char does not match, it is not a plaindrome
				return false;
		}
	}

	// if loop completes, it means it IS a plaindrome
	return true;
}

