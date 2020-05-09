// this program takes udp packets from infineon radar kit
// and perform 2d fft with fftw3
// then display the range-doppler plot using sdl2
// when passing a folder name to the program, it will record
// all packets to the folder as well as the pictures
// that are synced with them

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

//pcap
#include <pcap.h>

//sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

//opencv
#include <opencv2/core/core_c.h>
#include <opencv2/opencv.hpp>
//#include <opencv2/imgproc/imgproc_c.h>
//#include <opencv2/videoio/videoio_c.h>
//#include <opencv2/imgproc/imgproc.hpp>

//fft
//#include <complex.h> 
#include <fftw3.h>

#define WINDOW_SIZE 128

//sdl
#define BASE   128
#define scale  2

static double hanning[WINDOW_SIZE];
static double time_begin = 0;

static int FFT_WINDOW_WIDTH  = BASE*scale;
static int FFT_WINDOW_HEIGHT = BASE*scale;

static int SPEC_WINDOW_WIDTH  = BASE*4+10;
static int SPEC_WINDOW_HEIGHT = BASE*2;

static int WINDOW_WIDTH  = BASE*4 + 680;
static int WINDOW_HEIGHT = BASE*4 + 70;

//time variable
time_t rawtime;
struct tm * timeinfo;
struct timeval tmsec;

//file variable
FILE *packet_file = NULL;
char record_name[64];
char file_name[64];

char folder_name[64];
char image_name[64];


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
SDL_Surface  *camera     = NULL;

SDL_Color back_color = {0xAF, 0xAF, 0xAF};
SDL_Color text_color = {0,    0,    0};
SDL_Color red   = {0xFF, 0,    0};
SDL_Color green = {0,    0xFF, 0};
SDL_Color blue  = {0,    0,    0xFF};
SDL_Rect  fft_local;
SDL_Rect  fft_txt_local;
SDL_Rect  spec_local;
SDL_Rect  spec_txt_local;
SDL_Rect  range_local;
SDL_Rect  range_txt_local;
SDL_Rect  cam_local;
SDL_Rect  cam_txt_local;
SDL_Rect  cam_time_local;
SDL_Rect  cam_error_local;
SDL_Rect  status_local;

static  int count = 1;
static  int fps = 0;
static long sec_last = 0;
static long frame_count = 0;
static  int status = 1; // 1: viewing, 2: recording, 3: paused
static  int flag_program_start = 0;
static  int flag_record_start = 0;
static  int record_count = 0;

// opencv
cv::Mat frame;
cv::VideoCapture cap;
int dev_cam = 2;

char dev_eth[] = "enp0s25";
char filter_exp[] = "udp port 49152";

