#ifndef MYSDL_H_
#define MYSDL_H_

//sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "myvar.h"

// sdl section
SDL_Event event;

//SDL_Renderer *renderer = NULL;
//SDL_Texture  *texture  = NULL;
SDL_Window   *window     = NULL;
SDL_Surface  *surface    = NULL;
SDL_Surface  *rvwindow   = NULL;
SDL_Surface  *specwindow = NULL;
SDL_Surface  *rtwindow   = NULL;
SDL_Surface  *message    = NULL;
SDL_Surface  *mapwindow  = NULL;

SDL_Color  back_color = {0xAF, 0xAF, 0xAF};
SDL_Color  text_color = {0,    0,    0};
SDL_Color  red   = {0xFF, 0,    0};
SDL_Color  green = {0,    0xFF, 0};
SDL_Color  blue  = {0,    0,    0xFF};

//fft section
SDL_Rect   fft_local;
SDL_Rect   fft_txt_local;
SDL_Rect   spec_local;
SDL_Rect   spec_txt_local;
SDL_Rect   range_local;
SDL_Rect   range_txt_local;

//map section
SDL_Rect   map_local;
SDL_Rect   map_txt_local;
SDL_Rect   map_time_local;
SDL_Rect   map_error_local;
SDL_Rect   status_local;
SDL_Rect   mapview_local;

//map scale section
SDL_Rect   local_m10;
SDL_Rect   local_m20;
SDL_Rect   local_m30;
SDL_Rect   local_0;
SDL_Rect   local_10;
SDL_Rect   local_20;
SDL_Rect   local_30;

//coordinate section
SDL_Rect   coordinate_txt_local;
SDL_Rect   coordinate_x_local;
SDL_Rect   coordinate_y_local;
SDL_Rect   coordinate_r_local;
SDL_Rect   coordinate_d_local;
SDL_Rect   coordinate_phi_local;

//feature section
SDL_Rect   feature_txt_local;
SDL_Rect   feature_core_local;
SDL_Rect   feature_v_mean_local;
SDL_Rect   feature_maxds_local;
SDL_Rect   feature_minds_local;
SDL_Rect   feature_pds_local;
SDL_Rect   feature_nds_local;

//target number section
//~ SDL_Rect   target_num_local;

