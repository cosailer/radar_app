#ifndef MYFFT_H_
#define MYFFT_H_

#include <stdint.h>

//fft
//#include <complex.h> 
#include <fftw3.h>
#include <stdlib.h>
#include <math.h>

#include "myvar.h"
#include "interpolate.h"

// calculate the magnitude of radar_image_fft2_1
void calculate_raw_image( fftw_complex array[128][128] )
{
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 128; r++)
        {
            double real = array[d][r][0];
            double img  = array[d][r][1];
            
            radar_image[d][r] = sqrt( real*real + img*img );
        }
    }
}

// apply hanning window to fft data
void fft_window( fftw_complex array[128][128] )
{
    for(int x = 0; x < 128; x ++)
    {
        for(int y = 0; y < 128; y++)
        {
            array[x][y][0] *= (hanning[x]*hanning[y]); //real data
            array[x][y][1] *= (hanning[x]*hanning[y]); //img  data
        }
    }
}

// A B      D C
// C D  to  B A
void fftshift()
{
    int N = 128;
    int x_des = 0;
    int y_des = 0;
    fftw_complex tmp;
    
    for(int x = 0; x < N; x++)
    {
        for(int y = 0; y < (N/2); y++)
        {
            x_des = (x+(N/2))%N;
            y_des = (y+(N/2))%N;
            
            //shift for fft2_1
            tmp[0] = radar_image_fft2_1[x_des][y_des][0];
            tmp[1] = radar_image_fft2_1[x_des][y_des][1];
            radar_image_fft2_1[x_des][y_des][0] = radar_image_fft2_1[x][y][0];
            radar_image_fft2_1[x_des][y_des][1] = radar_image_fft2_1[x][y][1];
            radar_image_fft2_1[x][y][0] = tmp[0];
            radar_image_fft2_1[x][y][1] = tmp[1];
            
            //shift for fft2_2
            tmp[0] = radar_image_fft2_2[x_des][y_des][0];
            tmp[1] = radar_image_fft2_2[x_des][y_des][1];
            radar_image_fft2_2[x_des][y_des][0] = radar_image_fft2_2[x][y][0];
            radar_image_fft2_2[x_des][y_des][1] = radar_image_fft2_2[x][y][1];
            radar_image_fft2_2[x][y][0] = tmp[0];
            radar_image_fft2_2[x][y][1] = tmp[1];
            
            //~ //fftshift for radar_image
            //~ double tmp = radar_image[x_des][y_des];
            //~ radar_image[x_des][y_des] = radar_image[x][y];
            //~ radar_image[x][y] = tmp;
            
            //~ //fftshift for radar_angle
            //~ tmp = radar_angle[x_des][y_des];
            //~ radar_angle[x_des][y_des] = radar_angle[x][y];
            //~ radar_angle[x][y] = tmp;
        }
    }
}


//convert all int16 adc data into complex data for fft
void adc2fft()
{
    for(int x = 0; x < 128; x ++)
    {
        for(int y = 0; y < 128; y++)
        {
            //adc_1
            radar_image_adc_1[x][y][0] = radar_adc_1[x][y]; //real data
            radar_image_adc_1[x][y][1] = 0;                 //img  data
            
            //adc_2
            radar_image_adc_2[x][y][0] = radar_adc_2[x][y]; //real data
            radar_image_adc_2[x][y][1] = 0;                 //img  data
        }
    }
}

//~ //buffer matrix transpose operation
//~ void fft_pixel_buffer_transpose()
//~ {
    //~ uint32_t tmp = 0;
    
    //~ for(int x = 0; x < 128; x ++)
    //~ {
        //~ for(int y = 0; y < 128; y++)
        //~ {
            //~ if(x > y)
            //~ {
                //~ tmp = fft_pixel_buffer[x][y];
                //~ fft_pixel_buffer[x][y] = fft_pixel_buffer[y][x];
                //~ fft_pixel_buffer[y][x] = tmp;
            //~ }
        //~ }
    //~ }
//~ }

//for each point in radar_image_fft2_1 & radar_image_fft2_2
//calculate angle information
void myCalcAngle()
{
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            //~ radar_image_fft2_1[x][y][0]; //real data 0
            //~ radar_image_fft2_1[x][y][1]; //img  data 0
            
            //~ radar_image_fft2_2[x][y][0]; //real data 1
            //~ radar_image_fft2_2[x][y][1]; //img  data 1
            
            //~ double real = (double)real0*(double)real1 + (double)img0*(double)img1;
            //~ double img  = (double)img0*(double)real1 - (double)img1*(double)real0;
            
            //use atan2 to calculate phase difference
            double real = radar_image_fft2_1[d][r][0]*radar_image_fft2_2[d][r][0] + radar_image_fft2_1[d][r][1]*radar_image_fft2_2[d][r][1];
            double img  = radar_image_fft2_1[d][r][1]*radar_image_fft2_2[d][r][0] - radar_image_fft2_1[d][r][0]*radar_image_fft2_2[d][r][1];
            
            double angle = atan2(img, real);// + ANGLE_OFFSET;
            
            //~ double angle = atan2(img, real) + M_PI*sin(8*M_PI/180);
            
            //set angle quadrant
            if( angle > (M_PI)){  angle -= M_PI2; }
            else if( angle < (-M_PI)){  angle += M_PI2; }
            
            //Arcus sinus (-Pi/2 to Pi/2), input= -1..1
            angle = asin(angle/M_PI);
            
            //conver to degree? Angle (-90...90)
            //angle = angle*180/M_PI;
            
            //apply interpolation to get more accurate angle
            radar_angle[d][r] = interpolate( angle );
            //~ radar_angle[d][r] = angle;
        }
    }
}

double myGetAngle( int input_r, int input_d, int gap )
{
    double average = 0;
    int count = 0;
    
    for(int d = input_d-gap; d < input_d+gap+1; d++)
    {
        if( (d < 0)||(d > 127) ) {  continue;  }
        
        for(int r = input_r-gap; r < input_r+gap+1; r++)
        {
            if( (r < 0)||(r > 127) ) {  continue;  }
            if( radar_angle[d][r] == 0 ) {  continue;  }
            
            average += radar_angle[d][r];
            count++;
        }
    }
    
    average = average/count;
    return average;
    //~ return radar_angle[input_d][input_r];
}

// 2d fft operation
void fft2_angle()
{
    int N = 128;
    
    fftw_plan fft1, fft2;
    
    // copy data to fft buffer
    adc2fft();
    
    // fft window
    fft_window(radar_image_adc_1);
    fft_window(radar_image_adc_2);
    
    fft1 = fftw_plan_dft_2d(N, N, &radar_image_adc_1[0][0], &radar_image_fft2_1[0][0], FFTW_FORWARD, FFTW_ESTIMATE);
    fft2 = fftw_plan_dft_2d(N, N, &radar_image_adc_2[0][0], &radar_image_fft2_2[0][0], FFTW_FORWARD, FFTW_ESTIMATE);
    
    fftw_execute(fft1);
    fftw_execute(fft2);
    fftw_destroy_plan(fft1);
    fftw_destroy_plan(fft2);
    
    //do fftshift on radar_image_fft2_1 and radar_image_fft2_2
    fftshift();
    
    //angle calculation
    myCalcAngle();
    
    //get radar_image
    calculate_raw_image(radar_image_fft2_1);
    
    //copy radar_image to fft_pixel_buffer
    //copy_to_pixel( radar_image, fft_pixel_buffer );
}



#endif
