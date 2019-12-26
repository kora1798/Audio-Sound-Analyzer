
#include "metrics.h"
#include "analyze_sound.h"
#include "analyzer3.h"
#include "trace.h"
#include "fft.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <ctime>
#include <limits>

typedef std::pair<int,int> pair_int;

#define in_range(x, x1, x2) (x)>(x1) && (x)<(x2)
#define in_range2(x, pair) (x)>(pair.first) && (x)<(pair.second)

//---------------------------------------------------------------------------
#include <stdlib.h> //qsort


//static int EV_MIN = 2000;

/*
bool evaluate_frame(FFT_Item * farray2, int k_num)
{
	bool b = false;

	const DeviceData & dd = g_dd;
	double ev = 0;
	static int K[5] = {0,4,3,3,2};
	static int k_use = 20;
	for(int k=0; k<k_use; k++){
		int ik = 0;
		if( in_range2(farray2[k].Freq, dd.A) ) ik = 1;
		else if( in_range2(farray2[k].Freq, dd.A2) ) ik = 2;
		else if( in_range2(farray2[k].Freq, dd.B) ) ik = 3;
		else if( in_range2(farray2[k].Freq, dd.B2) ) ik = 4;
		double ev_k = K[ik] * (k_use-k);
		ev += ev_k;
	}
	b = ev>EV_MIN;
	return b;
}
*/

static int __cdecl cmp_item(const void* p1, const void *p2)
{
	return int( ((FFT_Item*)p2)->Amplitude - ((FFT_Item*)p1)->Amplitude ); 
}

#define ADD_AV(av, i, val) (av*i + val) / (i+1); 

double amp_frame(const double * array, int N, int LEN_MS, int Freq)
{
	double av = Frequency_Amplitude(array, N, LEN_MS, Freq);
	return av / LEN_MS;
}

double amp_frame(const double * array, int N, int LEN_MS, int Freq_Min, int Freq_Max)
{
	int freq_step = 1000 / LEN_MS;
	double av = 0;
	for(int freq=Freq_Min, i=0; freq<=Freq_Max; freq+=freq_step, i++){ 
		double amp = Frequency_Amplitude(array, N, LEN_MS, freq);
		av = ADD_AV(av, i, amp); 
	}
	return av / LEN_MS;
}

/*
bool check_frame(const double * array, int N, int LEN_MS)
{
	bool b_zone2 = false;
	//FFT_Item * farray = new FFT_Item[N];
	//ToFourier(array, N, farray, LEN_MS);

	// Так нельзя - слишком медленно
	//static int Freq_Start = 200;
	//static int Freq_Stop = 500; 
	//int k_stop = f2k(Freq_Start, N, LEN_MS);
	//int k_start = f2k(Freq_Stop, N, LEN_MS);
	//int k_num = k_stop-k_start;
	//FFT_Item * farray2 = new FFT_Item[k_num];
	//ToFourier_Range(array, N, farray2, k_start, k_stop, LEN_MS);
	//qsort(farray2, k_num, sizeof(FFT_Item), cmp_item);
	//b_zone2 = evaluate_frame(farray2, k_num);
	
	double av = amp_frame(array, N, LEN_MS);
	const DeviceData & dd = g_dd;
	b_zone2 = av>dd.Level;
	return b_zone2;
}*/

struct F_Data 
{
	double av; // Средняя амплитуда последней секунды
	bool b_zone;

	double amp_max_sec; // Максимальная амплитуда Частоты в последней секунде
	int i_amp_max_sec; // Позиция этой амплитуды

	double amp_max_total; // Максимальная амплитуда Частоты за время наблюдения
	
	double av_max_in_zone; // Максимальная средняя амплитуда 500 мс фрагмента в зоне
	int i_av_max_in_zone; // Позиция этой амплитуды

	// Бесмысленный double amp_zone_start; // Амплитуда Частоты в найденной зоне (от старта зоны длиной 500мс)

