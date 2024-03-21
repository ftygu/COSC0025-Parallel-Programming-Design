#include <iostream>
#include <sys/time.h> // 包含获取时间的函数
using namespace std;

int* GenerateDimensions(int start, int end, int step, int& arraySize) {
    arraySize = (end - start) / step + 1;
    int* dimensions = new int[arraySize];
    for(int i = 0; i < arraySize; i++) {
        dimensions[i] = start + i * step;
    }
    return dimensions;
}

void Initialize(int* LinearArray, int** TwoDArray, int Dimension) {
    for(int idx = 0; idx < Dimension; idx++) {
        LinearArray[idx] = idx;
        for(int j = 0; j < Dimension; j++) {
            TwoDArray[idx][j] = idx + j;
        }
    }
}

// 使用gettimeofday替代QueryPerformanceFrequency和QueryPerformanceCounter
long long GetCurrentMicroseconds() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000000 + time.tv_usec;
}

void StandardApproach(int* LinearArray, int** TwoDArray, int* AccumulatedSums, int Dimension) {
    long long startTime = GetCurrentMicroseconds();
    for(int i = 0; i < Dimension; i++) {
        AccumulatedSums[i] = 0;
        for(int j = 0; j < Dimension; j++) {
            AccumulatedSums[i] += LinearArray[j] * TwoDArray[j][i];
        }
    }
    long long endTime = GetCurrentMicroseconds();
    cout << "Standard (" << Dimension << "): " << (endTime - startTime) / 1000.0 << "ms" << endl;
}

void RefinedStrategy(int* LinearArray, int** TwoDArray, int* AccumulatedSums, int Dimension) {
    long long startTime = GetCurrentMicroseconds();
    for(int i = 0; i < Dimension; i++) {
        AccumulatedSums[i] = 0;
        for(int j = 0; j < Dimension; j++) {
            AccumulatedSums[i] += LinearArray[j] * TwoDArray[j][i];
        }
    }
    long long endTime = GetCurrentMicroseconds();
    cout << "Refined (" << Dimension << "): " << (endTime - startTime) / 1000.0 << "ms" << endl;
}

void LoopUnfolding(int* LinearArray, int** TwoDArray, int* AccumulatedSums, int Dimension) {
    long long startTime = GetCurrentMicroseconds();
    for(int i = 0; i < Dimension; i++) {
        AccumulatedSums[i] = 0;
        for(int j = 0; j < Dimension; j += 10) {
            for(int k = 0; k < min(10, Dimension - j); k++) {
                AccumulatedSums[i] += LinearArray[j+k] * TwoDArray[j+k][i];
            }
        }
    }
    long long endTime = GetCurrentMicroseconds();
    cout << "Unfolded (" << Dimension << "): " << (endTime - startTime) / 1000.0 << "ms" << endl;
}

void ExecuteForDimensions(int* Dimensions, int NumDimensions) {
    for(int dimIndex = 0; dimIndex < NumDimensions; dimIndex++) {
        int Dimension = Dimensions[dimIndex];
        int* LinearArray = new int[Dimension];
        int** TwoDArray = new int*[Dimension];
        for(int i = 0; i < Dimension; i++)
            TwoDArray[i] = new int[Dimension];
        int* AccumulatedSums = new int[Dimension];

        Initialize(LinearArray, TwoDArray, Dimension);
        StandardApproach(LinearArray, TwoDArray, AccumulatedSums, Dimension);
        RefinedStrategy(LinearArray, TwoDArray, AccumulatedSums, Dimension);
        LoopUnfolding(LinearArray, TwoDArray, AccumulatedSums, Dimension);

        delete[] LinearArray;
        for(int i = 0; i < Dimension; i++)
            delete[] TwoDArray[i];
        delete[] TwoDArray;
        delete[] AccumulatedSums;
    }
}

int main() {
    int NumDimensions;
    int* Dimensions = GenerateDimensions(16, 4096, 16, NumDimensions);
    ExecuteForDimensions(Dimensions, NumDimensions);
    delete[] Dimensions;
}
