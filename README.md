NOTE: When you run the program with my_mpi, since it is not printing anything, it will look like it is not doing anything
but it will put all the average and std deviations in the output.txt file when the program ends.

NOTE: The configuration in my_prun is for 4 nodes and 8 processes. If it is changed, we would have to make changes in my_prun as well.

Implemented MPI library runs approximately 100 times slower than original MPI library
in terms of microseconds (from ~20 microseconds to ~2000 microseconds).

The other results are same. The first RTT is more than subsequent RTTs.

Output file generated -> output.txt
