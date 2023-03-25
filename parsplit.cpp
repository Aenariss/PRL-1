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
    if (!num_file) exit (1); // error opening file

    uint8_t i;
    while (num_file.read((char*)&i, sizeof(uint8_t)))
	    nums.push_back(i);

    if (nums.size() == 0) return 1; // invalid number of read numbers
    return nums;
}

/* Function to calculate the index of the median */
int getMean(vector<int> numbers) {
    int size = numbers.size();
    int median;
    // even
    if (size % 2 == 0) {
        median = (size/2)-1; // 6 numbers, median is the 3rd, so its index is 2.
    }
    // odd
    else {
        median = floor(size/2) // 5 numbers -> 1 2 3 4 5 -> 3 is the one, aka index 2
    }
    return median;
    // 1 2 3 4 5 6
}

int main(int argc, char *argv[]) {

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* root process loads the numbers and chooses the median (the middle) */
    /* if the number of numbers is even, choose the one closer to the beginning */
    if (rank == 0) {
        auto numbers = readNumbers();

        int mean_index = getMean(numbers);

    }

    int ping_pong_count = 0;
    int partner_rank = 1;




    // Finalize the MPI environment.
    MPI_Finalize();
}
