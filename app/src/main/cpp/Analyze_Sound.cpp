//#include "stdafx.h"

#include "analyze_sound.h"

#include <stdlib.h>
//#include <stdio.h>
//#include <tchar.h>
//#include <conio.h>
#include <math.h>
//d#include <string.h>
#include <memory.h>

//#include <limits>

//-------------------------------------
int array_max_index(const int array[], int n)
{
	int max = 0;
	int i_max = 0;
	for(int i=0; i<n; i++){
		if(array[i]>max){
			max = array[i];
			i_max = i;
		}
	}
	return i_max;
}

int array_max_index(const double array[], int n)
{
	double max = 0;
	int i_max = 0;
	for(int i=0; i<n; i++){
		if(array[i]>max){
			max = array[i];
			i_max = i;
		}
	}
	return i_max;
}

//---------------------------------------
void a2a(const short * array, int n, double * farray)
{
	for(int i=0; i<n; i++) 
		farray[i] = array[i];
}

void normalize(const short * samples, int n_sample, double * norm_samples)
{
	short max_ = 0;
	short min_ = 0;
	for(int i=0; i<n_sample; i++){
		max_ = std::max( max_, samples[i] );
		min_ = std::min( min_, samples[i] );
	}

	short amax = std::max( (short)max_,(short)-min_);
		
	for(int i=0; i<n_sample; i++){
		norm_samples[i] = double(samples[i]) / amax;
	}
}

void normalize(double * samples, int n_sample)
{
	double max = 0;
	double min = 0;
	for(int i=0; i<n_sample; i++){
		max = std::max( max, samples[i] );
		min = std::min( min, samples[i] );
	}

	double amax = std::max(max,-min);
		
	for(int i=0; i<n_sample; i++){
		samples[i] = samples[i] / amax;
	}
}

static int __cdecl cmp_double_abs(const void* p1, const void *p2)
{
	double v1 = fabs( *((double*)p1) );
	double v2 = fabs( *((double*)p2) );
	return int(v2 - v1); 
}

void normalize_percent(const short * samples, int n_sample, double * norm_samples, double percent)
{
	double * sorted = new double[n_sample];
	for(int i=0; i<n_sample; i++)
		sorted[i] = samples[i];

	qsort(sorted, n_sample, sizeof(double), cmp_double_abs);

	int j = n_sample-n_sample*percent/100;
	double amax = abs( sorted[j] );

	for(int i=0; i<n_sample; i++){
		norm_samples[i] = std::min(1.0, samples[i] / amax);
	}

	delete [] sorted;
}

double calc_abs_av(const double * samples, int n_sample)
{
	double av = 0;
	for(int i=0; i<n_sample; i++){
		av = ( av * i + abs(samples[i]) ) / (i+1); 
	}
	return av;
}

double calc_abs_av(const short * samples, int n_sample)
{
	double av = 0;
	for(int i=0; i<n_sample; i++){
		av = ( av * i + abs(samples[i]) ) / (i+1); 
	}
	return av;
}
	
void adjust_abs_av(double * samples, int n_sample, double av_wanted)
{
	double av = calc_abs_av(samples, n_sample);
	double k = av_wanted / av;
	for(int i=0; i<n_sample; i++){
		samples[i] *= k;
		samples[i] = std::max(-1.0,samples[i]);
		samples[i] = std::min(1.0,samples[i]);
	}
}

void adjust_abs_av_0(double * samples, int n_sample, double av_wanted)
{
	double av = calc_abs_av(samples, n_sample);
	double k = av_wanted / av;
	for(int i=0; i<n_sample; i++){
		samples[i] *= k;
		samples[i] = std::max(-1.0,samples[i]);
		samples[i] = std::min(1.0,samples[i]);
	}
}

void prob_distribution(double probs[], int n_prob, const double * norm_samples, int n_sample)
{
	memset(probs, 0, n_prob*sizeof(double));
	for(int i=0; i<n_sample; i++){
		double x = std::min(2.0, norm_samples[i]+1); /// ���������� � �������� [0,2]
		int index = int( (n_prob-1) * x / 2 );
		if(index<n_prob)
			probs[index]++;
		else
			; //__debugbreak(); // index=index; // detTrap()
	}
	for(int j=0; j<n_prob; j++){
		probs[j] /= n_sample;
	}
}