void sdl_init()
{
    SDL_Init( SDL_INIT_VIDEO );
    TTF_Init();
    
    //SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer );
    
    window = SDL_CreateWindow( "radar tracker v0.5",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               WINDOW_WIDTH,
                               WINDOW_HEIGHT,
                               SDL_WINDOW_SHOWN );
    
    //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    surface = SDL_GetWindowSurface( window );
    SDL_FillRect( surface, NULL, SDL_MapRGB( surface->format, 0xAF, 0xAF, 0xAF ) );
    
    rvwindow = SDL_CreateRGBSurface(0, FFT_WINDOW_WIDTH, FFT_WINDOW_HEIGHT, 32, 0, 0, 0, 0);
    specwindow = SDL_CreateRGBSurface(0, SPEC_WINDOW_WIDTH, SPEC_WINDOW_HEIGHT, 32, 0, 0, 0, 0);
    rtwindow = SDL_CreateRGBSurface(0, FFT_WINDOW_WIDTH, FFT_WINDOW_HEIGHT, 32, 0, 0, 0, 0);
    
    mapwindow = SDL_CreateRGBSurface(0, 640, 320, 32, 0, 0, 0, 0);
    
    //left panel
    fft_txt_local.x = 10;
    fft_txt_local.y = 10;
    
    fft_local.x = 10;
    fft_local.y = 30;
    
    spec_txt_local.x = 10;
    spec_txt_local.y = 256+40;
    
    spec_local.x = 10;
    spec_local.y = 256+60;
    
    range_txt_local.x = 256+20;
    range_txt_local.y = 10;
    
    range_local.x = 256+20;
    range_local.y = 30;
    
    //right panel
    map_txt_local.x = 512+30;
    map_txt_local.y = 10;
    
    map_time_local.x = 512+30;
    map_time_local.y = 40;
    
    status_local.x = 512+450;
    status_local.y = 40;
    
    mapview_local.x = 512+30;
    mapview_local.y = 90;
    
    map_local.x = 512+30;
    map_local.y = 62+50;
    
    map_error_local.x = 512+320;
    map_error_local.y = 32+250;
    
    //map scale
    local_0.x = 512+30+200+115;
    local_0.y = 62+50+320;
    
    local_10.x = 512+30+200+100+60;
    local_10.y = 62+50+320;
    
    local_20.x = 512+30+200+100+110;
    local_20.y = 62+50+320;
    
    local_30.x = 512+30+200+100+160;
    local_30.y = 62+50+320;
    
    local_m10.x = 512+30+200+100-50;
    local_m10.y = 62+50+320;
    
    local_m20.x = 512+30+200+100-100;
    local_m20.y = 62+50+320;
    
    local_m30.x = 512+30+200+100-150;
    local_m30.y = 62+50+320;
    
    //coordinate panel
    coordinate_txt_local.x = 512+30;
    coordinate_txt_local.y = 62+50+320+10+20;
    
    coordinate_x_local.x = 512+30;
    coordinate_x_local.y = 62+50+320+25+20;
    
    coordinate_y_local.x = 512+30;
    coordinate_y_local.y = 62+50+320+40+20;
    
    coordinate_r_local.x = 512+30;
    coordinate_r_local.y = 62+50+320+55+20;
    
    coordinate_d_local.x = 512+30;
    coordinate_d_local.y = 62+50+320+70+20;
    
    coordinate_phi_local.x = 512+30;
    coordinate_phi_local.y = 62+50+320+85+20;
    
    //feature panel
    feature_txt_local.x = 512+30+200;
    feature_txt_local.y = 62+50+320+10+20;
    
    feature_core_local.x = 512+30+200;
    feature_core_local.y = 62+50+320+25+20;
    
    feature_maxds_local.x = 512+30+200;
    feature_maxds_local.y = 62+50+320+40+20;
    
    feature_minds_local.x = 512+30+200;
    feature_minds_local.y = 62+50+320+55+20;
    
    feature_v_mean_local.x = 512+30+200;
    feature_v_mean_local.y = 62+50+320+70+20;
    
    feature_pds_local.x = 512+30+200;
    feature_pds_local.y = 62+50+320+85+20;
    
    feature_nds_local.x = 512+30+200;
    feature_nds_local.y = 62+50+320+100+20;
}

void add_cam_txt( uint32_t sec, uint32_t usec )
{
    TTF_Font *font = TTF_OpenFont("FreeMono.ttf", 20);
    
    if( font==NULL ){ printf("\n>> FreeMono.ttf not found, exiting\n"); exit(1); }
    //TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    
    char str[64];
    
    //gettimeofday( &tmsec, NULL );
    
    if(sec == sec_last) { count++; }
    else
    {
        fps = count;
        count = 1;
        sec_last = sec;
    }
    
    sprintf(str, "TIME:%d.%06d  FPS:%02d  NET:%s", sec, usec, fps, dev_eth);
    message = TTF_RenderText_Shaded(font, str, text_color, back_color);
    SDL_BlitSurface( message, NULL, surface, &map_txt_local);
    
    double time_current = sec + usec*1e-6 - time_begin;
    
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    sprintf(str, "[ %0.2f s ]   ", time_current );
    message = TTF_RenderText_Shaded(font, str, text_color, back_color);
    SDL_BlitSurface( message, NULL, surface, &map_time_local);
    
    SDL_FreeSurface(message);
    TTF_CloseFont(font);
}

void add_txt( const char *txt, SDL_Rect *location)
{
    TTF_Font *font = TTF_OpenFont("FreeMono.ttf", 15);
    
    if( font==NULL ){ printf("\n>> FreeMono.ttf not found, exiting\n"); exit(1); }
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    
    message = TTF_RenderText_Shaded(font, txt, text_color, back_color);
    
    SDL_BlitSurface( message, NULL, surface, location);
    
    SDL_FreeSurface(message);
    TTF_CloseFont(font);
}


void add_status( const char *txt )
{
    TTF_Font *font = TTF_OpenFont("FreeMono.ttf", 20);
    
    if( font==NULL ){ printf("\n>> FreeMono.ttf not found, exiting\n"); exit(1); }
    
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    
    message = TTF_RenderText_Shaded(font, txt, red, back_color);
    
    SDL_BlitSurface( message, NULL, surface, &status_local);
    
    SDL_FreeSurface(message);
    TTF_CloseFont(font);
}

