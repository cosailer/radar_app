#ifndef MYDSP_H_
#define MYDSP_H_

#include <stdint.h>

#include <stdlib.h>
#include <cmath>

#include "myvar.h"
#include "interpolate.h"
#include "kalman.h"
#include "ssa.hpp"

//debug: output array
void disp( std::vector<double> &input )
{
    for(unsigned r = 0; r < input.size(); ++ r)
    {
        //printf( "%5.f, ", input[r] );
        if( input[r] > 0)
        {
            printf("[%d,%f], ", r, input[r] );
        }
    }
	
	printf("\n");
}

void clear_buffer( double target[][128] )
{
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            target[d][r] = 0;
        }
    }
}


int S = 255;
int D = 64; // range : 0 - 4D ( 0 - 256)

uint8_t clamp(int input)
{
    uint8_t output = 0;
    
    if(input <= 0)
    {
        output = 0;
    }
    else if(input <= D)
    {
        output = input*S/D;
    }
    else
    {
        output = S;
    }
    
    return output;
}

// get jet colormap value
uint32_t set_color_jet(uint8_t index)
{
    uint8_t color = index;
    
    uint8_t r = clamp(1.5*D - abs(color - 3*D));
    uint8_t g = clamp(1.5*D - abs(color - 2*D));
    uint8_t b = clamp(1.5*D - abs(color - D));
    
    return set_color( r, g, b );
}

// get hot-to-cold colormap value
uint32_t set_color_thermal(uint8_t index)
{
    uint8_t color = index;
    
    uint8_t r = clamp(2*D - abs(color - 4*D));
    uint8_t g = clamp(2*D - abs(color - 2*D));
    uint8_t b = clamp(2*D - abs(color));
    
    return set_color( r, g, b );
}

//copy from source to destination
void copy_int_int( int32_t source[][128], int32_t target[][128] )
{
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            target[d][r] = source[d][r];
        }
    }
}

//copy from source to destination
void copy_double_double( double source[][128], double target[][128] )
{
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            target[d][r] = source[d][r];
        }
    }
}

//copy from source to destination
void copy_to_pixel( double source[][128], int32_t target[][128] )
{
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            double tmp = source[d][r];
            tmp = display_contrast*tmp/0x0000FFFF;
            target[d][r] = set_color_jet((uint8_t)tmp);
        }
    }
}

//this function get the local maxima of a 1D array in input
void myGetPeak1D( std::vector<double> &input, int gap )
{
    for( int i = 0; i < (input.size()-gap); i++ )
    {
        if( input[i] <= 0) {  continue;  }
        
        // check the surrounding cell with size 'gap'
        for(int ii = i-gap; ii < i+gap+1; ii++)
        {
            if((ii < 0)||(ii > input.size())) {  continue;  }
            
            //if surrounding cell has a bigger value, make the detection 0
            if(input[ii] > input[i])
            {
                input[i] = 0;
                break;
            }
        }
        
    }
    
    for( int i = (input.size()-gap+1); i < input.size(); i++ )
    {
        input[i] = 0;
    }
    
}

//this function get the local maxima of a 2D image in radar_image_peak
void myGetPeak2D( double source[][128], int gap )
{
    int peak_mark = 1;
    
    //copy raw image to peak, and write only on peak
    copy_double_double( source, radar_image_peak );
    
    //for each point in fft_pixel_buffer
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            //if a target detection found, check gap cells around it 
            if(source[d][r] <= 1) {  continue;  }
            
            peak_mark = 1; //reset mark
            
            // check the surrounding cell with size 'gap'
            for(int dd = d-gap; dd < d+gap+1; dd++)
            {
                if((dd < 0)||(dd > 127)) {  continue;  }
                
                for(int rr = r-gap; rr < r+gap+1; rr++)
                {
                    //check boundary
                    if((rr < 0)||(rr > 63)) {  continue;  }
                    
                    //surrounding cell detection check
                    //if(source[dd][rr] <= 1) {  continue;  }
                    
                    //if surrounding cell has a bigger value, then this cell is not a peak
                    if(source[dd][rr] > source[d][r])
                    {
                        peak_mark = 0;
                        break;
                    }
                }
            }
            
            //if current cell is a peak, clear the surrounding cells
            if(peak_mark)
            {
                for(int dd = d-gap; dd < d+gap+1; dd++)
                {
                    if((dd < 0)||(dd > 127)) {  continue;  }
                    
                    for(int rr = r-gap; rr < r+gap+1; rr++)
                    {
                        //check boundary
                        if((rr < 0)||(rr > 63)) {  continue;  }
                        
                        //ship current cell
                        if( (dd==d)&&(rr==r) )  { continue;  }
                        
                        //surrounding cell detection check
                        //if(radar_image_peak[dd][rr] > 0)
                        {
                            radar_image_peak[dd][rr] = -1;
                        }
                    }
                }
            }
            else //current cell is not a peak, clear it
            {
                radar_image_peak[d][r] = -1;
            }
        }
    }
}