	int i_zone_start1;
	int i_zone_stop1;
	int i_zone_start2;
	int i_zone_stop2;

	double av_zone; // Средняя амплитуда в зоне

	int answer; // 0 - не определилась, 4 - короткая, 5 - никакая, 6 - длинная 
	int signal; // Одноразовый сигнал, является ли последняя найденная зона ответом. 4 или 6, равный answer
};

#define MS_LEN 200 // 250 // 500

struct Freq_Level {
	int Freq1;
	int Freq2;
	int Level;
};

struct DeviceData {
	int ms_short;
	int ms_long;
	int ms_zone2_len_min;
	int ms_zone2_len_max;

	#define N_FREQ 3
	Freq_Level freq[N_FREQ];
};

static DeviceData dd_home = {
	530, 640,
	450, 700,
	225, 225, 2500,  
};

static DeviceData dd_acropolis = {
	530, 640,
	450, 700,
	225, 225, 9000, // 17000,  
};

static DeviceData dd_center = { // Не получается зону по частоте определять
	530, 640,
	450, 700,
	240, 255, 8000,
};

static DeviceData dd_radisson_big = { // Не получается зону по частоте определять
	530, 640,
	450, 700,
	250, 250, 6000,
};

static DeviceData dd_nese = { // Не получается зону по частоте определять
	530, 640,
	450, 700,
	270, 270, 15000, //265
};


static DeviceData g_dd = dd_acropolis; 

