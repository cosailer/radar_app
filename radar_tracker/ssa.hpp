#ifndef SSA_HPP_
#define SSA_HPP_

//core class
class core_t
{
  public:
    
    std::vector<int> buffer;
    
    int c_max;
    int c_min;
    int cycle;
    int ramp_dir;
    double walk_mean_v;
    
    core_t(int cmax, int cmin)
    {
        //buffer.resize(cmax,0);
        c_max = cmax;
        c_min = cmin;
        cycle = 0;
        ramp_dir = 0;
        walk_mean_v = 0;
    }
    
    void update_buffer(int value)
    {
        buffer.push_back(value);
        
        //check if the buffer is full
        //then get peak/trough according to object type
        //if buffer is full, find peak
        if( buffer.size() == c_max )
        {
            //type 0: core, find peak/trough
            get_walk_mean();  // calculate walk_mean_v
            cycle = get_extreme_one(); //update cycle
            
            //no peak found
            if(cycle==-1)
            {
                cycle = C_Max-2*C_Min-1;
            }
            
            //remove all buffer before the peak/trough
            buffer.erase(buffer.begin(),buffer.begin()+cycle+C_Min);
            cycle = cycle + C_Min; //calculate actual cycle length
        }
    }
    
    void get_walk_mean()
    {
        walk_mean_v = 0;
        
        for( int i=0; i < buffer.size(); i++)
        {
            walk_mean_v = walk_mean_v + buffer[i];
        }
        
        walk_mean_v = 63 - walk_mean_v/buffer.size();
    }
    
    //get the first peak for below 64/first trough for above 64
    int get_extreme_one()
    {
        //for buffer[0] to buffer[c_max-c_min-1]
        for(int i = 0; i < c_max-c_min ; i++)
        {
            //evaluate each valid point buffer[i]
            if(buffer[i] > 1)
            {
                int mark = 1;
                
                //for positive speed
                if(buffer[i] > 63)
                {
                    //find trough
                    for(int j = 1; j < c_min ; j++)
                    {
                        //if a larger value found, clear buffer[i] and continue to buffer[i+1]
                        if( buffer[i+j] < buffer[i] )
                        {
                            mark = 0;
                            break;
                        }
                    }
                }
                
                //for negative speed
                if(buffer[i] <= 63)
                {
                    //find peak
                    for(int j = 1; j < c_min ; j++)
                    {
                        //if a larger value found, clear buffer[i] and continue to buffer[i+1]
                        if( buffer[i+j] > buffer[i] )
                        {
                            mark = 0;
                            break;
                        }
                    }
                }
                
                //if buffer[i] is a peak, return
                if(mark == 1)
                {
                    return i;
                }
            }
        
        }
        
        //no peak found, return -1
        return -1;
    }
    
};

//max doppler class
class max_doppler_t
{
  public:
    
    std::vector<int> buffer;
    std::vector<int> buffer_core;
    
    int c_max;
    int c_min;
    int cycle;
    int ramp_dir;
    int max_doppler_pds;
    int min_doppler_nds;
    
    max_doppler_t(int cmax, int cmin)
    {
        //buffer.resize(cmax,0);
        c_max = cmax;
        c_min = cmin;
        cycle = 0;
        ramp_dir = 0;
        max_doppler_pds = 0;
    }
    
    void update_buffer(int value, int core_value)
    {
        //std::cout << value << std::endl;
        
        buffer.push_back(value);
        buffer_core.push_back(core_value);
        
        //check if the buffer is full
        //then get peak/trough according to object type
        //if buffer is full, find peak
        if( buffer.size() == c_max )
        {
            cycle = get_peak_one(ramp_dir); //update cycle
            
            //no peak found
            if(cycle==-1)
            {
                cycle = C_Max-2*C_Min-1;
            }
            else
            {
                max_doppler_pds = buffer[cycle] - buffer_core[cycle];
            }
            
            //update ramp direction
            if(buffer[cycle+C_Min] <= buffer[cycle+C_Min+1])
            { ramp_dir = 1; }
            else
            { ramp_dir = 0; }
            
            //remove all buffer before the peak/trough
            buffer.erase(buffer.begin(), buffer.begin()+cycle+C_Min);
            buffer_core.erase(buffer_core.begin(), buffer_core.begin()+cycle+C_Min);
            
            cycle = cycle + C_Min; //calculate actual cycle length
        }
    }
    
