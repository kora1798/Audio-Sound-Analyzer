#pragma once


void filter(short * samples, int n_sample);

void a2a(const short * array, int n, double * farray);
void normalize(const short * samples, int n_sample, double * norm_samples);

void normalize_percent(const short * samples, int n_sample, double * norm_samples, double percent);

double calc_abs_av(const short * samples, int n_sample);
double calc_abs_av(const double * samples, int n_sample);
double calc_entropy(const double * norm_samples, int n_sample);

void fill_frames_abs_av_0(const short * samples, int n_sample, int frame_size, double * frames, int n_frames, int step);

void fill_frames_abs_av_2(const double * norm_samples, int n_sample, int frame_size, double * frames, int n_frames);
	// step = 1/2 frame
void fill_frames_abs_av_1(const double * norm_samples, int n_sample, int frame_size, double * frames, int n_frames);
	// step = frame

void fill_frames_max(const double * norm_samples, int n_sample, int frame_size, double * frames, int n_frames);

void fill_frames_entropy_0(const double * norm_samples, int n_sample, int frame_size, double * frames, int n_frames, int step);
	// step = frame

// ������� ������ ���������
int find_peak_indexes(double * frames, int n, int * peaks, int n_peak_max, int NN);


void adjust_abs_av(double * samples, int n_sample, double av_wanted);
void adjust_abs_av_0(double * samples, int n_sample, double av_wanted);

int test_frames(const double * norm_samples, int n_sample, int frame_size);

int find_max(const short * samples, int n_sample);
int count_av(const short * samples, int n_sample);

int count_hi(const short * samples, int n_sample, int Threshold);
int count_low(const short * samples, int n_sample, int Threshold);

void find_last_sound(int nSamplesPerSec, short * samples, int n_sample);

//int find_pauses(const double * frames, int n_frame, int a[], int a_len, double T, int W);
#include <vector>
typedef std::pair <int, int> Zone;
typedef Zone * ZonePtr; 
typedef const Zone * ZoneCPtr; 
#define Zone_Center( z ) (z.first+z.second)/2;
int find_pauses_av(const double * frames, int n_frame, Zone pauses[], int n_pause_max, double T, int W);
// ���� ����� ������� (W) � ������� ������� ���� ������ T. ---------//������� - ����� ���� 

int find_pauses2(const double * frames, int n_frame, Zone pauses[], int n_pause_max, double T, int P, int W);
// ���� ����� ������� (W) � ������� P% ������� ���� ������ T. ---------//������� - ����� ���� 

int find_pauses3(const double * frames, int n_frame, Zone pauses[], int n_pause_max, double T, int WMin, int WMax);
// ���� ����� ������� (WMin..WMax) � ������� ������ ���� ������ T. ---------//������� - ����� ����  

int scan_forward(const double * frames_abs_av, int n_frame, int pos0, int b_hi, double Level, int Count);
// ���� ����� ������� (Count) � ������� ��� ����/���� ������ Level. ������� - ������ ������
int scan_back(const double * frames_abs_av, int pos0, int b_hi, double Level, int Count); 
// ���� ����� ������� (Count) � ������� ��� ����/���� ������ Level. ������� - ����� ������

int scan_forward_percent(const double * frames, int n_frame, int pos0, int b_hi, double Level, int Count, int Percent);
// ���� ����� ������� (Count) � ������� P% ����/���� ������ Level. ������� - ������ ������

int scan_forward_av(const double * frames, int n_frame, int pos0, int b_hi, double Level, int W);
// ���� ����� ������� (W), � ������� ������� ����/���� ������ Level. ������� - ������ ������
int scan_back_av(const double * frames, int pos0, int b_hi, double Level, int W); 
// ���� ����� ������� (W), � ������� ������� ����/���� ������ Level. ������� - ����� ������

int scan_forward_drop(const double * frames, int n_frame, int pos0, int b_hi, int W, double K, double Level);
// ���� ������. // ���� ����� ������� (W), � ������� ������� ����/���� ��������� ������, ����������� �� K. ������� - ������ ������

/// ����� � ������. ���� � ������. 
/// ����� ������ ��������� ����� �� N �������, � ������� ������� ��������� ���� ������� �� X percent
int scan_back_for_silence_start(const double * frames, int n_frame, int pos0, int N, int X);

