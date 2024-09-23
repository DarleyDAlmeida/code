#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mpi.h>

int main ( int argc, char *argv[] );
int prime_number ( int n, int id, int p );
void timestamp ( );

int main ( int argc, char *argv[] )
{
    int id;
    int ierr;
    int n;
    int n_factor;
    int n_hi;
    int n_lo;
    int p;
    int primes;
    int primes_part;
    double wtime;

    n_lo = 1;
    n_hi = 1048576;
    n_factor = 2;

    // Inicialização do MPI.
    ierr = MPI_Init ( &argc, &argv );
    if ( ierr != MPI_SUCCESS )
    {
        std::cerr << "MPI_Init retornou código de erro " << ierr << std::endl;
        exit ( 1 );
    }

    // Obter o número de processos.
    ierr = MPI_Comm_size ( MPI_COMM_WORLD, &p );
    if ( ierr != MPI_SUCCESS )
    {
        std::cerr << "MPI_Comm_size retornou código de erro " << ierr << std::endl;
        MPI_Abort(MPI_COMM_WORLD, ierr);
    }

    // Determinar o rank deste processo.
    ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );
    if ( ierr != MPI_SUCCESS )
    {
        std::cerr << "MPI_Comm_rank retornou código de erro " << ierr << std::endl;
        MPI_Abort(MPI_COMM_WORLD, ierr);
    }

    if ( id == 0 )
    {
        timestamp ( );
        std::cout << "\n";
        std::cout << "PRIME_MPI\n";
        std::cout << "  Versão em C++/MPI\n";
        std::cout << "\n";
        std::cout << "  Um programa exemplo em MPI para contar o número de primos.\n";
        std::cout << "  O número de processos é " << p << "\n";
        std::cout << "\n";
        std::cout << "     N           Pi          Tempo\n";
        std::cout << "\n";
    }

    n = n_lo;
    while ( n <= n_hi )
    {
        if ( id == 0 )
        {
            wtime = MPI_Wtime ( );
        }

        ierr = MPI_Bcast ( &n, 1, MPI_INT, 0, MPI_COMM_WORLD );
        if ( ierr != MPI_SUCCESS )
        {
            std::cerr << "MPI_Bcast retornou código de erro " << ierr << std::endl;
            MPI_Abort(MPI_COMM_WORLD, ierr);
        }

        primes_part = prime_number ( n, id, p );

        ierr = MPI_Reduce ( &primes_part, &primes, 1, MPI_INT, MPI_SUM, 0,
            MPI_COMM_WORLD );
        if ( ierr != MPI_SUCCESS )
        {
            std::cerr << "MPI_Reduce retornou código de erro " << ierr << std::endl;
            MPI_Abort(MPI_COMM_WORLD, ierr);
        }

        if ( id == 0 )
        {
            wtime = MPI_Wtime ( ) - wtime;
            std::cout << "  " << std::setw(10) << n
                      << "  " << std::setw(10) << primes
                      << "  " << std::setw(12) << wtime << "\n";
        }

        n = n * n_factor;
    }

    // Finalização do MPI.
    ierr = MPI_Finalize ( );
    if ( ierr != MPI_SUCCESS )
    {
        std::cerr << "MPI_Finalize retornou código de erro " << ierr << std::endl;
        exit ( 1 );
    }

    // Encerramento.
    if ( id == 0 )
    {
        std::cout << "\n";
        std::cout << "PRIME_MPI - Processo mestre:\n";
        std::cout << "  Execução finalizada normalmente.\n";
        std::cout << "\n";
        timestamp ( );
    }
    return 0;
}

int prime_number ( int n, int id, int p )
{
    int i;
    int j;
    int prime;
    int total = 0;

    for ( i = 2 + id; i <= n; i += p )
    {
        prime = 1;
        int limit = static_cast<int>(std::sqrt(i));
        for ( j = 2; j <= limit; j++ )
        {
            if ( ( i % j ) == 0 )
            {
                prime = 0;
                break;
            }
        }
        total += prime;
    }
    return total;
}

void timestamp()
{
    std::time_t t = std::time(nullptr);
    std::cout << std::put_time(std::localtime(&t), "%d %B %Y %I:%M:%S %p") << "\n";
}