void sdl_init()
{
    SDL_Init( SDL_INIT_VIDEO );
    TTF_Init();
    
    //SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer );
    
    window = SDL_CreateWindow( "radar viewer v0.1",
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
    cam_txt_local.x = 512+30;
    cam_txt_local.y = 30;
    
    cam_time_local.x = 512+30;
    cam_time_local.y = 60;
    
    status_local.x = 512+450;
    status_local.y = 60;
    
    cam_local.x = 512+30;
    cam_local.y = 30+62;
    
    cam_error_local.x = 512+320;
    cam_error_local.y = 32+250;
    
}

SDL_Surface* MatToSurface(const cv::Mat &matImg)
{
    IplImage opencvimg2 = (IplImage)matImg;
    IplImage* opencvimg = &opencvimg2;

     //Convert to SDL_Surface
    SDL_Surface *frameSurface = SDL_CreateRGBSurfaceFrom(
                         (void*)opencvimg->imageData,
                         opencvimg->width, opencvimg->height,
                         opencvimg->depth*opencvimg->nChannels,
                         opencvimg->widthStep,
                         0xff0000, 0x00ff00, 0x0000ff, 0);
    
    return frameSurface;
    
    //cvReleaseImage(&opencvimg);
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
    
    sprintf(str, "TIME:%d.%06d  FPS:%02d  NET:%s  CAM:%d", sec, usec, fps, dev_eth, dev_cam);
    message = TTF_RenderText_Shaded(font, str, text_color, back_color);
    SDL_BlitSurface( message, NULL, surface, &cam_txt_local);
    
    double time_current = sec + usec*1e-6 - time_begin;
    
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    sprintf(str, "[ %0.2f s ]   ", time_current );
    message = TTF_RenderText_Shaded(font, str, text_color, back_color);
    SDL_BlitSurface( message, NULL, surface, &cam_time_local);
    
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

void add_camera()
{
    cap.read(frame);
    
    camera = MatToSurface(frame);
    SDL_BlitSurface( camera, NULL, surface, &cam_local);
    SDL_FreeSurface(camera);
    
    //SDL_UpdateWindowSurface( window );
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

void create_record()
{
    //create a record, if it has not been created 
    if( flag_record_start == 0)
    {
        // cmd handle
        struct stat st = {0};
        
        //folder_name = std::string(argv[1]);
        sprintf(folder_name, "record_%d", record_count );
    
        //test the folder if exists
        while(stat( folder_name, &st) != -1)
        {
            record_count += 10;
            sprintf(folder_name, "record_%d", record_count );
        }
        
        //create a folder to save capture
        mkdir( folder_name, 0777);
        
        sprintf(file_name, "record_%d/record_%d.cap", record_count, record_count );
    
        //open file and write 24-byte global header first
        packet_file = fopen( file_name,"ab+");
    
        pcap_file_header gheader;
          gheader.magic = 0xA1B2C3D4;
          gheader.version_major = PCAP_VERSION_MAJOR;
          gheader.version_minor = PCAP_VERSION_MINOR;
          gheader.thiszone      = 0;
          gheader.sigfigs       = 0;
          gheader.snaplen       = 65535;
          gheader.linktype      = 1;
    
        fwrite( &gheader , 24, 1, packet_file);
        
        flag_program_start = 0;
        flag_record_start = 1;
        
        usleep(100000);
        
        printf( "\n>> record_%d created \n", record_count );
    }
}

void finish_record()
{
    if( flag_record_start == 1 )
    {
         fclose(packet_file);
         flag_record_start = 0;
         record_count++;
    }
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
                    create_record();
                    add_status( "[recording]" );
                }
                else if( status == 2 )
                {
                    status = 1;
                    finish_record();
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


#define SPEC_LEN (256+5)

// pixel buffer
uint32_t   fft_pixel_buffer[128][128];
uint32_t  spec_pixel_buffer[SPEC_LEN][128];
uint32_t     spec_pixel_tmp[SPEC_LEN][128];

uint32_t  range_pixel_buffer[128][128];
uint32_t     range_pixel_tmp[128][128];

//local temp radar image
int16_t radar_adc_1[128][128];
int16_t radar_adc_2[128][128];

int chirp_count;

//complex adc data
fftw_complex radar_image_adc_1[128][128];
fftw_complex radar_image_adc_2[128][128];

//fft 1 data
fftw_complex radar_image_fft1_1[128][128];
fftw_complex radar_image_fft1_2[128][128];

//fft 1 data transpose
fftw_complex radar_image_fft1_1_t[128][128];

//fft 2 data
fftw_complex radar_image_fft2_1[128][128];
fftw_complex radar_image_fft2_2[128][128];

//color maps
uint32_t set_color(uint8_t r, uint8_t g, uint8_t b)
{
    int color = ((uint32_t)r<<16) + ((uint32_t)g<<8) + (uint32_t)b;

    return color;
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

// copy content in radar_image_fft1 to fft_pixel_buffer
void fft_to_buffer( fftw_complex array[][128] )
{
    //for each pixel in the 128x128 buffer
    //expand the window
    for(int x = 0; x < 128; x++)
    {
        for(int y = 0; y < 128; y++)
        {
            double real = array[x][y][0];
            double img  = array[x][y][1];
            
            //fft_pixel_buffer[x][y] = sqrt( real*real + img*img );
            
            double tmp = sqrt( real*real + img*img );
            tmp = 255*tmp/0x0000FFFF;
            
            //small filter
            //if( tmp < 30 ) { tmp = 0; }
            
            fft_pixel_buffer[x][y] = set_color_jet((uint8_t)tmp);
            //fft_pixel_buffer[x][y] = set_color_thermal((uint8_t)tmp);
            
        }
    }
    
    //printf("\nvalue = %d\n", fft_pixel_buffer[0][0]);
}

// apply hanning window to fft data
void fft_window( fftw_complex array[][128] )
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
    uint32_t tmp = 0;
    
    for(int x = 0; x < N; x++)
    {
        for(int y = 0; y < (N/2); y++)
        {
            x_des = (x+(N/2))%N;
            y_des = (y+(N/2))%N;
            
            tmp = fft_pixel_buffer[x_des][y_des];
            fft_pixel_buffer[x_des][y_des] = fft_pixel_buffer[x][y];
            fft_pixel_buffer[x][y] = tmp;
        }
    }
}

void delete_mirror()
{
    for(int x = 0; x < 128; x++)
    {
        for(int y = 0; y < 64; y++)
        {
            fft_pixel_buffer[x][y] = 0;
        }
    }
}

//expand small buffer into bigger sdl surface
void expand_surface_fft(SDL_Surface *surface, uint32_t buffer[][128])
{
    //for each pixel in the 128x128 buffer
    //expand the window
    for(int x = 0; x < 128; x++)
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
                    uint32_t pixel_index = (scale*x)+(FFT_WINDOW_WIDTH)*(scale*y) +h + v*FFT_WINDOW_WIDTH;
                    *((uint32_t *)surface->pixels+ pixel_index) = buffer[x][y];
                }
            }
        }
    }
}