    //get the first peak from buffer
    int get_peak_one(int direction)
    {
        //for buffer[0] to buffer[c_max-c_min-1]
        for(int i = 0; i < c_max-c_min-1 ; i++)
        {
            //check ramp direction
            if(direction==0)
            {
                //update ramp direction
                if(buffer[i] < buffer[i+1])
                { direction = 1; }
                else
                { direction = 0; }
                
                continue;
            }
            
            //evaluate each valid point buffer[i]
            if(buffer[i] > 1)
            {
                int peak_mark = 1;
                
                //check c_min ahead
                for(int j = 1; j < c_min ; j++)
                {
                    //if a larger value found
                    if( buffer[i+j] > buffer[i] )
                    {
                        peak_mark = 0;
                        break;
                    }
                }
                
                //if buffer[i] is a peak, return
                if(peak_mark == 1)
                {
                    return i;
                }
            }
        }
        
        //no peak found, return -1
        return -1;
    }
    
    
};

//doppler class
class min_doppler_t
{
  public:
    
    std::vector<int> buffer;
    std::vector<int> buffer_core;
    
    int c_max;
    int c_min;
    int cycle;
    int ramp_dir;
    int max_doppler_pds;
    int min_doppler_nds;
    
    min_doppler_t(int cmax, int cmin)
    {
        //buffer.resize(cmax,0);
        c_max = cmax;
        c_min = cmin;
        cycle = 0;
        ramp_dir = 0;
        min_doppler_nds = 0;
    }
    
    void update_buffer(int value, int core_value)
    {
        buffer.push_back(value);
        buffer_core.push_back(core_value);
        
        //check if the buffer is full
        //then get peak/trough according to object type
        //if buffer is full, find peak
        if( buffer.size() == c_max )
        {
            cycle = get_trough_one(ramp_dir); //update cycle
            
            //no peak found
            if(cycle==-1)
            {
                cycle = C_Max-2*C_Min-1;
            }
            else
            {
                min_doppler_nds = buffer_core[cycle] - buffer[cycle];
            }
            
            //update ramp direction
            if(buffer[cycle+C_Min] < buffer[cycle+C_Min+1])
            { ramp_dir = 1; }
            else
            { ramp_dir = 0; }
            
            //remove all buffer before the peak/trough
            buffer.erase(buffer.begin(), buffer.begin()+cycle+C_Min);
            buffer_core.erase(buffer_core.begin(), buffer_core.begin()+cycle+C_Min);
            
            cycle = cycle + C_Min; //calculate actual cycle length
        }
    }
    
    //get the first trough from buffer
    int get_trough_one(int direction)
    {
        //for buffer[0] to buffer[c_max-c_min-1]
        for(int i = 0; i < c_max-c_min ; i++)
        {
            
            //check ramp direction
            if(direction==1)
            {
                //update ramp direction
                if(buffer[i] < buffer[i+1])
                { direction = 1; }
                else
                { direction = 0; }
                
                continue;
            }
            
            //evaluate each valid point buffer[i]
            if(buffer[i] > 1)
            {
                int trough_mark = 1;
                
                //check c_min ahead
                for(int j = 1; j < c_min ; j++)
                {
                    //if a smaller value found, clear buffer[i] and continue to buffer[i+1]
                    if( buffer[i+j] < buffer[i] )
                    {
                        trough_mark = 0;
                        break;
                    }
                }
                
                //if buffer[i] is a peak, return
                if(trough_mark == 1)
                {
                    return i;
                }
            }
        }
        
        //no trough found, return -1
        return -1;
    }
    
    
};

//objects for single target tracking
core_t core(C_Max,C_Min);  //core object
max_doppler_t max_doppler(C_Max,C_Min);  //max doppler object
min_doppler_t min_doppler(C_Max,C_Min);  //min doppler object

#endif
