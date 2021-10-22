#include <mpich/mpi.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

const int MAX_STRING = 100;

// get_current_timestamp returns timestamp in microseconds.
unsigned long get_current_timestamp_microseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000000 + tv.tv_usec;
}

int main() {
    int comm_sz;
    int my_rank;
    // package_id | send_timestamp
    unsigned long buf[2];
    struct MPI_Status st;

    // Send 5 packages.
    int total_packages = 5;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == 0) {
        for (int package_id = 1; package_id <= total_packages; package_id++) {
            buf[0] = package_id;
            buf[1] = get_current_timestamp_microseconds();
            MPI_Send(buf, 2, MPI_UNSIGNED_LONG, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(buf, 2, MPI_UNSIGNED_LONG, 1, 0, MPI_COMM_WORLD, &st);
            unsigned long rtt = get_current_timestamp_microseconds() - buf[1];
            printf("Task 0: package_id = %ld, rtt = %ldus\n", buf[0], rtt);
        }
    } else {
        for (int package_id = 1; package_id <= total_packages; package_id++) {
            MPI_Recv(buf, 2, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD, &st);
            MPI_Send(buf, 2, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}