//expand small buffer into bigger sdl surface
void expand_surface_spec(SDL_Surface *surface, uint32_t buffer[][128])
{
    //for each pixel in the 128x128 buffer
    //expand the window
    for(int x = 0; x < SPEC_LEN; x++)
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
                    uint32_t pixel_index = (scale*x)+(SPEC_WINDOW_WIDTH)*(scale*y) +h + v*SPEC_WINDOW_WIDTH;
                    *((uint32_t *)surface->pixels+ pixel_index) = buffer[x][y];
                }
            }
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

//16-byte pcap packet header
struct pcaprec_hdr_t
{
    uint32_t ts_sec;         /* timestamp seconds */
    uint32_t ts_usec;        /* timestamp microseconds */
    uint32_t incl_len;       /* number of octets of packet saved in file */
    uint32_t orig_len;       /* actual length of packet */
}__attribute__((packed));

// pcap section
void print_hex(const u_char *packet, int size)
{
    for(int i = 0; i < size; i++)
    {
        if( i%16 == 0 ) printf("\n");
        
        printf( "%02X ", *(packet+i) );
    }
    
    printf("\n");
}

//convert all int16 adc data into complex data for fft
void adc2fft()
{
    for(int x = 0; x < 128; x ++)
    {
        for(int y = 0; y < 128; y++)
        {
            radar_image_adc_1[x][y][0] = radar_adc_1[x][y]; //real data
            radar_image_adc_1[x][y][1] = 0;                 //img  data
        }
    }
}

