/**
 * PRL 2023 Projekt 1
 * Autor: Vojtech Fiala <xfiala61>
*/

#include <mpi.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <math.h>
#include <iostream>

using namespace std;

/* Read the 1 byte large numbers from the input file `numbers` */
vector<uint8_t> readNumbers() {
    ifstream num_file ("numbers", ios::binary);
    vector<uint8_t> nums;
    if (!num_file) exit(1); // error opening file

    uint8_t i;
    while (num_file.read((char*)&i, sizeof(uint8_t)))
	    nums.push_back(i);

    return nums;
}

/* Function to calculate the index of the median */
int getMed(vector<uint8_t> numbers) {
    int size = numbers.size();
    int median;
    // even
    if (size % 2 == 0) {
        median = (size/2)-1; // 6 numbers, median is the 3rd, so its index is 2.
    }
    // odd
    else {
        median = floor(size/2); // 5 numbers -> 1 2 3 4 5 -> 3 is the one, aka index 2
    }
    return median;
}

/* Function to print a vector in a certain way, very pretty */
void printVecNoBreak(vector<uint8_t> vec) {
    auto size = vec.size();
    // if the vector is empty, dont print anything
    if (size != 0) {
        // print all members but the last one
        for (int i = 0; i < size-1; i++) { 
            cout << static_cast<int>(vec[i]) << ", ";
        }
        //print the last one
        cout << static_cast<int>(vec[size-1]);
    }
}

/* Function to print 3 vectors, intended for L, E and G vectors */
void print3Vecs(vector<uint8_t> L, vector<uint8_t> E, vector<uint8_t> G) {
    cout << "L: [";
    printVecNoBreak(L);
    cout << "]\n";

    cout << "E: [";
    printVecNoBreak(E);
    cout << "]\n";

    cout << "G: [";
    printVecNoBreak(G);
    cout << "]\n";
}

/* Function to calculate the sum of a vector */
int vectorSum(vector<int> vec) {
    int sum = 0;
    for (auto x : vec) {
        sum += x;
    }
    return sum;
}

int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    int median_val;
    int each_cpu_numbers;
    vector<uint8_t> numbers;
    /* root process loads the numbers and chooses the median (the middle) and broadcast it */
    if (rank == 0) {
        numbers = readNumbers();

        if (numbers.size() == 0) MPI_Abort(MPI_COMM_WORLD, 1); // error, invalid number of numbers
        if (numbers.size() % size != 0) MPI_Abort(MPI_COMM_WORLD, 1); // number of nubmers cant be evenl;y divided by number of processes

        auto median_index = getMed(numbers);
        median_val = numbers[median_index];

        each_cpu_numbers = numbers.size() / size; // divide the number of numbers that will be assigned to each process
    }

    MPI_Bcast(&median_val, 1, MPI_INT, 0, MPI_COMM_WORLD); // broadcast the mean from the root to others
    MPI_Bcast(&each_cpu_numbers, 1, MPI_INT, 0, MPI_COMM_WORLD); // broadcast size of subarray for each core

    vector<uint8_t> part_of_ns;
    part_of_ns.resize(each_cpu_numbers); // resize vector for each processor to save its part into

    // send each processor its number input
    // the number of input is without remain divisible by n of processors
    MPI_Scatter(numbers.data(), each_cpu_numbers, MPI_UINT8_T, part_of_ns.data(), each_cpu_numbers, MPI_UINT8_T, 0, MPI_COMM_WORLD); // scatter the nubmers
    
    // create L, E, G vectors
    vector<uint8_t> L, E, G;

    // go through given numbers and depending on their value, put them into one of the three vectors
    for (auto x : part_of_ns) {
        if      (x > median_val) {
            G.push_back(x);
        }
        else if (x < median_val) {
            L.push_back(x);
        }
        // ==
        else {
            E.push_back(x);
        }
    }

    // gather all sizes pf each vector from the processes
    vector<int> sizes_L, sizes_E, sizes_G;
    // get the actual sizes for gather
    int sizeL = L.size();
    int sizeE = E.size();
    int sizeG = G.size();
    if (rank == 0) {
        sizes_L.resize(size); // vector of results has size of number of processors that parsed the info
        sizes_E.resize(size); 
        sizes_G.resize(size);
    }
    // gather SIZES
    MPI_Gather(&sizeL, 1, MPI_INT, sizes_L.data(), 1, MPI_INT, 0, MPI_COMM_WORLD); // root receives only 1 nubmer - the size
    MPI_Gather(&sizeE, 1, MPI_INT, sizes_E.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&sizeG, 1, MPI_INT, sizes_G.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    int total_size_L = 0;
    int total_size_E = 0;
    int total_size_G = 0;

    if (rank == 0) {
        total_size_L = vectorSum(sizes_L);
        total_size_E = vectorSum(sizes_E);
        total_size_G = vectorSum(sizes_G);
    }

    vector<uint8_t> total_L, total_E, total_G;
    int *displsL = NULL;
    int *displsE = NULL;
    int *displsG = NULL;
    // figure final positions in combined L, E, G
    if (rank == 0) {
        displsL = (int*) malloc(size * sizeof(int)); // calculate displacement relative to position
        displsE = (int*) malloc(size * sizeof(int));
        displsG = (int*) malloc(size * sizeof(int));
        displsL[0] = 0;
        displsE[0] = 0;
        displsG[0] = 0;

        for (int i = 1; i < size; i++) {
            displsL[i] = displsL[i-1] + sizes_L[i-1];
            displsE[i] = displsE[i-1] + sizes_E[i-1];
            displsG[i] = displsG[i-1] + sizes_G[i-1];
        }

        total_L.resize(total_size_L); // i already ahve the size
        total_E.resize(total_size_E); // i already ahve the size
        total_G.resize(total_size_G); // i already ahve the size
    }

    MPI_Gatherv(L.data(), L.size(), MPI_UINT8_T, total_L.data(), sizes_L.data(), displsL, MPI_UINT8_T, 0, MPI_COMM_WORLD); // displacement does the index stuff
    MPI_Gatherv(E.data(), E.size(), MPI_UINT8_T, total_E.data(), sizes_E.data(), displsE, MPI_UINT8_T, 0, MPI_COMM_WORLD); // displacement does the index stuff
    MPI_Gatherv(G.data(), G.size(), MPI_UINT8_T, total_G.data(), sizes_G.data(), displsG, MPI_UINT8_T, 0, MPI_COMM_WORLD); // displacement does the index stuff

    if (rank == 0) {

        // final printing
        print3Vecs(total_L,total_E,total_G);

        free(displsL);
        free(displsE);
        free(displsG);
    }
    
    MPI_Finalize();
}