bool f(const short * samples, int n_sample, F_Data & fdata)
{
	bool b_zone_finished = false; 

	static int ms_len = MS_LEN; // Длина фрагмена для анализа, миллисекунд
	static int len = ms2n(ms_len);
	static int ms_step = 5;
	static int step = ms2n(ms_step);

	double * fsamples = new double[n_sample]; 
	a2a(samples, n_sample, fsamples);

	fdata.av = calc_abs_av(fsamples, n_sample);

	fdata.amp_max_sec = 0;
	fdata.av_max_in_zone = 0;

	// Получили очередную секунду. 
	
	// ---- Анализируем все новые окна шириной ms_len
	//int i_start = n_sample - ms2n(1000) - len;
	//int i_stop = n_sample - len;
	
	// Анализируем секундный кусок начиная со вотрой половины предыдущей секунды
	int i_start = n_sample - ms2n(1500);
	int i_stop = n_sample - ms2n(500);

	double av = calc_abs_av(fsamples+i_start, len); // Первое окно
	for(int i=i_start; i<i_stop; i+=step){
		if(i!=i_start)
			av += (abs(fsamples[i+len-1])-abs(fsamples[i-1]))/len; // Двигаем окно

		//double amp = amp_frame(fsamples+i, len, ms_len, g_dd.Freq);
		double amp = amp_frame(fsamples+i, len, ms_len, g_dd.freq[0].Freq1, g_dd.freq[0].Freq2);
		if(amp>fdata.amp_max_sec){
			fdata.amp_max_sec = amp;
			fdata.i_amp_max_sec = i;
		}
		//bool b_zone = amp > g_dd.Level;
		bool b_zone = amp > g_dd.freq[0].Level;
		if(b_zone){
			// b_zone2 = true; не здесь, так как может закончится в следующей секунде
			if(fdata.b_zone){
			} else {
				fdata.b_zone = true;
				b_zone_finished = false;
				fdata.i_zone_start1 = i;
				fdata.i_zone_stop1 = i + len;
				fdata.av_max_in_zone = 0;
			}

			if(av>fdata.av_max_in_zone){
				fdata.av_max_in_zone = av;
				fdata.i_av_max_in_zone = i;
			}

		} else {
			if(fdata.b_zone){
				fdata.i_zone_stop1 = i + len;
				fdata.b_zone = false;
				b_zone_finished = true; // Нашли законченную зону 
				fdata.answer = 0;
			} 
		}

	}

	if(b_zone_finished){ // Check
		if( fdata.i_zone_start1<0 ) // Слишком длинная 
			b_zone_finished = false;
	}
	
	if(b_zone_finished){
		//int i_center = (fdata.i_zone_stop1 + fdata.i_zone_start1) / 2; // Зависит от уровня отсечки Level
		//int i_center = fdata.i_amp_max + len/2;
		//double av = calc_abs_av(fsamples+i_center-len/2, len);
		
		//int i_start_av_count = fdata.i_av_max_in_zone + 0.2*len;
		//int len_av_count = 0.6*len;
		//double av = calc_abs_av(fsamples+i_start_av_count, len_av_count);
		//int i_start_scan_forward = fdata.i_amp_max + 0.8*len; // i_center
		//int i_start_scan_back = fdata.i_amp_max + 0.2*len; // i_center
		

		int len_zone1 = fdata.i_zone_stop1 - fdata.i_zone_start1;
		fdata.av_zone = calc_abs_av(fsamples+fdata.i_zone_start1, len_zone1);
		int i_start_scan_forward = fdata.i_zone_stop1;
		int i_start_scan_back = fdata.i_zone_start1;

		// Ищем тишину
		{
			/*
			static int percent_f = 80;
			double level_f = fdata.av_zone * percent_f / 100; // av

			static int ms_w_f = 50;
			int w_f = ms2n(ms_w_f);		
		
			fdata.i_zone_stop2 = scan_forward_av(fsamples, n_sample, i_start_scan_forward, false, level_f, w_f);
			*/

			/*
			static int ms_w_f1 = 100;
			static int percent_level_f1 = 60;
			static int percent_scan_f1 = 95;

			static int ms_w_f2 = 40;
			static int percent_level_f2 = 50;
			static int percent_scan_f2 = 95;
		
			int w_f1 = ms2n(ms_w_f1);
			double level_f1 = fdata.av_zone * percent_level_f1 / 100;
			int stop_f1 = scan_forward_percent(fsamples, n_sample, i_start_scan_forward, false, level_f1, w_f1, percent_scan_f1);
			
			int w_f2 = ms2n(ms_w_f2);
			double level_f2 = fdata.av_zone * percent_level_f2 / 100;
			int stop_f2 = scan_forward_percent(fsamples, n_sample, i_start_scan_forward, false, level_f2, w_f2, percent_scan_f2);
			
			fdata.i_zone_stop2 = std::max(stop_f1, stop_f2);

			static double k_drop = 0.5;
			fdata.i_zone_stop2 = scan_forward_drop(fsamples, n_sample, i_start_scan_forward, false, w_f, k_drop);
			*/

			static int ms_w = 50;
			int w = ms2n(ms_w);
			/*
			#define N_TRY 5
			static int a_percent[N_TRY] = {60,65,70,75,80};
			static double a_k_drop[N_TRY] = {0.3,0.35,0.4,0.45,0.5};
			for(int i=0; i<N_TRY; i++){
				double level = fdata.av_zone * a_percent[i] / 100; // av
				//fdata.i_zone_stop2 = scan_forward_av(fsamples, n_sample, i_start_scan_forward, false, level, w);
				fdata.i_zone_stop2 = scan_forward_drop(fsamples, n_sample, i_start_scan_forward, false, w, a_k_drop[i]);
				if(fdata.i_zone_stop2) 
					break;
			}*/

			static int percent = 80;
			double level = fdata.av_zone * percent / 100; // av
			fdata.i_zone_stop2 = scan_forward_drop(fsamples, n_sample, i_start_scan_forward, false, w, 0, level);



		}

		if(fdata.i_zone_stop2){
			
			i_start_scan_back = fdata.i_zone_stop2 - ms2n(g_dd.ms_zone2_len_min);

			// Ищем паузу
			static int percent_b = 80;
			static int ms_w_b = 50;
			double level_b = fdata.av_zone * percent_b / 100; // av
			int w_b = ms2n(ms_w_b);		
			//fdata.i_zone_start2 = scan_back_av(fsamples, i_start_scan_back, false, level_b, w_b);

			#define N_TRY 3
			static int a_percent_b[N_TRY] = {60,70,80};
			for(int i=0; i<N_TRY; i++){
				double level_b = fdata.av_zone * a_percent_b[i] / 100; // av
				fdata.i_zone_start2 = scan_back_av(fsamples, i_start_scan_back, false, level_b, w_b);
				if(fdata.i_zone_start2) 
					break;
			}

			//fdata.amp_zone_start = amp_frame(fsamples+fdata.i_zone_start2, len, ms_len, g_dd.Freq);

			int ms_len = n2ms(fdata.i_zone_stop2 - fdata.i_zone_start2);
			if( in_range(ms_len, g_dd.ms_zone2_len_min, g_dd.ms_short) ){
				fdata.answer = 4;
				fdata.signal = fdata.answer;
			} else if( in_range(ms_len, g_dd.ms_short, g_dd.ms_long) ){
				fdata.answer = 5;
			} else if( in_range(ms_len, g_dd.ms_long, g_dd.ms_zone2_len_max) ){
				fdata.answer = 6;
				fdata.signal = fdata.answer;
			} else { 
				fdata.answer = 0;
			}
		}
	}

	fdata.amp_max_total = std::max(fdata.amp_max_sec, fdata.amp_max_total);

	delete [] fsamples;
	return b_zone_finished;
}


