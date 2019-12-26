#pragma once

#define k2f(k, N, LEN_MS) double(N-k) * 1000 / LEN_MS // ������� � ������
#define f2k(f, N, LEN_MS) N - f * LEN_MS / 1000

struct FFT_Item
{
	 int k; // ����� ���������
 	 double Freq; // ������� ���������
     double Amplitude; // ��������� ��� ���
     double Faza; // ���� ��� ���
     double Re;// �������� �����
     double Im; // ������
     double Freq0; // ������� �� ������ ������� ��������� ����. �� �����, ��� ��� ������
};

void ToFourier(const double array[], int N, FFT_Item farray[], int LEN_MS);
	// �������������� �����

	void From_Fourier(const FFT_Item * input, int N, double output[] );
	// �������� �������������� �����. �� ����� ������������ Re � Im. ������ �� �������������.

void ToFourier_K(const double array[], int N, FFT_Item & item, int k, int LEN_MS);
	// �������������� ����� ��� ����� �������. ����� ������������, ���� ����� �������� � �� ��������� ��������� ��������������

void ToFourier_Range(const double array[], int N, FFT_Item farray[], int k_start, int k_stop, int LEN_MS); 
	// �������������� ����� ��� ��������� ��������� ������. ����� ������������, ���� ����� �������� � �� ��������� ��������� ��������������

double Frequency_Amplitude(const double array[], int N, int LEN_MS, int freq);



