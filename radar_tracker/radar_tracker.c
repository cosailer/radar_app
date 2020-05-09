// this program takes udp packets from infineon radar kit
// and perform 2d fft with fftw3
// then display the range-doppler plot using sdl2
// when passing a folder name to the program, it will record
// all packets to the folder as well as the pictures
// that are synced with them

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

//pcap
#include <pcap.h>

#include "kalman.h"
#include "myfft.h"
#include "mysdl.h"
#include "myvar.h"
#include "mydsp.h"

int chirp_count;

//file variable
FILE *packet_file = NULL;
char *file = NULL;

static unsigned long packet_num = 0;

//operation mode: 0: use live stream from radar module, 1: use recordings
static int op_mode = 0;

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

// 2d array dynamic allocation
// packets[packet_num][record_size]
uint8_t **packets = NULL;

void packets_alloc( int num, int size )
{
    // allocate mem for array pointer
    packets = (uint8_t**)malloc( num * sizeof(uint8_t *) );
    
    for(int n = 0; n < num; n++)
    {
        // allocate mem for rows pointer
        packets[n] = (uint8_t*)malloc( size * sizeof(uint8_t) );
    }
}

void file_init(char *filename)
{
    //file operation
    struct stat info;
    //filename = argv[1];

    stat(filename, &info); 
    unsigned long file_size = info.st_size;
    
    if( file_size <= 0 )
    {
        printf("\nplease check input file !\n");
        exit(1);
    }
    
    if( filename == NULL )
    {
        printf("\nplease check input file !\n");
        exit(1);
    }
    
    printf("file name: %s\n", filename);
    
    printf("file size: %lu\n", file_size);
    
    char *packets_raw = (char *)malloc(file_size);
    
    FILE *packet_file = fopen(filename, "rb");
    
    size_t blocks_read = fread(packets_raw, info.st_size, 1, packet_file);
    
    // first 24 bytes is global header
    int gheader_size = 24;
    
    //other header size
    int header_size = 16+42;
    int payload_size = 515;
    
    // then 58 bytes UDP header + 515 bytes payload
    int record_size = header_size + payload_size;
    
    // get the total packet number
    packet_num = ( file_size-gheader_size )/record_size;
    
    //the last packets at the end of the file maybe incomplete
    packet_num = packet_num - 2;
    
    // parse the packets
    packets_alloc( packet_num, record_size );
    
    // initialize the 2d array with raw packets
    for(int i = 0; i < packet_num; i++)
    {
        for(int k = 0; k < record_size; k++)
        {
            packets[i][k] = packets_raw[ gheader_size + record_size*i + k ];
        }
    }
    
    // check packets
    int index = 0;
    int loss_occ = 0;
    int frame_num = 0;
    
    // forward loop
    for(int i = 0; i < packet_num; i++)
    {
        if( packets[i][3+57] != index)   // check current packet index
        {
            printf("   packets loss at %d\n", i );
            loss_occ = loss_occ + 1;
        
            // avoid skipping frame count later
            if( packets[i][3+57] < index)  {  frame_num = frame_num + 1; }
        
            index = packets[i][3+57];
        }
    
        if(index == 127)
        {
            index = 0;
            frame_num = frame_num + 1;
        }
        else
        {  index = index + 1;  }
    }

    // total frame count must - 1, since last loop add 1
    frame_num = frame_num - 2;

    printf( "> packet processing...\n" );
    printf( "  input file : %s\n", filename );
    printf( "  packet loss occurrence : %d\n", loss_occ );
    printf( "  total frame number : %d\n", frame_num );
    
    
    // remove the extra unwanted packets at the beginning and at the end,  if there is any
    // find the start and the end
    int index_start = 0;
    int index_end   = 0;

    for(long i = 0; i < packet_num; i++)
    {
        if( packets[i][3+57] == 0 )
        {
            index_start = i;
            break;
        }
    }
    
    for(long i = packet_num-1; i >= 0; i--)
    {
        if( packets[i][3+57] == 127 )
        {
            index_end = i;
            break;
        }
    }
    
    // remove packets
    printf( "> total packets before removal : %ld\n", packet_num );
    printf( "  record size before removal : %d\n", record_size );
    printf( "  packet start : %d\n", index_start );
    printf( "  packet end   : %d\n", index_end );

    packets = &packets[index_start];
    
    packet_num = index_end - index_start + 1;
    
    printf( "> total packets after removal : %ld\n", packet_num );
    
    uint32_t begin_sec  = *(uint32_t *)(&packets[0][0]);
    uint32_t begin_usec = *(uint32_t *)(&packets[0][4]);
    
    uint32_t end_sec  = *(uint32_t *)(&packets[packet_num-1][0]);
    uint32_t end_usec = *(uint32_t *)(&packets[packet_num-1][4]);
        
    double time_begin = begin_sec + begin_usec*1e-6;
    double time_end = end_sec + end_usec*1e-6;
    
    double time_elapse = time_end - time_begin;
    
    printf( "  total frame time elapsed : %0.2f s\n", time_elapse );

    double fm = frame_num / time_elapse;
    printf( "  measurement frequency : %0.2f fps\n", fm );

    printf("> packet extraction complete, start replay\n");
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
        set_map( 0x5F, 0x5F, 0x5F );
        
        //do 2d fft and calculate angle
        fft2_angle();
        
        //remove main clutter
        myRemClutter();
        
        calibration();
        
        if(calibration_finished == 1)
        {
            //raw sample mode
            if(track_mode==0)
            {
                image_process_raw();
            }
            //single target
            if(track_mode==1)
            {
                image_process_single();
            }
            //multi-target
            else if(track_mode==2)
            {
                image_process_multi();
            }
        }
        
        // draw fft display
        SDL_LockSurface( rvwindow );
        expand_surface( rvwindow, display_pixel_buffer, 128 );
        SDL_UnlockSurface( rvwindow );
        SDL_BlitSurface( rvwindow, NULL, surface, &fft_local);
        
        //calculate range-t plot
        calc_range_plot();
        
        //draw range-t view
        SDL_LockSurface( rtwindow );
        expand_surface( rtwindow, range_pixel_buffer, 128 );
        SDL_UnlockSurface( rtwindow );
        SDL_BlitSurface( rtwindow, NULL, surface, &range_local);
        
        //calculate the spectrogram
        calc_spectrogram();
        
        //draw spectrogram
        SDL_LockSurface( specwindow );
        expand_surface( specwindow, spec_pixel_buffer, 256+5);
        SDL_UnlockSurface( specwindow );
        SDL_BlitSurface( specwindow, NULL, surface, &spec_local);
        
        //add some txt
        add_cam_txt(header->ts.tv_sec, header->ts.tv_usec);
        add_txt( "Range-Doppler plot", &fft_txt_local);
        add_txt( "Range-T plot", &range_txt_local);
        add_txt( "Doppler spectrogram", &spec_txt_local);
        add_txt( "Map view", &mapview_local);
        
        //draw camera frame
        //add_txt( "NO IMAGE", &map_error_local);

        //draw map
        SDL_LockSurface( mapwindow );
        expand_map( mapwindow );
        SDL_UnlockSurface( mapwindow );
        SDL_BlitSurface( mapwindow, NULL, surface, &map_local);
        
        draw_map_scale();
        
        SDL_UpdateWindowSurface( window );
        
        frame_count++;
    }
    
    chirp_count++;
    
    // check keys
    check_key_input();
}

