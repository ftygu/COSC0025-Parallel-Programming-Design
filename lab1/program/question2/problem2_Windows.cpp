#include <iostream>
#include <windows.h>
using namespace std;

#define ull unsigned long long int

ull *a;
int LOOP = 10;

void init(ull N)
{
    a = new ull[N];
    for (ull i = 0; i < N; i++)
        a[i] = i;
}

void ordinary(ull N)
{
    long long int begin, end, freq;
    QueryPerformanceFrequency((LARGE_INTEGER *) &freq);
    QueryPerformanceCounter((LARGE_INTEGER*) &begin);
    for(int l=0;l<LOOP;l++)
    {
        ull sum = 0;
        for (ull i = 0; i < N; i++)
            sum += a[i];
    }
    QueryPerformanceCounter((LARGE_INTEGER*) &end);
    cout<<"ordinary:"<<(end-begin)*1000.0/freq/LOOP<<"ms"<<endl;
}

void optimize(ull N)
{
    long long int begin, end, freq;
    QueryPerformanceFrequency((LARGE_INTEGER *) &freq);
    QueryPerformanceCounter((LARGE_INTEGER*) &begin);
    for(int l=0;l<LOOP;l++)
    {
        ull sum1 = 0, sum2 = 0;
        for(ull i=0;i<N-1; i+=2)
            sum1+=a[i],sum2+= a[i+1];
        ull sum = sum1 + sum2;
    }
    QueryPerformanceCounter((LARGE_INTEGER*) &end);
    cout<<"optimize:"<<(end-begin)*1000.0/freq/LOOP<<"ms"<<endl;
}

int main()
{
    ull start = 16, end = 4096, step = 16;
    for(ull N = start; N <= end; N += step)
    {
        init(N);
        ordinary(N);
        optimize(N);
        delete[] a;
    }
}