void prob_distribution_abs_row(double probs[], int n_prob, const short * samples, int n_sample)
{
	memset(probs, 0, n_prob*sizeof(double));
	for(int i=0; i<n_sample; i++){
		double x = abs( samples[i] );  
		int index = int( n_prob * x );
		probs[index]++; 
	}
	for(int j=0; j<n_prob; j++){
		probs[j] /= n_sample;
	}
}

//#define log_2(x) log((x)) / log(2.0)

double calc_entropy(const double * norm_samples, int n_sample)
{
	#define NPROBS 201
	double probs[NPROBS];
	prob_distribution(probs, NPROBS, norm_samples, n_sample);

	double entropy = 0;
	for(int j=0; j<NPROBS; j++){
		if(probs[j] > std::numeric_limits<double>::epsilon()) {
			double log_2 = log(probs[j]) / log(2.0);
			entropy += probs[j] * log_2;
		}
	}
	entropy = -entropy;
	return entropy;
}


void cut_peaks(short * samples, int n_sample, short Thresold)
{
	for(int i=0; i<n_sample; i++){
		if(samples[i] > Thresold)
			samples[i] = Thresold;
		else if(samples[i] < -Thresold)
			samples[i] = -Thresold;
	}
}

void cut_peaks(double * samples, int n_sample, double Thresold)
{
	for(int i=0; i<n_sample; i++){
		if(samples[i] > Thresold)
			samples[i] = Thresold;
		else if(samples[i] < -Thresold)
			samples[i] = -Thresold;
	}
}

void filter(short * samples, int n_sample)
{
	double av = calc_abs_av(samples, n_sample);
	#define Thresold_K 5
	cut_peaks(samples, n_sample, (int)av*Thresold_K);
}

void filter(double * samples, int n_sample, double Threshold)
{
	double av = calc_abs_av(samples, n_sample);
	cut_peaks(samples, n_sample, Threshold);
}


int find_max(double * frames, int n)
{
	int i_max = -1;
	double max=0;
	for(int i=0; i<n; i++){
		if(frames[i] > max){
			max = frames[i];
			i_max = i;
		}
	}
	return i_max;
}

// ������� ������ ���������
int find_peak_indexes(double * frames, int n, int * peaks, int n_peak_max, int NN)
{
	int n_peak = 0;
	double max = 0;
	int i_max = -1;

	//for(int i=n-1-NN; i>=NN; i--){
	for(int i=NN; i<n-NN; i++){
		int j_max = find_max(frames+i-NN, 2*NN+1);
		if(j_max==NN){
			if(n_peak<n_peak_max) 
				peaks[n_peak++] = i;

			if(frames[i]>max){
				max = frames[i];
				i_max = i;
			}
		}
	}
	return i_max;
}


//----------------------------------------------------------------------------------------------

int scan_forward(const double * frames, int n_frame, int pos0, int b_hi, double Level, int Count)
{
	int pos = 0;
	int count = 0;
	for(int j=pos0; j<n_frame; j++){
		bool b = b_hi && frames[j] > Level || !b_hi && frames[j] < Level;
		if(b){
			if(++count >= Count){
				pos = j - Count + 1;
				break;
			}
		} else {
			count = 0;
		}
	}
	return pos;
}

int scan_back(const double * frames, int pos0, int b_hi, double Level, int Count)
{
	int pos = 0;
	int count = 0;
	for(int j=pos0; j>0; j--){
		bool b = b_hi && frames[j] > Level || !b_hi && frames[j] < Level;
		if(b){
			if(++count >= Count){
				pos = j;
				break;
			}
		} else {
			count = 0;
		}
	}
	return pos+Count;
}

int scan_forward_av(const double * frames, int n_frame, int pos0, int b_hi, double Level, int W)
{
	double av = calc_abs_av(frames+pos0, W); // ������ ����
	int pos = 0;
	for(int j = pos0; j<n_frame-W; j++){
		if(j>pos0)
			av = av + (abs(frames[j+W-1])-abs(frames[j-1]))/W;
		bool b = b_hi && av > Level || !b_hi && av < Level;
		if(b){
			pos = j;
			break;
		}
	}
	return pos;
}

int scan_back_av(const double * frames, int pos0, int b_hi, double Level, int W)
{
	double av = calc_abs_av(frames+pos0-W, W); // ������ ����
	int pos = 0;
	for(int j = pos0-W; j>=0; j--){
		if(j<pos0-W)
			av = av + (abs(frames[j])-abs(frames[j+W-1]))/W;
		bool b = b_hi && av > Level || !b_hi && av < Level;
		if(b){
			pos = j;
			break;
		}
	}
	return pos+W;
}

