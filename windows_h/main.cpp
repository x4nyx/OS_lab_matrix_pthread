#include <cstdlib>
#include <chrono>
#include <windows.h>
#include <map>
#include <iostream>
#include <vector>


void makeMatrix(int**& a, int N) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            a[i][j] = rand() % 10;
        }
    }
}


void printMatrix(int** a, int N) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            std::cout << a[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void zeroMatrix(int**& a, int N) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            a[i][j] = 0;
        }
    }
}

void simpleMultiply(int** a, int** b, int**& c, int N) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            c[i][j] = 0;
            for (int k = 0; k < N; ++k) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

void multiplyBlock(int **a, int** b, int**&c, int N, int bSize, int I, int J) {
    for (int i = I; i < min(N, I + bSize); ++i) {
        for (int j = J; j < min(N, J + bSize); ++j) {
            c[i][j] = 0;
            for (int k = 0; k < N; ++k) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

void blockMultiplyDefault(int** a, int** b, int**& c,
                          int N, int bSize) {
    for (int I = 0; I < N; I += bSize) {
        for (int J = 0; J < N; J += bSize) {
            multiplyBlock(a, b ,c ,N, bSize, I, J);
        }
    }
}

struct Data {
    int** a;
    int** b;
    int**& c;
    int N, bSize;
    int I, J;

    Data(int** a, int** b, int**& c, int N, int bSize, int I, int J) : a(a), b(b), c(c), N(N),
                                                                       bSize(bSize), I(I), J(J) {}

};

LPTHREAD_START_ROUTINE *__RPC_CALLEE newRoutine(LPVOID *data) {
    Data* newData = (Data*) data;
    multiplyBlock(newData->a, newData->b, newData->c,
                  newData->N, newData->bSize, newData->I, newData->J);
    ExitThread(0);
}

int blockMultiplyThread(int** a, int** b, int**& c,
                        int N, int bSize) {
    std::vector<HANDLE> threads;
    for (int I = 0; I < N; I += bSize) {
        for (int J = 0; J < N; J += bSize) {
            threads.emplace_back(CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&newRoutine),
                                              new Data(a, b, c, N, bSize, I, J),
                                              0, NULL));
        }
    }


    for (HANDLE &thread: threads) {
        WaitForSingleObject(thread, INFINITE);
    }

    return (int)threads.size();
}


int** calculateTime(int** a, int** b, int** c, int N) {
    int** time = new int*[N];
    for(int i = 0 ; i < N; i++) {
        time[i] = new int[3];
    }
    for (int i = 0; i < N; i++) {
        auto start1 = std::chrono::system_clock::now();
        blockMultiplyDefault(a, b, c, N, i + 1);
        auto end1 = std::chrono::system_clock::now();
        auto durDefault = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

        auto start2 = std::chrono::system_clock::now();
        int numOfThreads = blockMultiplyThread(a, b, c, N, i + 1);
        auto end2 = std::chrono::system_clock::now();
        auto durThread = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
        time[i][0] = durDefault;
        time[i][1] = numOfThreads;
        time[i][2] = durThread;
    }
    return time;
}

void printTime(int** time, int N) {
    for(int i = 0; i < N; i++) {
        std::cout << i + 1 << ") Size of block: " << i + 1
                  << "; Duration of default multiply: " << time[i][0] << " ms;\n"
                  << "Number of threads: " << time[i][1]
                  << "; Duration of thread multiply: " << time[i][2] << " ms;\n\n";
    }
}

int calculateSimpleTime(int** a, int** b, int** c, int N) {
    auto start = std::chrono::system_clock::now();
    simpleMultiply(a, b, c, N);
    auto end = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

void printSimpleTime(int** a, int** b, int** c, int N) {
    std::cout << "Duration of simple multiply: " << calculateSimpleTime(a, b, c, N) << " ms;\n\n";
}

int main() {
    int N = 50;// Размер матрицы

    int** a = new int*[N];
    int** b = new int*[N];
    int** c = new int*[N];

    for(int i = 0; i < N; i++) {
        a[i] = new int[N];
        b[i] = new int[N];
        c[i] = new int[N];
    }

    std::srand(time(0));

    makeMatrix(a, N);
    makeMatrix(b, N);
    zeroMatrix(c, N);

    //printMatrix(a, N);
    //printMatrix(b, N);

    //simpleMultiply(a, b, c, N);
    //printMatrix(c, N);

    printSimpleTime(a, b, c, N);
    printTime(calculateTime(a, b ,c, N), N);

    return 0;
}