//ca-cfar evaluation for each peak in radar_image_peak
void myCFARPeak2D( int w, int h, double pfa)
{
    int guard = 1;
    double train_num = ( (w+guard)*2+1 ) * ( (h+guard)*2+1 ) - pow((guard*2+1),2);
    //int count = 0;
    
    int x_w = guard + w;
    int y_h = guard + h;
    
    double alpha = train_num *( pow(pfa, -1/train_num) - 1 );
    double pcfar = 0; //power of surrunding cell
    
    for(int x_main = x_w; x_main < 128-x_w+2; x_main++)
    {
        for(int y_main = y_h; y_main < 128-y_h+2; y_main++)
        {
            //skip if no peak found
            if( radar_image_peak[x_main][y_main] <= 0 ) { continue; }
            
            //count = 0;
            //sum up all surrunding cells
            for(int x_cell = x_main-x_w; x_cell < x_main+x_w+1; x_cell++)
            {
                for(int y_cell = y_main-y_h; y_cell < y_main+y_h+1; y_cell++)
                {
                    if( (x_cell>=x_main-guard)&&(x_cell<=x_main+guard)&&(y_cell>=y_main-guard)&&(y_cell<=y_main+guard) )
                    {  continue;  }
                    else
                    {
                        pcfar = pcfar + radar_image[x_cell][y_cell];
                        //count++;
                    }
                }
            }
            
            //calculate noise power
            pcfar = alpha*pcfar/train_num;
            
            //make detection decision
            if( radar_image_peak[x_main][y_main] < pcfar )
            {
                radar_image_peak[x_main][y_main] = 0;
            }
            
        }
    }
    
}

//apply threshold to radar_image_th
void myRemThreshold( double value)
{
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            if(radar_image_th[d][r] < value)
            {
                radar_image_th[d][r] = -1;
            }
        }
    }
}

//calculate normalized RCS signal in radar_image_th by multiply with r^2
void myNormRCS()
{
    for(int r = 1; r < 65; r++)
    {
        for(int d = 0; d < 128; d++)
        {
            radar_image_th[d][64-r] = radar_image[d][64-r]*r*r;
        }
    }
}

//apply mask from th1 to angle
void myNormAngle()
{
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            if(radar_image_peak[d][r] < 1)
            {
                radar_angle[d][r] = 0;
            }
        }
    }
}

//remove main clutter in radar_image
void myRemClutter()
{
    //remove mirror
    for(int d = 0; d < 128; d++)
    {
        for(int r = 64; r < 128; r++)
        {
            radar_image[d][r] = 0;
        }
    }
    
    //clear main cluster
    for(int r = 0; r < 61; r++)
    {
        radar_image[63][r] = 0;
        radar_image[64][r] = 0;
        radar_image[65][r] = 0;
    }
    
    //clear side clusters
    for(int d = 53; d < 75; d++)
    {
        radar_image[d][63] = 0;
    }
    
    for(int d = 56; d < 72; d++)
    {
        radar_image[d][62] = 0;
    }
    
    for(int d = 62; d < 66; d++)
    {
        radar_image[d][61] = 0;
    }
    
}

//remove environment noise in radar_image
void myRemEnvNoise()
{
    //minus the average envionment noise
    //calculate average buffer for each frame
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            radar_image[d][r] -= radar_image_env[d][r];
            
            if( radar_image[d][r] < 0 )
            {  radar_image[d][r] = 0;  }
        }
    }
}

