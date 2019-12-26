#pragma once

#define k2f(k, N, LEN_MS) double(N-k) * 1000 / LEN_MS // Частота в герцах
#define f2k(f, N, LEN_MS) N - f * LEN_MS / 1000

struct FFT_Item
{
	 int k; // Номер гармоники
 	 double Freq; // Частота гармоники
     double Amplitude; // Амплитуда для АЧХ
     double Faza; // Фаза для ФЧХ
     double Re;// Реальная часть
     double Im; // Мнимая
     double Freq0; // Частота по версии авторов исходного кода. Не понял, что это значит
};

void ToFourier(const double array[], int N, FFT_Item farray[], int LEN_MS);
	// Преобразование Фурье

	void From_Fourier(const FFT_Item * input, int N, double output[] );
	// Обратное преобразование Фурье. Из вхожа используются Re и Im. Фунция не тестировалась.

void ToFourier_K(const double array[], int N, FFT_Item & item, int k, int LEN_MS);
	// Преобразование Фурье для одной частоты. Можно использовать, если нужна скорость и не требуется обратного преобразования

void ToFourier_Range(const double array[], int N, FFT_Item farray[], int k_start, int k_stop, int LEN_MS); 
	// Преобразование Фурье для неполного диапазона частот. Можно использовать, если нужна скорость и не требуется обратного преобразования

double Frequency_Amplitude(const double array[], int N, int LEN_MS, int freq);