//key input, p = pause, q = exit ...
void check_key_input()
{
    if( SDL_PollEvent(&event) )
    {
        if( event.type == SDL_QUIT )
        {
            printf("\n>> window closed, %ld frames captured\n", frame_count);
            exit(0);
        }
        
        else if(event.type == SDL_KEYDOWN)
        {
            switch( event.key.keysym.sym )
            {
              case SDLK_r: // start recording
                if( status == 1 )
                {
                    status = 2;
                    //create_record();
                    //add_status( "[recording]" );
                }
                else if( status == 2 )
                {
                    status = 1;
                    //finish_record();
                    add_status( "[viewing]  " );
                }
                break;
              /*
              case SDLK_f: // finish recording
                if( status == 2 )
                {
                    status = 1;
                    finish_record();
                }
                add_status( "[viewing]  " );
                break;
              */
              
              case SDLK_p: // pause
                if( status == 1 )
                {
                    status = 3;
                    add_status( "[paused]   " );
                }
                else if ( status == 3 )
                {
                    status = 1;
                    add_status( "[viewing]  " );
                }
                break;
                    
              case SDLK_q:
                printf("\n>> window closed, %ld frames captured\n", frame_count);
                exit(0);
                break;
                
              default:
                ;
            }
        }
        
        SDL_UpdateWindowSurface( window );
    }
}

//press q key to quit
void press_q_quit()
{
    if( SDL_PollEvent(&event) )
    {
        if( event.type == SDL_QUIT )
        {
            printf("\n>> window closed, %ld frames captured\n", frame_count);
            exit(0);
        }
        
        if(event.type == SDL_KEYDOWN)
        {
            switch( event.key.keysym.sym )
            {
                    
              case SDLK_q:
                printf("\n>> window closed, %ld frames captured\n", frame_count);
                exit(0);
                break;
                
              default:
                ;
            }
        }
        
    }
    
    SDL_UpdateWindowSurface( window );
}

//expand small buffer into bigger sdl surface
void expand_surface(SDL_Surface *surface, int32_t buffer[][128], int window_width)
{
    //for each pixel in the 128x128 buffer
    //expand the window
    for(int x = 0; x < window_width; x++)
    {
        for(int y = 0; y < 128; y++)
        {
            //random color
            //buffer[x][y] = rand()*10000;//set_color( rand()%0xFF, rand()%0xFF, rand()%0xFF);
            
            //draw the pixel into scale version
            for(int v = 0; v < scale; v++) // vertical scale
            {
                for(int h = 0; h < scale; h++) //horizental scale
                {
                    //if( buffer[x][y] == 0) { buffer[x][y] = set_color(0x3F,0x3F,0x3F); }
                    
                    uint32_t pixel_index = (scale*x)+(window_width*2)*(scale*y) +h + v*(window_width*2);
                    *((uint32_t *)surface->pixels+ pixel_index) = buffer[x][y];
                }
            }
        }
    }
}