//expand buffer from half image to full
void myExpandBuffer()
{
    //expand upper image to full image
    for(int x = 0; x < 128; x++)
    {
        for(int y = 63; y >= 0; y--)
        {
            display_pixel_buffer[x][2*y] = fft_pixel_buffer[x][y];
            display_pixel_buffer[x][2*y+1] = fft_pixel_buffer[x][y];
        }
    }
}

//separate the raw image by angle information
void mySplitAoA()
{
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            if( radar_angle[d][r] >= 0 )
            {
                radar_image_right[d][r] = radar_image[d][r];
            }
            else
            {
                radar_image_left[d][r] = radar_image[d][r];
            }
        }
    }
}

void calibration()
{
    //do calibration for maybe 20 frames.
    //not enough calibration frames
    if(calibration_finished == 0)
    {
        if(calibration_frame_count < NLEN)
        {
            // do calibration calculation and return
            // add each frame to buffer
            for(int d = 0; d < 128; d++)
            {
                for(int r = 0; r < 64; r++)
                {
                    radar_image_env[d][r] += radar_image[d][r];
                }
            }
            
            calibration_frame_count++;
            //myExpandBuffer();
            return;
        }
        else
        {
            //calculate average buffer
            for(int d = 0; d < 128; d++)
            {
                for(int r = 0; r < 64; r++)
                {
                    radar_image_env[d][r] /= NLEN;
                }
            }
            
            calibration_finished = 1;
        }
        
        //now try to find the main cluter
        
        //sum up range data
        std::vector<double> noise_sum(64, 0);
        
        for(int r = 0; r < 64; r++)
        {
            for(int d = 0; d < 128; d++)
            {
                noise_sum[r] += radar_image_env[d][r];
            }
            
            noise_sum[r] /= 64;
        }
        
        myGetPeak1D( noise_sum, 7 );
        
        //disp(noise_sum);
        //exit(0);
        
        //find peak index and enhance the noise
        for(int r = 0; r < 64; r++)
        {
            if( noise_sum[r] > 0)
            {
                for(int d = 0; d < 128; d++)
                {
                    radar_image_env[d][r-1] *= NR;
                    radar_image_env[d][r] *= NR;
                    radar_image_env[d][r+1] *= NR;
                }
            }
        }
        
    }
}

//get core_cycle, max_doppler_cycle, min_doppler_cycle,
//NDS, PDS, walk_mean_v, walk_acceleration
void myGetSSA()
{
    //for the extracted radar_image_th
    //sum up doppler data
    std::vector<double> doppler_sum(128, 0);
        
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            doppler_sum[d] += radar_image_extract[d][r];
        }
    }
    
    //get the max element
    double core_value = *max_element(doppler_sum.begin(), doppler_sum.end());
    int core_index = 0;
    int max_doppler_index = 0;
    int min_doppler_index = 0;
    
    if(core_value < 1) //no valid detection, skip
    {
        return;
    }
    
    //find the index of the first occurrence of the max element
    for(int d = 0; d < 128; d++)
    {
        if(doppler_sum[d] == core_value)
        {
            core_index = d;
            break;
        }
    }
    
    //find max doppler index
    for(int d = 128; d >= 0; d--)
    {
        if(doppler_sum[d] > 1)
        {
            max_doppler_index = d;
            break;
        }
    }
    
    //find min doppler index
    for(int d = 0; d < 128; d++)
    {
        if(doppler_sum[d] > 1)
        {
            min_doppler_index = d;
            break;
        }
    }
    
    //~ std::cout << max_doppler_index << ", " << min_doppler_index << std::endl;
    
    //update ssa buffer
    core.update_buffer(core_index);
    max_doppler.update_buffer(max_doppler_index, core_index);
    min_doppler.update_buffer(min_doppler_index, core_index);
    
}