int main(int argc, char **argv)
{
    char *option = NULL;
    
    //file name provided, use recording
    if(argc == 3)
    {
        //take the second argument as tracking mode
        option = argv[2];
        track_mode = atoi(argv[2]);
        
        //take the first argument as file name
        file = argv[1];
        file_init(file);
        op_mode = 1;
    }
    else if(argc == 2)
    {
        //take the first argument as tracking mode
        option = argv[1];
        track_mode = atoi(argv[1]);
        op_mode = 0;
    }
    else
    {
        printf("\n>> please check your input\n");
        printf(  "recording: radar_tracker  filename  tracking_mode\n");
        printf(  "live data: radar_tracker  tracking_mode\n\n");
        exit(1);
    }
    
    //fft init
    //array_2d_clear(fft_pixel_buffer_env);
    kalman_init();
    
    
    // calculate hanning window
    for (int i=0; i<WINDOW_SIZE; i++)
    {
        hanning[i] = (1.0 - cos(2.0 * M_PI * i/(WINDOW_SIZE-1))) * 0.5;
    }
    
    //sdl section
    sdl_init();
    add_status( "[viewing]  " );
    
    // pcap section
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    
    //dev = pcap_lookupdev(errbuf);
    
    struct bpf_program fp;
    
    struct pcap_pkthdr header;
    
    bpf_u_int32 mask;
    bpf_u_int32 net;

    //int packet_num = 128*16*60; //set packet number limit
    
    if(op_mode == 0)
    {
        printf(">> listen on eth: %s\n", dev_eth);
    
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
    }
    
    else if (op_mode == 1)
    {
        for( int i = 0; i < packet_num; i++)
        {
            //add time stamp
            header.ts.tv_sec = *(uint32_t *)(&packets[i][0]);
            header.ts.tv_usec = *(uint32_t *)(&packets[i][4]);;
            
            //process packets
            packet_callback( 0, &header, packets[i]+16 );
            usleep(100);
            
            //printf("i = %d\n", i);
        }
    }
    
    add_status( "[finished]    " );
    
    //do not close window when finished
    while(true)
    {
        //check_key_input();
        press_q_quit();
        usleep(100);
    }
    
    //SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
    
    printf("\n>> done\n");
    
    return 0;
}