///expand map buffer into map surface
void expand_map(SDL_Surface *surface)
{
    int scale2 = 5;
    //for each pixel in the 128x128 buffer
    //expand the window
    for(int x = 0; x < 128; x++)
    {
        for(int y = 0; y < 64; y++)
        {
            //draw the pixel into scale version
            for(int v = 0; v < scale2; v++) // vertical scale
            {
                for(int h = 0; h < scale2; h++) //horizental scale
                {
                    uint32_t pixel_index = (scale2*x)+(640)*(scale2*y) +h + v*(640);
                    *((uint32_t *)surface->pixels+ pixel_index) = map_target_buffer[x][y];
                }
            }
        }
    }
    
    //add map vertical lines
    for(int y = 0; y < 320; y++)
    {
        *((uint32_t *)surface->pixels + 320 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //0
        *((uint32_t *)surface->pixels + 370 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //10
        *((uint32_t *)surface->pixels + 420 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //20
        *((uint32_t *)surface->pixels + 470 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //30
        *((uint32_t *)surface->pixels + 520 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //40
        *((uint32_t *)surface->pixels + 570 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //50
        *((uint32_t *)surface->pixels + 620 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //60
        
        *((uint32_t *)surface->pixels + 270 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //-10
        *((uint32_t *)surface->pixels + 220 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //-20
        *((uint32_t *)surface->pixels + 170 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //-30
        *((uint32_t *)surface->pixels + 120 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //-40
        *((uint32_t *)surface->pixels +  70 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //-50
        *((uint32_t *)surface->pixels +  20 + 640*y) = set_color( 0x8F, 0x8F, 0x8F ); //-60
    }
    
    //add map horizontal lines
    for(int x = 0; x < 640; x++)
    {
        *((uint32_t *)surface->pixels + x + 640*20) = set_color( 0x8F, 0x8F, 0x8F ); //60
        *((uint32_t *)surface->pixels + x + 640*70) = set_color( 0x8F, 0x8F, 0x8F ); //50
        *((uint32_t *)surface->pixels + x + 640*120) = set_color( 0x8F, 0x8F, 0x8F ); //40
        *((uint32_t *)surface->pixels + x + 640*170) = set_color( 0x8F, 0x8F, 0x8F ); //30
        *((uint32_t *)surface->pixels + x + 640*220) = set_color( 0x8F, 0x8F, 0x8F ); //20
        *((uint32_t *)surface->pixels + x + 640*270) = set_color( 0x8F, 0x8F, 0x8F ); //10
    }
    
}

void set_map(uint8_t r, uint8_t g, uint8_t b)
{
    //for each pixel in the 640x480 map buffer
    for(int x = 0; x < 128; x++)
    {
        for(int y = 0; y < 64; y++)
        {
            map_target_buffer[x][y] = set_color( r, g, b);
        }
    }
    
}


//clear the content in surface
void clear_surface(SDL_Surface *surface)
{
    for(int x = 0; x < 128*scale; x++)
    {
        for(int y = 0; y < 128*scale; y++)
        {
             uint32_t pixel_index = x + BASE*scale*y;
             *((uint32_t *)surface->pixels+ pixel_index) = 0;
        }
    }
}

static int frame_index = 0;

void calc_spectrogram()
{
    uint32_t max[128];
    
    //reset frame_index
    //if(frame_index == 256) { frame_index = 0; }
    frame_index %= SPEC_LEN;
    
    //get the max doppler
    for(int x = 0; x < 128; x++)
    {
        max[x] = 0;
        
        for( int y = 0; y < 128; y++)
        {
            //sum[x] = sum[x] + fft_pixel_buffer[x][y];
            if( max[x] < display_pixel_buffer[x][y] )
            { max[x] = display_pixel_buffer[x][y]; }
        }
        
        //add the reversed spectrogram
        spec_pixel_tmp[frame_index][128-x] = max[x];
    }
    
    //make the spec_pixel_buffer continues
    for(int x = 0; x < SPEC_LEN; x++)
    {
        for(int y = 0; y < 128; y++)
        {
            spec_pixel_buffer[x][y] = spec_pixel_tmp[ (x+frame_index+1)%SPEC_LEN ][y];
        }
    }
    
    frame_index++;
}



static int range_index = 0;

void calc_range_plot()
{
    uint32_t max[128];
    
    //reset frame_index
    //if(frame_index == 256) { frame_index = 0; }
    range_index %= 128;
    
    //remove mirror from range-doppler plot
    //delete_mirror();
    
    //get the max doppler
    for(int y = 0; y < 128; y++)
    {
        max[y] = 0;
        
        for( int x = 0; x < 128; x++)
        {
            //sum[x] = sum[x] + fft_pixel_buffer[x][y];
            if( max[y] < display_pixel_buffer[x][y] )
            { max[y] = display_pixel_buffer[x][y]; }
        }
        
        range_pixel_tmp[range_index][y] = max[y];
        //range_pixel_buffer[range_index][y] = max[y];
    }
    
    //make the spec_pixel_buffer continues
    for(int x = 0; x < 128; x++)
    {
        for(int y = 0; y < 128; y++)
        {
            range_pixel_buffer[x][y] = range_pixel_tmp[ (x+range_index+1)%128 ][y];
        }
    }
    
    range_index++;
}

void draw_map_scale()
{
    add_txt( "0", &local_0);
    add_txt( "10", &local_10);
    add_txt( "20", &local_20);
    add_txt( "30", &local_30);
    add_txt( "-10", &local_m10);
    add_txt( "-20", &local_m20);
    add_txt( "-30", &local_m30);
}

#endif
