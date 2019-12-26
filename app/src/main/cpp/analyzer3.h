#pragma once

#include "a2.h"

enum A2_Param
{
	// 11+
	a2_param_state=11,
	a2_param_zone2_len,
 	a2_param_av_1sec,
	a2_param_av_zone1,
	
	a2_15,
	a2_16,
	a2_answer,
	a2_signal,
	
	a2_param_zone2_start, //ms    19
    a2_param_zone2_stop, //ms	  20

	a2_param_av_total,
    a2_param_n_pause,
	a2_param_n_pause_per_5sec_zone1,

	a2_param_silence_start,

	// Analyzer 3
	a2_param_zone2_start1, 
	a2_param_zone2_stop1, 
	a2_param_zone2_max_freq, 

	a2_param_number
};

#if defined WIN32

//#include <tchar.h>
//int a2_file(const TCHAR * filepath, bool b_play, bool b_stop_at_zone2); // файл для анализа
//int a2_file(const char * filepath, bool b_play, bool b_stop_at_zone2); 

#include "Analyze_Sound.h"

int a2_get_pauses(ZoneCPtr * p_pauses, int * p_pause_origin);
//double ev_frame(const short * samples, int N, int LEN_MS);

#endif
