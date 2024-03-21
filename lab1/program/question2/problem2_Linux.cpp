#include <iostream>
#include <sys/time.h>
using namespace std;

#define ull unsigned long long

const ull Size = 67108864;
ull Sequence[Size];
int Iterations = 1;

void Prepare()
{
    for (ull index = 0; index < Size; index++)
        Sequence[index] = index;
}

void StandardMethod()
{
    struct timeval beginTime, endTime;
    gettimeofday(&beginTime, NULL);
    for(int round = 0; round < Iterations; round++)
    {
        ull totalSum = 0;
        for (ull i = 0; i < Size - 1; i += 2)
            totalSum += Sequence[i] + Sequence[i + 1];
    }
    gettimeofday(&endTime, NULL);
    cout << "Standard:" << ((endTime.tv_sec - beginTime.tv_sec) * 1000000 + (endTime.tv_usec - beginTime.tv_usec)) * 1.0 / 1000 / Iterations << "ms" << endl;
}

void EnhancedApproach()
{
    struct timeval beginTime, endTime;
    gettimeofday(&beginTime, NULL);
    for(int cycle = 0; cycle < Iterations; cycle++)
    {
        ull sumFirstHalf = 0, sumSecondHalf = 0;
        for(ull i = 0; i < Size - 1; i += 2)
        {
            sumFirstHalf += Sequence[i];
            sumSecondHalf += Sequence[i + 1];
        }
        ull combinedSum = sumFirstHalf + sumSecondHalf;
    }
    gettimeofday(&endTime, NULL);
    cout << "Enhanced:" << ((endTime.tv_sec - beginTime.tv_sec) * 1000000 + (endTime.tv_usec - beginTime.tv_usec)) * 1.0 / 1000 / Iterations << "ms" << endl;
}

int main()
{
    Prepare();
    StandardMethod();
    EnhancedApproach();
}