//------------------------------------------------------------------------
static FILE * logfile;

static void start_log()
{
	char logfilename[1000];
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	strftime(logfilename, 1000, "%Y%m%d-%H%M%S.txt", timeinfo);
	#if defined WIN32
	const char dir[] = "";
	#else
	//const char dir[] = "/storage/emulated/0/myFiles/";
    //const char dir[] = "/storage/emulated/0/WAVEFiles/";
    const char dir[] = "/storage/emulated/0/D+/";
	#endif
	char path[1000];
	sprintf(path, "%s%s", dir, logfilename);

    logfile = fopen(path, "w");
}

static void stop_log()
{
	if(logfile){
		fclose(logfile);
		logfile = 0;
	}
}

//-----------------------------------------------------------------------------------------
#define SEC 2
#define N_SAMPLE SEC*SamplesPerSec

struct A2_Data
{
	bool b_active;
	int sec;
	short samples[N_SAMPLE];

	F_Data fdata;
	int state;
	int sec_zone2;
	/*
	bool b_zone2;
	double amp_224;

	int ms_zone2_start; // от начала 5-секундного фрагмента
	int ms_zone2_stop; // от начала 5-секундного фрагмента
	*/
};

static A2_Data data;
int a2_init()
{
	int ret = 0;
	return ret;
}

int a2_dispose()
{
	int ret = 0;
	return ret;
}

int a2_start() // Начало работы. После этого вызова, можно вызывать a2_fragment()
{
	memset(&data, 0, sizeof(A2_Data));
	data.b_active = true;
	start_log();
	print_line(logfile, data.sec,"a2_start()");
	return 0;
}

int a2_stop() // Конец работы.
{
	data.b_active = true;
	print_line(logfile, data.sec, "a2_stop()");
	stop_log();
	return 0;
}
	
int a2_fragment(const short * samples_in, int n_sample_in)
{
	if(!data.b_active) return -1;

	n_sample_in = std::min(n_sample_in, N_SAMPLE);

	int n_sample_old = N_SAMPLE-n_sample_in;
	memmove(data.samples, data.samples+n_sample_in, n_sample_old*sizeof(short));
	memcpy(data.samples+n_sample_old, samples_in, n_sample_in*sizeof(short));
	
	if(data.fdata.b_zone) { // Зона началась в предыдущей секунде
		data.fdata.i_zone_start1 -= ms2n(1000);
		data.fdata.i_zone_stop1 -= ms2n(1000);
	}
	bool b_zone2 = f(data.samples, N_SAMPLE, data.fdata);
	if(b_zone2){
		data.state = 2;
		data.sec_zone2 = data.sec;
		print_line(logfile, data.sec, "Zone av=%.0f freq_amp=%.0f ms_start=%i ms_len=%i answer=%i", 
			data.fdata.av, data.fdata.amp_max_sec, n2ms(data.fdata.i_zone_start2), n2ms(data.fdata.i_zone_stop2-data.fdata.i_zone_start2), data.fdata.answer );
	} else {
		data.state = 0;
	}
	data.sec++;
	return 0;
}

