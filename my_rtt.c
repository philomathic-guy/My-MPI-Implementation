/*
Single Author info:
rtnaik Rohit Naik
Group info:
nphabia Niklesh Phabiani
rtnaik	Rohit Naik
anjain2 Akshay Narendra Jain
*/

#include <stdio.h>
#include <stdlib.h>
// Note that we have my_mpi here.
#include "my_mpi.h"
#include <time.h>
#include <math.h>

/* This is the root process */
#define  ROOT       0

int main (int argc, char *argv[])
{
    /* process information */
    //int   numproc, rank, len;

    int rank = atoi(argv[2]);

    /* current process hostname */
    //char  hostname[MPI_MAX_PROCESSOR_NAME];

    /* initialize MPI */
    MPI_Init(&argc, &argv);

    /* get the number of procs in the comm */
    //MPI_Comm_size(MPI_COMM_WORLD, &numproc);

    /* get my rank in the comm */
    //MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* get some information about the host I'm running on */
    //MPI_Get_processor_name(hostname, &len);

    // Loop indices.
    int i;
    int j;
    // For getting 10 values of RTT.
    int count;
    // For looping through different sizes.
    int currSize;
    MPI_Status status;

    // Max size of messages.
    int size = 2097152;

    double times[10];
    double averagetime[17];
    double stddeviation;
    double stddeviations[17];
    double variance;
    double time;
    double sum1;
    // Only to be created for node with rank 0.
    double *allPairsAvgTimes;
    double *allPairsStdDeviations;

    if (rank == 0) {
        // All message sizes for one pair and then for the second pair and then the third and so on.
        allPairsAvgTimes = (double*) malloc(4*17 * sizeof(double));
        // All message sizes for one pair and then for the second pair and then the third and so on.
        allPairsStdDeviations = (double*) malloc(4*17 * sizeof(double));
    }
    int currIdx = 0;
    for(currSize=32; currSize<=size; currSize *= 2) {
        char msg[currSize];
        double totaltime = 0;
        for(count=0; count<11; count++) {
            // If the rank of the current process is even, then do all the calculations for communications between rank and rank+1.
            if (rank % 2 == 0) {
                struct timeval starttime, endtime;
                gettimeofday(&starttime, NULL);
                // Message to be sent.
                sprintf(msg, "Hello %d", i+1);
                MPI_Send(msg, currSize, MPI_CHAR, rank+1, 0, MPI_COMM_WORLD);
                MPI_Recv(msg, currSize, MPI_CHAR, rank+1, 0, MPI_COMM_WORLD, &status);
                gettimeofday(&endtime, NULL);
                // CHECK how to use gettimeofday().
                if (count == 0)
                    continue;
                times[count] = (endtime.tv_usec - starttime.tv_usec);
                totaltime += times[count];
            }
            else {
                MPI_Recv(msg, currSize, MPI_CHAR, rank-1, 0, MPI_COMM_WORLD, &status);
                MPI_Send(msg, currSize, MPI_CHAR, rank-1, 0, MPI_COMM_WORLD);
            }
        }
        if (rank % 2 == 0) {
            averagetime[currIdx] = (double)totaltime/10;
            sum1 = 0;
            for (i=0; i<10; i++) {
                sum1 = sum1 + pow((times[i] - (double)totaltime/10), 2);
            }
            variance = sum1/(double)10;
            // Store standard deviation.
            stddeviation = sqrt(variance);
            stddeviations[currIdx] = stddeviation;
        }
        currIdx++;
    }
    MPI_Barrier();
    //printf("Came out finally in rank %d\n", rank);
    //fflush(stdout);
    if (rank != 0 && rank % 2 == 0) {
        // Send average time for all message sizes for (rank, rank+1) pair.
        MPI_Send(averagetime, 17, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        // Send the standard deviation for all message sizes for (rank, rank+1) pair.
        MPI_Send(stddeviations, 17, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    if (rank == 0) {
        // For process 0, just add it to the final array.
        for (i=0; i<17; i++) {
            allPairsAvgTimes[i] = averagetime[i];
            allPairsStdDeviations[i] = stddeviations[i];
        }
        // For other processes, add to the final array using MPI_Recv.
        MPI_Gather(allPairsAvgTimes, allPairsStdDeviations, 17, MPI_DOUBLE, 2, 0, MPI_COMM_WORLD, &status);
        MPI_Gather(allPairsAvgTimes, allPairsStdDeviations, 17, MPI_DOUBLE, 2, 1, MPI_COMM_WORLD, &status);
        MPI_Gather(allPairsAvgTimes, allPairsStdDeviations, 17, MPI_DOUBLE, 4, 0, MPI_COMM_WORLD, &status);
	      MPI_Gather(allPairsAvgTimes, allPairsStdDeviations, 17, MPI_DOUBLE, 4, 1, MPI_COMM_WORLD, &status);
        MPI_Gather(allPairsAvgTimes, allPairsStdDeviations, 17, MPI_DOUBLE, 6, 0, MPI_COMM_WORLD, &status);
        MPI_Gather(allPairsAvgTimes, allPairsStdDeviations, 17, MPI_DOUBLE, 6, 1, MPI_COMM_WORLD, &status);

        // Add all results to a file for ease of creating a plot.
        char filename[1024];
        sprintf(filename, "output.txt");
        FILE *fp = fopen(filename, "w");
        // For each process.
        for(i = 0; i < 4; i++)
        {
            // Print all average times first.
            for (j = i*17; j < i*17 + 17; j++) {
                fprintf(fp, "%f ", allPairsAvgTimes[j]);
            }
            fprintf(fp, "\n\n");
            // Print all SDs second.
            for (j = i*17; j < i*17 + 17; j++) {
                fprintf(fp, "%f ", allPairsStdDeviations[j]);
            }
            fprintf(fp, "\n\n");
        }
        fclose(fp);
    }
    MPI_Finalize();
}
