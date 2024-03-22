#include <iostream>
#include<fstream>
#include <windows.h>
using namespace std;

int **b;
int *a, *sum;
int LOOP = 10;
fstream f;

void init(int N) {
    a = new int[N];
    sum = new int[N];
    b = new int*[N];
    for(int i = 0; i < N; i++) {
        a[i] = i;
        sum[i] = 0;
        b[i] = new int[N];
        for(int j = 0; j < N; j++) {
            b[i][j] = i + j;
        }
    }
}

void cleanUp(int N) {
    delete[] a;
    delete[] sum;
    for(int i = 0; i < N; i++) {
        delete[] b[i];
    }
    delete[] b;
}

void ordinary(int N) {
    long long int begin, end, freq;
    QueryPerformanceFrequency((LARGE_INTEGER *) &freq);
    QueryPerformanceCounter((LARGE_INTEGER*) &begin);
    for(int l = 0; l < LOOP; l++) {
        for(int i = 0; i < N; i++) {
            sum[i] = 0;
            for(int j = 0; j < N; j++) {
                sum[i] += a[j] * b[j][i];
            }
        }
    }
    QueryPerformanceCounter((LARGE_INTEGER*) &end);
    f << "ordinary:" << (end - begin) * 1000.0 / freq / LOOP << "ms" << endl;
}

void optimize(int N) {
    long long int begin, end, freq;
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&begin);
    for(int l=0;l<LOOP;l++)
    {
        for(int i=0;i<N;i++)
            sum[i]=0;
        for(int j=0;j<N;j++)
            for(int i=0;i<N;i++)
                sum[i]+=a[j]*b[j][i];
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&end);
    f << "optimize:" << (end - begin) *1000.0 / freq / LOOP << "ms" << endl;
}

void unroll(int N) {
    long long int begin, end, freq;
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&begin);
    for(int l = 0; l < LOOP; l++) {
        for(int i = 0; i < N; i++) 
        sum[i] = 0;
        for(int j = 0; j < N; j+=16) {
        int tmp0=0,tmp1=0,tmp2=0,tmp3=0,tmp4=0,tmp5=0,tmp6=0,tmp7=0,tmp8=0,tmp9=0,tmp10=0,tmp11=0,tmp12=0,tmp13=0,tmp14=0,tmp15=0;
        for(int i=0;i<N;i++)
        {
            tmp0+=a[j+0]*b[j+0][i];
            tmp1+=a[j+1]*b[j+1][i];
            tmp2+=a[j+2]*b[j+2][i];
            tmp3+=a[j+3]*b[j+3][i];
            tmp4+=a[j+4]*b[j+4][i];
            tmp5+=a[j+5]*b[j+5][i];
            tmp6+=a[j+6]*b[j+6][i];
            tmp6+=a[j+6]*b[j+6][i];
            tmp7+=a[j+7]*b[j+7][i];
            tmp8+=a[j+8]*b[j+8][i];
            tmp9+=a[j+9]*b[j+9][i];
            tmp10+=a[j+10]*b[j+10][i];
            tmp11+=a[j+11]*b[j+11][i];
            tmp12+=a[j+12]*b[j+12][i];
            tmp13+=a[j+13]*b[j+13][i];
            tmp14+=a[j+14]*b[j+14][i];
            tmp15+=a[j+15]*b[j+15][i];
        }
        sum[j+0]=tmp0;
        sum[j+1]=tmp1;
        sum[j+2]=tmp2;
        sum[j+3]=tmp3;
        sum[j+4]=tmp4;
        sum[j+5]=tmp5;
        sum[j+6]=tmp6;
        sum[j+7]=tmp7;
        sum[j+8]=tmp8;
        sum[j+9]=tmp9;
        sum[j+10]=tmp10;
        sum[j+11]=tmp11;
        sum[j+12]=tmp12;
        sum[j+13]=tmp13;
        sum[j+14]=tmp14;
        sum[j+15]=tmp15;
        }
        
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&end);
    f << "unroll:" << (end - begin) * 1000.0 / freq / LOOP << "ms" << endl;
}

void testSizes(int start, int end, int step) {
    for (int N = start; N <= end; N += step) {
        f << "Testing with N = " << N << endl;
        init(N);
        ordinary(N);
        optimize(N);
        unroll(N);
        cleanUp(N);
    }
}

int main() {
    f.open("txt.txt",ios::out);
    int start = 16;
    int end = 4096;
    int step = 16;

    testSizes(start, end, step);

    return 0;
}