int a2_get_parameter(int param)
{
	int ret = 0;
	if(param==1){
		ret = g_dd.ms_short;
	} else if(param==2){
		ret = g_dd.ms_long;
	} else if(param==3){
		ret = g_dd.freq[0].Level;
	}


	else if(param==11){
		ret = data.state;	
	} else if(param==12){
		ret = (int)data.fdata.av;
	} else if(param==13){
		ret = (int)data.fdata.amp_max_sec; 



	// Нашли Зону
		//ret = (int)data.fdata.amp_zone_start;
		//ret = (int)data.fdata.av_zone;	

	} else if(param==14){

	} else if(param==15){
		ret = n2ms(data.fdata.i_zone_stop2-data.fdata.i_zone_start2);
	} else if(param==16){
		ret = data.sec - data.sec_zone2;
	} else if(param==17){
		ret = data.fdata.answer;	
	} else if(param==18){
		if(data.fdata.signal){
			ret = data.fdata.signal;
			data.fdata.signal=0;
		}
	} else if(param==19)
		ret = n2ms(data.fdata.i_zone_start2);
	else if(param==20)
		ret = n2ms(data.fdata.i_zone_stop2) ; 
	
	else if(param==21)
		ret = n2ms(data.fdata.i_zone_start1);
	else if(param==22)
		ret = n2ms(data.fdata.i_zone_stop1); 

	else if(param==23)
		ret = n2ms(data.fdata.i_amp_max_sec); 
	else if(param==24)
		ret = n2ms(data.fdata.i_av_max_in_zone); 

	if(param>=19)
		; // ret += 3000; // от начала 5-секундного фрагмента

	return ret;
}

int a2_set_parameter(int param, int value)
{
	int ret = 0;
    print_line(logfile, data.sec, "a2_set_parameter(%i)=%i", param, value);
	if(param==1){
		g_dd.ms_short = value;
	} else if(param==2){
		g_dd.ms_long = value;
	} else if(param==3){
		g_dd.freq[0].Level = value; 
	}
	return ret;
}


int a2_learn(int param)
{
 	if(param==1){
		if(data.fdata.amp_max_total > 1000)
			g_dd.freq[0].Level = int(data.fdata.amp_max_total) / 100 * 100; // Округляем до 100
		else
			g_dd.freq[0].Level = int(data.fdata.amp_max_total) / 10 * 10; // Округляем до 10
		print_line(logfile, data.sec, "a2_learn(%i) = %i", param, g_dd.freq[0].Level);
		data.fdata.amp_max_total = 0;
	} else {
		print_line(logfile, data.sec, "a2_learn(%i)", param);
	}
	return 0;
}

int a2_comment(const char * comment)
{
    if(comment){
	    print_line(logfile, data.sec, "%s", comment);
	}
	return 0;
}

#if defined WIN32

int a2_get_pauses(ZoneCPtr * p_pauses, int * p_pause_origin)
{
	*p_pauses = 0; 
	* p_pause_origin = data.sec - 2; //2019 5; //4
	return 0;
}

double ev_frame(const short * samples, int N, int LEN_MS)
{
	double av = 0;
	double * fsamples = new double[N];
	normalize(samples, N, fsamples);
	//av = amp_frame(fsamples, N, LEN_MS, g_dd.Freq);
	av = amp_frame(fsamples, N, LEN_MS, g_dd.freq[0].Freq1, g_dd.freq[0].Freq2);
	delete [] fsamples;
	return av;
}

#endif // defined WIN32