//buffer matrix transpose operation
void fft_pixel_buffer_transpose()
{
    uint32_t tmp = 0;
    
    for(int x = 0; x < 128; x ++)
    {
        for(int y = 0; y < 128; y++)
        {
            if(x > y)
            {
                tmp = fft_pixel_buffer[x][y];
                fft_pixel_buffer[x][y] = fft_pixel_buffer[y][x];
                fft_pixel_buffer[y][x] = tmp;
            }
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
    
    //remove mirror from range-doppler plot
    delete_mirror();
    
    //get the max doppler
    for(int x = 0; x < 128; x++)
    {
        max[x] = 0;
        
        for( int y = 0; y < 128; y++)
        {
            //sum[x] = sum[x] + fft_pixel_buffer[x][y];
            if( max[x] < fft_pixel_buffer[x][y] )
            { max[x] = fft_pixel_buffer[x][y]; }
        }
        
        spec_pixel_tmp[frame_index][x] = max[x];
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
            if( max[y] < fft_pixel_buffer[x][y] )
            { max[y] = fft_pixel_buffer[x][y]; }
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

// 2d fft operation
void fft2()
{
    int N = 128;
    
    fftw_plan fft1, fft2;
    
    // copy data to fft buffer
    adc2fft();
    
    // fft window
    fft_window(radar_image_adc_1);
    
    fft1 = fftw_plan_dft_2d(N, N, &radar_image_adc_1[0][0], &radar_image_fft1_1[0][0], FFTW_FORWARD, FFTW_ESTIMATE);
    
    fftw_execute(fft1);
    fftw_destroy_plan(fft1);
    
    //copy from fft buffer to display buffer
    fft_to_buffer(radar_image_fft1_1);
    
    //do fftshift on display buffer
    fftshift();
    
    //delete_mirror();
    
    //ptinf("\n>> fft done\n");
}

    
//packet callback function
void packet_callback(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    //pause the data stream, ignore the data
    if( status == 3 )
    {
        check_key_input();
        return;
    }
    
    // payload starts at packet[42], packet[44] is the chirp number
    // chirp number is from udp packet
    // chirp count is local chirp count
    // frame_count is local measurement frame count
    
    int chirp = *(packet+44);
    
    //if chirp number is 0, take the picture
    if(chirp == 0)
    {
        //sync each frame, start from chirp 0
        chirp_count = 0;
        
        cap.read(frame);
        
        //save pictures first
        if( status == 2 )
        {
            //make sure the saved image has the same name width
            sprintf( image_name, "%s/%ld.%06ld.jpg", folder_name, header->ts.tv_sec, header->ts.tv_usec);
            imwrite( image_name, frame);
        }
        
        if( flag_program_start == 0 )
        {
            time_begin = header->ts.tv_sec + header->ts.tv_usec*1e-6;
            flag_program_start = 1;
        }
    }
    
    //for each packet/chirp, extract adc data
    for(int i = 0; i < 256; i++)
    {
        //if( i%16 == 0 ) printf("\n");
        
        int16_t high = *(packet+44+i*2+1);
        int16_t low  = *(packet+44+i*2+2);
        
        int16_t combined = (high << 8) + low;
        
        if(i <128) {  radar_adc_1[chirp][i] = combined; }
        else       {  radar_adc_2[chirp][i-128] = combined; }
    }
    
    //printf("\nchirp = %d, chirp_count = %d", chirp, chirp_count);
    
    // whenever 128 chirps received (0 - 127)
    if(chirp_count == 127)
    {
        // do 2d fft
        fft2();
        
        // draw fft display
        SDL_LockSurface( rvwindow );
        expand_surface_fft( rvwindow, fft_pixel_buffer );
        SDL_UnlockSurface( rvwindow );
        SDL_BlitSurface( rvwindow, NULL, surface, &fft_local);
        
        //calculate range-t plot
        calc_range_plot();
        
        // draw range-t view
        SDL_LockSurface( rtwindow );
        expand_surface_fft( rtwindow, range_pixel_buffer );
        SDL_UnlockSurface( rtwindow );
        SDL_BlitSurface( rtwindow, NULL, surface, &range_local);
        
        //calculate the spectrogram
        calc_spectrogram();
        
        //draw spectrogram
        SDL_LockSurface( specwindow );
        expand_surface_spec( specwindow, spec_pixel_buffer );
        SDL_UnlockSurface( specwindow );
        SDL_BlitSurface( specwindow, NULL, surface, &spec_local);
        
        //add some txt
        add_cam_txt(header->ts.tv_sec, header->ts.tv_usec);
        add_txt( "Range-Doppler plot", &fft_txt_local);
        add_txt( "Range-T plot", &range_txt_local);
        add_txt( "Doppler Spectrogram", &spec_txt_local);
        
        //draw camera frame
        //cap.read(frame);
        
        if( !cap.isOpened() )
        {
            add_txt( "NO IMAGE", &cam_error_local);
        }
        else
        {
            camera = MatToSurface(frame);
            SDL_BlitSurface( camera, NULL, surface, &cam_local);
            SDL_FreeSurface(camera);
        }
        
        //camera = MatToSurface(frame);
        //SDL_BlitSurface( camera, NULL, surface, &cam_local);
        //SDL_FreeSurface(camera);
        
        SDL_UpdateWindowSurface( window );
        
        frame_count++;
        
    }
    
    chirp_count++;
    
    //save packets after the chirp number check
    if( status == 2 )
    {
        //save each packet
        pcaprec_hdr_t packet_header;
          packet_header.ts_sec   = header->ts.tv_sec;
          packet_header.ts_usec  = header->ts.tv_usec;
          packet_header.incl_len = header->len;
          packet_header.orig_len = header->caplen;
    
        //write 16-byte packet header
        fwrite( &packet_header, 16, 1, packet_file);
    
        //write actual packet with 515-byte payload
        fwrite( packet, header->len, 1, packet_file);
    }
    
    // check keys
    check_key_input();
}

int main(int argc, char **argv)
{   
    // calculate hanning window
    for (int i=0; i<WINDOW_SIZE; i++)
    {
        hanning[i] = (1.0 - cos(2.0 * M_PI * i/(WINDOW_SIZE-1))) * 0.5;
    }
    
    //open video device
    cap.open(dev_cam);
    
    //set framerate to 60fps
    cap.set(CV_CAP_PROP_FPS, 60);
    
    //set camera resolution to 640x480
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
    cap.read(frame); //dummy read
    
    //sdl section
    sdl_init();
    add_status( "[viewing]  " );
    
    // pcap section
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    
    //dev = pcap_lookupdev(errbuf);
    
    struct bpf_program fp;
    
    struct pcap_pkthdr header;
    const u_char *packet;	
    
    bpf_u_int32 mask;
    bpf_u_int32 net;

    int packet_num = 128*16*60;
    
    printf(">> listen on eth: %s, cam: %d\n", dev_eth, dev_cam);
    
    // setup
    pcap_lookupnet(dev_eth, &net, &mask, errbuf);
    
    handle = pcap_open_live(dev_eth, BUFSIZ, 1, 1000, errbuf);
    
    if(handle == NULL)
    {
        printf(">> fail to open device %s, check setup/permission\n", dev_eth);
        return 1;
    }
    
    pcap_compile(handle, &fp, filter_exp, 0, net);
    pcap_setfilter(handle, &fp);
    
    chirp_count = 0;
    
    // packet callback
    pcap_loop(handle, 0, packet_callback, NULL);

    pcap_freecode(&fp);
    pcap_close(handle);
    
    fclose(packet_file);
    
    //SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
    
    printf("\n>> done\n");
    
    return 0;
}
