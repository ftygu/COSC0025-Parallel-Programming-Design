#include <iostream>
#include <windows.h>
#include <immintrin.h>
using namespace std;


const unsigned long long int N = 1024;
unsigned long long int a[N];
int LOOP = 1;

void init()
{
    for (unsigned long long int i = 0; i < N; i++)
        a[i] = i;
}

void ordinary()
{
    long long int begin, end, freq;
    QueryPerformanceFrequency((LARGE_INTEGER *) &freq);
    QueryPerformanceCounter((LARGE_INTEGER*) &begin);
    for(int l=0;l<LOOP;l++)
    {
        // init();
        unsigned long long int sum = 0;
        for (int i = 0; i < N; i++)
            sum += a[i];
    }
    QueryPerformanceCounter((LARGE_INTEGER*) &end);
    cout<<"ordinary:"<<(end-begin)*1000.0/freq/LOOP<<"ms"<<endl;
}

void optimize()
{
    long long int begin, end, freq;
    QueryPerformanceFrequency((LARGE_INTEGER *) &freq);
    QueryPerformanceCounter((LARGE_INTEGER*) &begin);
    for(int l=0;l<LOOP;l++)
    {
        unsigned long long int sum1 = 0, sum2 = 0;
        for(int i=0;i<N-1; i+=2)
            sum1+=a[i],sum2+= a[i+1];
        unsigned long long int sum = sum1 + sum2;
    }
    QueryPerformanceCounter((LARGE_INTEGER*) &end);
    cout<<"optimize:"<<(end-begin)*1000.0/freq/LOOP<<"ms"<<endl;
}

void avx2_optimized()
{
    LARGE_INTEGER begin, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&begin);
    for(int l = 0; l < LOOP; l++)
    {
        __m256i sumVec = _mm256_setzero_si256();
        int i = 0;
        for(; i <= N-4; i+=4)
        {
            __m256i temp = _mm256_loadu_si256((__m256i*)&a[i]);
            sumVec = _mm256_add_epi64(sumVec, temp);
        }
        unsigned long long int tempArray[4];
        _mm256_storeu_si256((__m256i*)tempArray, sumVec);
        unsigned long long int sum = tempArray[0] + tempArray[1] + tempArray[2] + tempArray[3];
        for(; i < N; i++)
            sum += a[i];
    }
    QueryPerformanceCounter(&end);
    cout << "avx2_optimized:" << (end.QuadPart - begin.QuadPart) * 1000.0 / freq.QuadPart / LOOP << "ms" << endl;
}

int main()
{
    init();
    ordinary();
    optimize();
}
