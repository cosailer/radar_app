#ifndef MYVAR_H_
#define MYVAR_H_

#include <fftw3.h>

//sdl
#define BASE   128
#define scale  2
#define WINDOW_SIZE 128

//setup
//track modes:
//0: raw sample mode
//1: single target mode
//2: multi-target mode
static int track_mode = 1;
static int C_Max = 16;
static int C_Min = 5;
static double Th1 = 5e2;
static double Th2 = 1e5;

static double NR = 2;
static int NLEN = 20;

static double g = 7;

static double pattern_w = 15;
static double pattern_h = 5;
static double pattern_a = 5;
static double pfa = 1e-8;

static double gate = 5*5;

//track state variables setup
static int A0LEN = 30;
static int P0LEN = 10;
static int P1LEN = 50;
static int E1LEN = 5;


//buffers
static double hanning[WINDOW_SIZE];
static double time_begin = 0;

static int FFT_WINDOW_WIDTH  = BASE*scale;
static int FFT_WINDOW_HEIGHT = BASE*scale;

static int SPEC_WINDOW_WIDTH  = BASE*4+10;
static int SPEC_WINDOW_HEIGHT = BASE*2;

static int WINDOW_WIDTH  = BASE*4 + 680;
static int WINDOW_HEIGHT = BASE*4 + 70;


#define SPEC_LEN (256+5)


//pixel buffer
static int32_t   display_pixel_buffer[128][128];     //buffer for display
static int32_t   fft_pixel_buffer[128][128];         //raw radar image buffer
static int32_t  spec_pixel_buffer[SPEC_LEN][128];    //spectrogram buffer
static int32_t     spec_pixel_tmp[SPEC_LEN][128];


//radar frames           [d]  [r]
static double radar_image[128][128];        //original radar frame

static double radar_image_th[128][128];       //with threshold
static double radar_image_env[128][128];      //environment noise
static double radar_image_left[128][128];     //left image negative angle
static double radar_image_right[128][128];    //right image positive angle
static double radar_image_peak[128][128];     //2d peak image
static double radar_image_extract[128][128];  //extracted pattern

//map buffer
//~ static int32_t    map_pixel_buffer[640][320];
static int32_t   map_target_buffer[128][64];

static int32_t  range_pixel_buffer[128][128];
static int32_t     range_pixel_tmp[128][128];

//local temp radar image
static int16_t radar_adc_1[128][128];
static int16_t radar_adc_2[128][128];



#define M_PI                  3.14159265358979323846
#define M_PI2                 2*3.14159265358979323846
#define ANGLE_OFFSET          0.970805519362733  //(M_PI*sin(18/180*M_PI))

//complex adc data
static fftw_complex radar_image_adc_1[128][128];
static fftw_complex radar_image_adc_2[128][128];

//fft 1 data
//fftw_complex radar_image_fft1_1[128][128];
//fftw_complex radar_image_fft1_2[128][128];

//fft 1 data transpose
//fftw_complex radar_image_fft1_1_t[128][128];

//fft 2 data
static fftw_complex radar_image_fft2_1[128][128];
static fftw_complex radar_image_fft2_2[128][128];

//angle information from fft2_1 and fft2_2, in rad
static double  radar_angle[128][128];

static  int count = 1;
static long sec_last = 0;

static  int display_contrast = 2048;
static  int fps = 0;

static long frame_count = 0;
static  int status = 1; // 1: viewing, 2: recording, 3: paused
static  int flag_program_start = 0;
static  int flag_record_start = 0;
static  int record_count = 0;

static  int calibration_finished = 0;
static  int calibration_frame_count = 0;

static char dev_eth[] = "enp0s25";
//char dev_eth[] = "enx00249b1553d4";
static char filter_exp[] = "udp port 49152";


//set color
static uint32_t set_color(uint8_t r, uint8_t g, uint8_t b)
{
    int color = ((uint32_t)r<<16) + ((uint32_t)g<<8) + (uint32_t)b;

    return color;
}

#endif
