#include "fft.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <memory.h>

void ToFourier_K(const double array[], int N, FFT_Item & item, int k, int LEN_MS)
{
	double Re = 0;
	double Im = 0;
	for(int i=0; i<N; i++){
		double Arg = 2 * M_PI * k * i / N;
		Re += array[i] * cos(Arg);
		Im -= array[i] * sin(Arg);
	}
	item.Re = Re;
	item.Im = Im;
	item.Amplitude = sqrt(pow(Re,2)+pow(Im,2));
	item.Faza = atan2(Im,Re) / M_PI * 180;
	item.Freq0 = (N-1)*k;
	item.Freq = k2f(k,N,LEN_MS); // Частота в герцах
	item.k = k;
}

double Frequency_Amplitude(const double array[], int N, int LEN_MS, int freq)
{
	int k = f2k(freq, N, LEN_MS);
	FFT_Item item;
	ToFourier_K(array, N, item, k, LEN_MS);
	return item.Amplitude; 
}

void ToFourier(const double array[], int N, FFT_Item farray[], int LEN_MS)
{
	for(int k=0; k<N; k++){
		ToFourier_K(array, N, farray[k], k, LEN_MS);
	}
}

void ToFourier_Range(const double array[], int N, FFT_Item farray[], int k_start, int k_stop, int LEN_MS)
{
	for(int k=k_start; k<k_stop; k++){
		ToFourier_K(array, N, farray[k-k_start], k, LEN_MS);
	}
}

void From_Fourier(const FFT_Item * input, int N, double output[] )
{
	for(int i=0; i<N; i++){
		double Re = 0;
		double Im = 0;
		for(int n=0; n<N; n++){
            double arg = 2 * M_PI * i * n / N;
			double a = input[n].Re; 
			double b = input[n].Im;
			double c = cos(arg);
			double d = sin(arg);
			Re += a * c - b * d;
            Im += a * d + b * c;
        }
        output[i] = sqrt(pow(Re, 2) + pow(Im, 2)) / N;
	}
}

//-----------------------------------------------------------------------
#include <stdlib.h> //qsort

static int __cdecl cmp_item(const void* p1, const void *p2)
{
	return int( ((FFT_Item*)p2)->Amplitude - ((FFT_Item*)p1)->Amplitude ); 
}

void test_fourier()
{
	static int SAMPLE_PER_SEC = 16000;
	static int LEN_MS = 500; // Длина фрагмена для анализа, миллисекуд
	int N = SAMPLE_PER_SEC * LEN_MS / 1000;

	static int Freq1 = 200;
	static int Freq2 = 400;

	// f(x) = sin(10*2*PI*x) + 0.5*sin(5*2*PI*x)
	// x - ceкунды, Гармоникти: 10 Герц, 5 Герц 

	int ASIZE = 2*N; // 2*N чтобы попробовать начать с любого места
	double * array = new double[ASIZE];   
	static int start = 111;

	for(int i=0; i<ASIZE; i++){
		double x = double(i)/SAMPLE_PER_SEC;
		array[i] = sin(Freq1*2*M_PI*x) + 0.5*sin(Freq2*2*M_PI*x);
	}

	FFT_Item * farray = new FFT_Item[N];
	ToFourier(array+start, N, farray, LEN_MS);

	qsort(farray+N/2, N/2, sizeof(FFT_Item), cmp_item);

	static int Freq_Start = 16;
	static int Freq_Stop = 1000; // максимум 8000 (половина частоты дискретизации)
	int k_stop = f2k(Freq_Start, N, LEN_MS);
	int k_start = f2k(Freq_Stop, N, LEN_MS);
	int k_num = k_stop-k_start;
	FFT_Item * farray2 = new FFT_Item[k_num];
	ToFourier_Range(array+start, N, farray2, k_start, k_stop, LEN_MS);
	qsort(farray2, k_num, sizeof(FFT_Item), cmp_item);



	delete [] farray2;
	delete [] farray;
	delete [] array;
}