int scan_forward_drop(const double * frames, int n_frame, int pos0, int b_hi, int W, double K__, double Level)
{
	int pos = 0;
	double k_best = 1;
	int count_level = 0;
	double av_pre = calc_abs_av(frames+pos0-W, W); 
	double av = calc_abs_av(frames+pos0, W);
	for(int j = pos0; j<n_frame-W; j++){
		if(j>pos0){
			av_pre = av_pre + (abs(frames[j-1])-abs(frames[j-W-1]))/W;
			av = av + (abs(frames[j+W-1])-abs(frames[j-1]))/W;
		}
		
		//double Level = av_pre * K;
		bool b_level = b_hi && av > Level || !b_hi && av < Level;
		//if(b_level){
		//	pos = j;
		//	break;
		//}
		if(b_level){
			count_level++;
			if(count_level>2*W){ // ����� ����������
				break;
			}
		} else {
			continue;
		}
		double k = av / av_pre;
		bool b_best = b_hi ? k > k_best : k < k_best;
		if(b_best){
			k_best = k;
			pos = j;
		}
	}
	return pos;
}

int scan_forward_percent(const double * frames, int n_frame, int pos0, int b_hi, double Level, int W, int Percent)
{
	int WP = W * Percent / 100;
	int count = 0;
	bool b;
	// ������ ����
	for(int j = pos0; j<pos0+W; j++){
		double val = abs(frames[j]);
		b = b_hi && val > Level || !b_hi && val < Level;
		if(b)
			count++;
	}

	int pos = 0;
	for(int j = pos0; j<n_frame-W; j++){
		if(j>pos0){
			double val_out = abs(frames[j-1]);
			bool b_out = b_hi && val_out > Level || !b_hi && val_out < Level;
			if(b_out)
				count--;
			double val_in = abs(frames[j+W-1]);
			bool b_in = b_hi && val_in > Level || !b_hi && val_in < Level;
			if(b_in)
				count++;
		}
		if(count >= WP){
			pos = j;
			break;
		}
	}
	return pos;
}


//----------------------------------------------------------
void fill_frames_abs_av_0(const short * samples, int n_sample, int frame_size, double * frames, int n_frames, int step)
{
	for(int j=0, i=0; j<n_frames && i<=n_sample-frame_size; j++, i+=step){
		double av = calc_abs_av(samples+i, frame_size);
		frames[j] = av;
	}
}

void fill_frames_abs_av_0(const double * norm_samples, int n_sample, int frame_size, double * frames, int n_frames, int step)
{
	for(int j=0, i=0; j<n_frames && i<=n_sample-frame_size; j++, i+=step){
		double av = calc_abs_av(norm_samples+i, frame_size);
		frames[j] = av;
	}
}

void fill_frames_abs_av_2(const double * norm_samples, int n_sample, int frame_size, double * frames, int n_frames)
{
	fill_frames_abs_av_0(norm_samples, n_sample, frame_size, frames, n_frames, frame_size/2);
}

void fill_frames_abs_av_1(const double * norm_samples, int n_sample, int frame_size, double * frames, int n_frames)
{
	fill_frames_abs_av_0(norm_samples, n_sample, frame_size, frames, n_frames, frame_size);
}

void fill_frames_entropy_0(const double * norm_samples, int n_sample, int frame_size, double * frames, int n_frames, int step)
{
	for(int j=0, i=0; j<n_frames && i<=n_sample-frame_size; j++, i+=step){
		double e = calc_entropy(norm_samples+i, frame_size);
		frames[j] = e;
	}
}

void fill_frames_max(const double * norm_samples, int n_sample, int frame_size, double * frames, int n_frames)
{
	int step = frame_size / 2;
	for(int j=0, i=0; j<n_frames && i<=n_sample-frame_size; j++, i+=step){
		int index_max = array_max_index(norm_samples+i, frame_size);
		frames[j] = norm_samples[i+index_max];
	}
}

int find_max(const short * samples, int n_sample)
{
	int max = 0;
	for(int i=0; i<n_sample; i++){
		max = std::max( max, abs(samples[i]) );
	}	
	return max;
}

int count_av(const short * samples, int n_sample)
{
	double av = 0;
	for(int i=0; i<n_sample; i++){
		av = ( av * i + abs(samples[i]) ) / (i+1); 
	}
	return (int)av;
}