//display ssa information on the sdl ssa panel
void show_ssa()
{
    //clear txt fields
    char str[64];
    
    //update display text
    add_txt( "features:", &feature_txt_local);
    
    sprintf( str, "core cycle: %2d    ", core.cycle );
    add_txt( str, &feature_core_local);
    
    sprintf( str, "max ds cycle: %2d    ", max_doppler.cycle );
    add_txt( str, &feature_maxds_local);
    
    sprintf( str, "min ds cycle: %2d    ", min_doppler.cycle );
    add_txt( str, &feature_minds_local);
    
    sprintf( str, "walk mean: %2.1f    ", core.walk_mean_v );
    add_txt( str, &feature_v_mean_local);
    
    sprintf( str, "PDS: %2d    ", max_doppler.max_doppler_pds );
    add_txt( str, &feature_pds_local);
    
    sprintf( str, "NDS: %2d    ", min_doppler.min_doppler_nds );
    add_txt( str, &feature_nds_local);
}

void show_coordinate()
{
    //display txt on coordinate panel
    char str[64];
    
    //update display text
    add_txt( "coordinates:", &coordinate_txt_local);
    
    sprintf( str, "x = %2d    ", track_one.current_x );
    add_txt( str, &coordinate_x_local);

    sprintf( str, "y = %2d    ", track_one.current_y );
    add_txt( str, &coordinate_y_local);

    sprintf( str, "r = %2d    ", track_one.current_r );
    add_txt( str, &coordinate_r_local);

    sprintf( str, "d = %2d    ", track_one.current_d );
    add_txt( str, &coordinate_d_local);

    sprintf( str, "phi = %2.1f deg   ", track_one.current_phi*180/M_PI );
    add_txt( str, &coordinate_phi_local);

}

void show_target_num()
{
    //display txt on coordinate panel
    char str[64];
    
    sprintf( str, "tracked target number : %2d    ", tracks_all.tracked_num );
    add_txt( str, &coordinate_txt_local);
}

//raw sample mode
void image_process_raw()
{
    copy_to_pixel( radar_image, fft_pixel_buffer );
    myExpandBuffer();
}
//image pre-processing
void image_process_single()
{
    //remove environment noise
    myRemEnvNoise();
    
    //copy to radar_image_th and apply Th1
    copy_double_double( radar_image, radar_image_th );
    myRemThreshold(Th1);
    
    //find local maxima in radar_image_th,
    //and save them in radar_image_peak
    myGetPeak2D( radar_image_th, g );
    
    //apply cfar evaluations to the local maxima in radar_image_peak using radar_image
    myCFARPeak2D(pattern_w,pattern_h,pfa);
    
    //copy_double_double( radar_image_peak, radar_image );
    
    //normalize angle with mask from Th1
    //~ myNormAngle();
    
    //single target kalman tracking
    myKalmanTracker_single();
    
    //enhance radar image
    myNormRCS();
    myRemThreshold(Th2);
    
    //extract single track to spectrogram
    track_one.extract_track();
    
    myGetSSA();
    show_ssa();
    show_coordinate();
    
    //expand fft_buffer to display_buffer
    copy_to_pixel( radar_image_extract, fft_pixel_buffer );
    myExpandBuffer();
    
    clear_buffer(radar_image_extract);
}


void image_process_multi()
{
    //remove environment noise
    myRemEnvNoise();
    
    //angle separation
    //mySplitAoA();
    
    //copy to radar_image_th and apply Th1
    copy_double_double( radar_image, radar_image_th );
    myRemThreshold(Th1);
    
    //find local maxima in radar_image_th,
    //and save them in radar_image_peak
    myGetPeak2D( radar_image_th, g );
    
    //cfar evaluation
    myCFARPeak2D(pattern_w,pattern_h,pfa);
    
    //multi target kalman tracking
    myKalmanTracker_multi();
    
    //enhance radar image
    myNormRCS();
    myRemThreshold(Th2);
    
    //extract multi-tracks to spectrogram
    for(int i = 0; i < tracks_all.tracks.size(); i++)
    {
        if( tracks_all.tracks[i].track_state == 1 )
        {
            tracks_all.tracks[i].extract_track();
        }
    }
    
    show_target_num();
    
    copy_to_pixel( radar_image_extract, fft_pixel_buffer );
    myExpandBuffer();
    
    clear_buffer(radar_image_extract);
}


#endif
