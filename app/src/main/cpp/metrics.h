#pragma once

#define SamplesPerSec 16000

#define ms2n(ms) (ms)*SamplesPerSec/1000 
#define n2ms(n) (n)*1000/SamplesPerSec