int count_hi(const short * samples, int n_sample, int Threshold)
{
	int count=0;
	for(int i=0; i<n_sample; i++){
		if( abs(samples[i]) > Threshold )
			count++;
	}	
	return count;
}

int count_low(const short * samples, int n_sample, int Threshold)
{
	int count=0;
	for(int i=0; i<n_sample; i++){
		if( abs(samples[i]) < Threshold )
			count++;
	}	
	return count;
}

int find_pauses_av(const double * frames, int n_frame, Zone pauses[], int n_pause_max, double T, int W)
// ���� ����� ������� (W) � ������� ������� ���� ������ T. ---------//������� - ����� ����  
{
	int n=0;
	double av20 = calc_abs_av(frames, W); // ������ ����
	bool b_in_pause = false;
	int start = 0;
	for(int j = 0; j<n_frame-W; j++){
		if(j>0)
			av20 = av20 + (frames[j+W-1]-frames[j-1])/W;
		bool b = av20 < T; 
		if(b){
			if(b_in_pause){
				; // ����� ������������
			} else {
				b_in_pause = true;
				start = j;
			}
		} else {
			if(b_in_pause){ // ����� �����
				//int x = start + (j-start)/2 + W/2; // �������� ����
				//if(n<a_len)
				//	a[n++] = x;
				if(n<n_pause_max){
					pauses[n++] = Zone(start, j+W);
				}
				b_in_pause = false;
			}
		}

	}
	/*
	if(b_in_pause){ // ����� � �����
		int x = start + W/2; // �������� ����
		if(n<a_len)
			a[n++] = x;
	}*/
	return n;
}



int find_pauses2(const double * frames, int n_frame, Zone pauses[], int n_pause_max, double T, int P, int W)
// ���� ����� ������� (W) � ������� P % ������� ���� ������ T. ---------//������� - ����� ����  
{
	int WP = W * P / 100;
	int n=0;
	// ������ ����
	int m = 0;
	for(int j=0; j<W; j++){
		if(frames[j]<T)
			m++;
	}
	bool b_in_pause = false;
	int start = 0;
	for(int j = 0; j<n_frame-W; j++){ // ������� ����
		if(j>0){
			if(frames[j-1]<T) m--;
		    if(frames[j+W-1]<T) m++;
		}
		if(m < WP){
			if(b_in_pause){
				; // ����� ������������
			} else {
				b_in_pause = true;
				start = j;
			}
		} else {
			if(b_in_pause){ // ����� �����
				if(n<n_pause_max){
					pauses[n++] = Zone(start, j+W);
				}
				b_in_pause = false;
			}
		}

	}
	return n;
}

int find_pauses3(const double * frames, int n_frame, Zone pauses[], int n_pause_max, double T, int WMin, int WMax)
// ���� ����� ������� (WMin..WMax) � ������� ������ ���� ������ T. ---------//������� - ����� ����  
{
	int n=0;
	bool b_in_pause = false;
	int start = 0;
	for(int j = 0; j<n_frame; j++){
		bool b = frames[j] < T; 
		if(b){
			if(b_in_pause){
				; // ����� ������������
			} else {
				b_in_pause = true;
				start = j;
			}
		} else {
			if(b_in_pause){ // ����� �����
				int len = j-start;
				if(len >= WMin && len <= WMax){		
					if(n<n_pause_max){
						pauses[n++] = Zone(start, j);
					}
				}
				b_in_pause = false;
			}
		}
	}
	return n;
}

/// ����� � ������. ���� � ������. 
/// ����� ������ ��������� ����� �� W �������, � ������� ������� ��������� ���� ������� �� X percent
int scan_back_for_silence_start(const double * frames, int n_frame, int pos0, int W, int X) 
{
	pos0 = std::max(pos0, n_frame-W);
	int pos=0;
	double avn=0; // ������� � ���� W
	double av=0; // ������� ����� � ���������� �� W
	int av_count=0;
	for(int j=pos0; j>0; j--){
		if(j==pos0){
			avn = calc_abs_av(frames+pos0, W); // ������ ����
			av = avn;
			av_count = 0;
		} else {
			avn += (frames[j]-frames[j+W])/W; 
			if(j<=pos0-W){
				av = ( av * av_count + frames[j+W] ) / (av_count+1);
				av_count++;
			}
		}

		if(j<pos0-2*W && avn > av * (100+X) / 100){
			pos = j;
			break; 
		}
	}
	return pos;
}
