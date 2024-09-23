#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 2000
/* number of rows and columns in matrix */

int main(int argc, char **argv)
{
    int numtasks, taskid, numworkers, source, dest, rows, offset, i, j, k;
    double t1, t2;
    MPI_Status status;

    double a[N][N], b[N][N], c[N][N]; // Only the master will use the full matrices

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    numworkers = numtasks - 1;

    /*---------------------------- master ----------------------------*/
    if (taskid == 0) {
        t1 = MPI_Wtime();

        // Initialize matrices a and b
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                a[i][j] = 1.0;
                b[i][j] = 2.0;
            }
        }

        /* send matrix data to the worker tasks */
        rows = N / numworkers;
        offset = 0;

        for (dest = 1; dest <= numworkers; dest++) {
            int rows_to_send = rows;
            if (dest <= N % numworkers) {
                rows_to_send += 1;
            }

            MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
            MPI_Send(&rows_to_send, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
            MPI_Send(&a[offset][0], rows_to_send * N, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
            MPI_Send(&b[0][0], N * N, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);

            offset += rows_to_send;
        }

        /* wait for results from all worker tasks */
        for (i = 1; i <= numworkers; i++) {
            int rows_received;
            MPI_Recv(&offset, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(&rows_received, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(&c[offset][0], rows_received * N, MPI_DOUBLE, i, 2, MPI_COMM_WORLD, &status);
        }

        t2 = MPI_Wtime();

        // Optionally print a small part of the result matrix
        printf("Sample result (c[0][0] to c[4][4]):\n");
        for (i = 0; i < 5; i++) {
            for (j = 0; j < 5; j++)
                printf("%6.2f ", c[i][j]);
            printf("\n");
        }

        printf("Elapsed time is %f seconds\n", t2 - t1);
    }

    /*---------------------------- worker----------------------------*/
    if (taskid > 0) {
        source = 0;
        MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);

        // Allocate memory for matrices a, b, and c
        double *a = (double *)malloc(rows * N * sizeof(double));
        double *b = (double *)malloc(N * N * sizeof(double));
        double *c = (double *)malloc(rows * N * sizeof(double));

        MPI_Recv(a, rows * N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(b, N * N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);

        /* Matrix multiplication */
        for (i = 0; i < rows; i++) {
            for (k = 0; k < N; k++) {
                c[i * N + k] = 0.0;
                for (j = 0; j < N; j++) {
                    c[i * N + k] += a[i * N + j] * b[j * N + k];
                }
            }
        }

        MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(c, rows * N, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);

        // Free allocated memory
        free(a);
        free(b);
        free(c);
    }

    MPI_Finalize();
    return 0;
}
