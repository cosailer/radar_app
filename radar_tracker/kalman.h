#ifndef KALMAN_H_
#define KALMAN_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <algorithm> 

#include "myvar.h"
#include "myfft.h"
#include "myHungarian.h"

using namespace boost::numeric::ublas;

#define e 1e-2
#define T 59.3e-3
#define MAX_VAL 4096.00

//kalman single-targets
//last max rcs point
//~ static int last_max_d = 64;
//~ static int last_max_r = 64;
//~ static int max_x = 64;
//~ static int max_y = 64;

//kalman multi-targets

//----------state transition matrix
static double  A0[4][4] = { {1, 0, T, 0},
                            {0, 1, 0, T},
                            {0, 0, 1, 0},
                            {0, 0, 0, 1} };
                            
//----------covariance matrix transition matrix
static double Ex0[4][4] = { { pow(T,4)/4,     0,      pow(T,3)/2,     0     },
                            {     0,      pow(T,4)/4,     0,      pow(T,3)/2},
                            { pow(T,3)/2,     0,      pow(T,2),       0     },
                            {     0,      pow(T,3)/2,     0,      pow(T,2)  } };

//----------observation noise 
static double Ez0[2][2] = { {e, 0},
                            {0, e} };

//----------observation matrix
static double  H0[2][4] = { {1, 0, 0, 0},
                            {0, 1, 0, 0} };

//----------initial states
zero_matrix<double> X0(4, 1);

//----------initial covariance matrix
zero_matrix<double> P0(4, 4);

static matrix<double>  A(4, 4);
static matrix<double> Ex(4, 4);
static matrix<double> Ez(2, 2);
static matrix<double>  H(2, 4);
static identity_matrix<double> I(4);

static matrix<double>  input_target(4, 1);

//find the first occurrence in a vector
int find_first_index( std::vector<int> input, int value)
{
    for(int i = 0; i< input.size(); i++)
    {
        if( input[i] == value )
        {  return i;  }
    }
    
    return -1;
}


//set all original value in matrix to target
void matrix_clear( matrix<double> &input )
{
    for(int r = 0; r < input.size1(); r++)
    {
        for(int c = 0; c < input.size2(); c++)
        {
            input(r,c) = 0;
        }
    }
}

//copy from 2d array(size 4) to matrix
void init_matrix_4( matrix<double> &input_matrix, double input_array[][4])
{
    for (unsigned r = 0; r < input_matrix.size1(); ++r)
    {    for (unsigned c = 0; c < input_matrix.size2(); ++c)
         {
             input_matrix(r, c) = input_array[r][c];
         }
    }
}

//copy from 2d array(size 2) to matrix
void init_matrix_2( matrix<double> &input_matrix, double input_array[][2])
{
    for (unsigned r = 0; r < input_matrix.size1(); ++r)
    {    for (unsigned c = 0; c < input_matrix.size2(); ++c)
         {
             input_matrix(r, c) = input_array[r][c];
         }
    }
}

//calculate the inverse of the matrix, with size 2
matrix<double> inverse_matrix_2( matrix<double> &input)
{
    //~ |a b|                  | d -b|
    //~ |c d|   ->   1/(ad-bc) |-c  a|
    
    matrix<double> output(2,2);
    
    double det = input(0,0)*input(1,1) - input(0,1)*input(1,0);
    
    if(det == 0)
    {
		output(0,0) = 0;
		output(0,1) = 0;
		output(1,0) = 0;
		output(1,1) = 0;
		return output;
	}
	else
	{
		output(0,0) =  input(1,1)/det;
		output(0,1) = -input(0,1)/det;
		output(1,0) = -input(1,0)/det;
		output(1,1) =  input(0,0)/det;
	}
	
	return output;
}


//track class for single track
class track_t
{
  public:
    matrix<double> X_p;
    matrix<double> X;
    matrix<double> P_p;
    matrix<double> P;
    matrix<double> K;
    
    std::string name;
    int track_score_std;
    int track_penalty;
    int track_alive;
    int track_state;
    
    int current_x;
    int current_y;
    int current_r;
    int current_d;
    double current_phi;
    
    track_t()
    {
          X = matrix<double>(4,1);
        X_p = matrix<double>(4,1);
          P = matrix<double>(4,4);
        P_p = matrix<double>(4,4);
          K = matrix<double>(4,2);
          
        name = "unknown";
        track_score_std = 0;
        track_penalty = 0;
        track_alive = 0;
        track_state = 0;
        
        current_x = 0;
        current_y = 0;
        current_r = 0;
        current_d = 0;
        current_phi = 0;
    }
    
    void track_penalty_increase()
    {
        track_penalty++;
        track_state_update();
    }
    
    void track_penalty_reset()
    {
        track_penalty = 0;
    }
    
    void track_alive_increase()
    {
        track_alive++;
        track_state_update();
    }
    
    // track management
    // state  0: untracked
    // state  1: tracked
    // state -1: lost
    
    // in state  0: track_alive  >= 20,   state 0 -> state  1
    // in state  0: track_penalty >= 10,  state 0 -> state -1
    // in state  1: track_penalty >= 50,  state 1 -> state  0
    // in state  1: track_score_std > 15, state 1 -> state  0
    // in state -1: track will be deleted
    
    void track_state_update()
    {
        if( track_state == 0 )
        {
            if( track_alive >= A0LEN )
            {
                track_state = 1;
                track_penalty = 0;
            }
            
            if( track_penalty >= P0LEN )
            {
                track_state = -1;
            }
        }
        else if( track_state == 1 )
        {
            if( track_penalty >= P1LEN )
            {
                track_state = 0;
                track_penalty = 0;
                track_alive = 0;
            }
            
            if( track_score_std >= E1LEN )
            {
                track_state = 0;
                track_penalty = 0;
                track_alive = 0;
            }
        }
    }
    
    //show target on map
    void track_map_update(int color)
    {
        int x = current_x + 64;
        int y = 64 - current_y;
        
        map_target_buffer[x][y] = set_color( 0xF*color, 0, 0);
    }
    
    //initilize with X0
    void track_init( std::vector <double> &input_vector )
    {
        X(0,0) = input_vector[0];
        X(1,0) = input_vector[1];
        current_x = input_vector[0];
        current_y = input_vector[1];
        current_r = input_vector[2];
        current_d = input_vector[3];
        current_phi = input_vector[4];
        
        matrix<double> tmp1, tmp2, tmp3, tmp4;
        
        //X_p  = A*X;
        X_p = prod(A, X);
    
        //P_p = A*P*A'+Ex;
        tmp1 = prod(A, P);
        P_p = prod(tmp1, trans(A)) + Ex;
            
        //K2 = P_p*H'* inv(H*P_p*H'+Ez);
        tmp1 = prod(P_p, trans(H));
        
        tmp2 = prod(H, P_p);
        tmp3 = prod(tmp2, trans(H)) + Ez;
        tmp4 = inverse_matrix_2(tmp3);
        
        K = prod(tmp1, tmp4);
    }
    
    //kalman predict
    void track_kalman_predict()
    {
        matrix<double> tmp1, tmp2, tmp3, tmp4;
        
        //X_p  = A*X;
        X_p = prod(A, X);
    
        //P_p = A*P*A'+Ex;
        tmp1 = prod(A, P);
        P_p = prod(tmp1, trans(A)) + Ex;
            
        //K2 = P_p*H'* inv(H*P_p*H'+Ez);
        tmp1 = prod(P_p, trans(H));
        
        tmp2 = prod(H, P_p);
        tmp3 = prod(tmp2, trans(H)) + Ez;
        tmp4 = inverse_matrix_2(tmp3);
        
        K = prod(tmp1, tmp4);
    }
    
    //kalman update
    void track_kalman_update( std::vector <double> &input_vector )
    {
        matrix<double> tmp1, tmp2;
        matrix<double> input_matrix(4, 1);
        
        input_matrix(0,0) = input_vector[0];
        input_matrix(1,0) = input_vector[1];
        input_matrix(2,0) = 0;
        input_matrix(3,0) = 0;
        
        //update
        //X = X_p+K*( H*input' - H*X_p ); 
        tmp1 = prod( H, input_matrix );
        tmp2 = prod( H, X_p );
        X = X_p + prod( K, tmp1-tmp2 ); 
        
        //P = (I-K*H) * P_p;
        P = prod( I-prod(K, H), P_p );
        
        current_x = X(0,0);
        current_y = X(1,0);
        current_r = input_vector[2];
        current_d = input_vector[3];
        current_phi = input_vector[4];
    }
    
    void extract_track()
    {
        //for each point in the radar_image_th
        for(int d = 0; d < 128; d++)
        {
            for(int r = 0; r < 64; r++)
            {
                if( (d>=current_d-pattern_w)&&(d<=current_d+pattern_w)&&(r>=current_r-pattern_h)&&(r<=current_r+pattern_h) )
                {
                    double angle_tmp = abs(radar_angle[d][r]-current_phi);
                    
                    if( angle_tmp > pattern_a ) { continue; }
                    
                    if( radar_image_extract[d][r] < 1)
                    {
                        radar_image_extract[d][r] = radar_image_th[d][r];
                    }
                }
            }
        }
    }
    
    void print_track()
    {
        std::cout << current_x << ", ";
        std::cout << current_y << ", ";
        std::cout << current_r << ", ";
        std::cout << current_d << ", ";
        std::cout << current_phi << std::endl;
    }
    
};

static track_t track_one;

//~ std::vector< double > in = { 64, 64, 0, 0, 0 };
//~ track_one.track_init(in);

class track_multi
{
  public:
    std::vector<track_t> tracks;
    int tracked_num;
    
    //initialize a new track
    void tracks_add( std::vector <double> &input_vector )
    {
        track_t new_track;
        
        new_track.track_init(input_vector);
        
        //add new track
        tracks.push_back(new_track);
    }
    
    //kalman predict for all tracks
    void tracks_predict_all()
    {
        for(int f = 0; f < tracks.size(); f++)
        {
            tracks[f].track_kalman_predict();
        }
    }
    
    //kalman update for specific track
    void tracks_update( int track_num, std::vector <double> &input_vector )
    {
        tracks[track_num].track_kalman_update(input_vector);
    }
    
    //map update for all tracks
    void tracks_map_update()
    {
        tracked_num = 0;
        
        //draw all targets on map
        for(int f = 0; f < tracks.size(); f++)
        {
            if(tracks[f].track_state == 0)
            {
                tracks[f].track_map_update(0);
            }
            if(tracks[f].track_state == 1)
            {
                tracks[f].track_map_update(f*16);
                tracked_num++;
            }
        }
    }
    
    //remove tracks
    void tracks_remove()
    {
        if( tracks.size() == 0)
        {  return;  }
        
        //remove all tracks that has state = -1
        for(auto it = tracks.end()-1; it != tracks.begin(); it--)
        {
            if( (*it).track_state == -1)
            {
                *it = tracks.back();
                tracks.pop_back();
                
                //~ tracks.erase(it);
            }
        }
    }
    
};

static track_multi tracks_all;

void kalman_init()
{
    init_matrix_4( A, A0 );
    init_matrix_4( Ex, Ex0 );
    init_matrix_2( Ez, Ez0 );
    init_matrix_4( H, H0 );
    
    //~ add_track(X0);
    //~ track_one.init();
}

//single target kalman tracker implemetation
void myKalmanTracker_single()
{
    //track_one.print_track();
    
    std::vector< std::vector <double> > track_input;
    std::vector <double>  cost;
    
    double gate = 25.0;
    
    double input_angle = 0;
    double input_x = 0;
    double input_y = 0;
    int    solution = 0;
    
    matrix<double> tmp1, tmp2, tmp3, tmp4;
    
    //kalman predict
    track_one.track_kalman_predict();
    
    //extract the valid target points from the current image
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            if(radar_image_peak[d][r] < 1)
            {  continue;  }
            
            //init a input array
            std::vector< double > input = { 0, 0, 0, 0, 0 };
            double  distance = 0;
            
            //polar to cartesian
            input_angle = myGetAngle( r, d, 1);
            
            input_x = (64-r)*sin( input_angle );
            input_y = (64-r)*cos( input_angle );
            
            input[0] = input_x;
            input[1] = input_y;
            input[2] = r;
            input[3] = d;
            input[4] = input_angle;
            
            distance = pow(input[0]-track_one.X_p(0,0), 2) + pow(input[1]-track_one.X_p(1,0), 2);
            
            if(std::isnan(distance))
            {
                //~ //distance = MAX_VAL;
                continue;
            }
            
            track_input.push_back(input);
            cost.push_back(distance);
        }
    }
    
    //if no track_input empty, continue
    if( track_input.size() == 0 )
    {
        track_one.track_map_update(32);
        return;
    }
    
    //find the smallest value in cost
    auto cost_min = *std::min_element(cost.begin(), cost.end());
    
    for(int i = 0; i < cost.size(); i++)
    {
        if(cost[i] == cost_min)
        {
            solution = i;
            break;
        }
    }
    
    //kalman update
    track_one.track_kalman_update(track_input[solution]);
    
    track_one.track_map_update(32);
}

void myKalmanTracker_multi()
{
    
    if(calibration_finished == 0)
    {
        //~ std::cout << calibration_finished << std::endl;
        return;
    }
    
    std::vector< std::vector <double> > track_input;
    
    double input_angle = 0;
    double input_x = 0;
    double input_y = 0;
    
    //do kalman predict for all tracks
    tracks_all.tracks_predict_all();
    
    //extract the valid target points from the current image
    for(int d = 0; d < 128; d++)
    {
        for(int r = 0; r < 64; r++)
        {
            if(radar_image_peak[d][r] <= 0)
            {  continue;  }
            
            //init a input array
            std::vector< double > input = { 0, 0, 0, 0, 0 };
            
            //polar to cartesian
            input_angle = myGetAngle( r, d, 1);
            
            input_x = (64-r)*sin( input_angle );
            input_y = (64-r)*cos( input_angle );
            //~ input_x = input_x + 64;
            //~ input_y = 64 - input_y;
            
            input[0] = input_x;
            input[1] = input_y;
            input[2] = r;
            input[3] = d;
            input[4] = input_angle;
            
            if(std::isnan(input_x))
            {
                continue;
            }
            
            track_input.push_back(input);
        }
    }
    
    //~ std::cout << track_input.size() << std::endl;
    
    //if no track_input empty, continue
    if( track_input.size() == 0 )
    {
        tracks_all.tracks_map_update();
        return;
    }
    
    int target_count = track_input.size();
    int track_count  = tracks_all.tracks.size();
    
    //calculate distance matrix
    matrix<double>  distance(target_count, track_count);
    
    for( int i = 0; i < target_count; i++)
    {
        for( int f = 0; f < track_count; f++)
        {
            //calculate euclidean distance squared
            distance(i,f) = pow(track_input[i][0]-tracks_all.tracks[f].X_p(0,0), 2) + pow(track_input[i][1]-tracks_all.tracks[f].X_p(1,0), 2);
            
            //apply gate limit
            if( distance(i,f) > gate )
            {
                distance(i,f) = MAX_VAL;
            }
        }
    }
    
    //minimize distance matrix to get cost matrix
    //   TO DO: find a more compact way to minimize distance matrix
    
    //   MAX  MAX  3    MAX
    //   MAX  MAX  MAX  MAX      MAX  3
    //   5    MAX  MAX  MAX  --> 5    MAX
    //   MAX  MAX  7    MAX      MAX  7
    
    std::vector<int> distance_r;
    std::vector<int> distance_c;
    
    //check each row of distance
    for(int r = 0; r < target_count; r++)
    {
        //if non-MAX_VAL found in the row, mark it 1, means not remove
        for(int c = 0; c < track_count; c++)
        {
            if( distance(r,c) != MAX_VAL )
            {
                distance_r.push_back(r);
                break;
            }
        }
    }
    
    //do the same with each column of distance
    for(int c = 0; c < track_count; c++)
    {
        //if non-MAX_VAL found in the row, mark it 1, means not remove
        for(int r = 0; r < target_count; r++)
        {
            if( distance(r,c) != MAX_VAL )
            {
                distance_c.push_back(c);
                break;
            }
        }
    }
    
    int cost_size = 0;
    
    if( distance_r.size() > distance_c.size() )
    {
        cost_size = distance_r.size();
    }
    else
    {
        cost_size = distance_c.size();
    }
    
    //now distance_r and distance_c marks with row and column to save
    //make cost matrix into a square matrix
    matrix<double>  cost(cost_size, cost_size, MAX_VAL);
    
    for (unsigned r = 0; r < distance_r.size(); ++ r)
    {
        for (unsigned c = 0; c < distance_c.size(); ++ c)
        {
            cost(r, c) = distance(distance_r[r], distance_c[c]);
        }
    }
    
    //~ display(cost);
    
    //apply hungarian associator
    std::vector<uint32_t> solutions(cost_size);
    double path_cost = myHunerianAssociator( cost, solutions);
    std::vector<int> target_list;
    std::vector<int> track_list;
 
    //get the assigned target_list indexed in target_input
    //make sure within the gate range
    for (unsigned c = 0; c < distance_c.size(); ++ c)
    {
        if( cost(solutions[c], c) < MAX_VAL )
        {
            target_list.push_back( distance_r[solutions[c]] );
            track_list.push_back( distance_c[c] );
        }
    }
    
    //kalman update for all tracks
    for(int f = 0; f < track_count; ++f)
    {
        //find current track number in track_list
        int index = find_first_index(track_list, f);
        
        //if not found, increase penalty and skip kalman update
        if(index == -1)
	    {
            tracks_all.tracks[f].track_penalty_increase();
            
            continue;
        }
        
        //reset penalty and increse alive
        tracks_all.tracks[f].track_penalty_reset();
        tracks_all.tracks[f].track_alive_increase();
        
        //now actually do the kalman update for current track
        tracks_all.tracks[f].track_kalman_update( track_input[ target_list[index] ] );
        
        //calculate track score
        //~ track_std(t,f) = ( X(1,f)-input_cartesian(2) )^2 + ( X(2,f)-input_cartesian(3) )^2;
    }
    
    //remove tracks
    tracks_all.tracks_remove();
    
    //init new tracks
    //kalman update for all tracks
    for(int i = 0; i < target_count; ++i)
    {
        //find current track number in track_list
        int index = find_first_index(target_list, i);
        
        //if found, the assignments already made, skip
        if(index != -1)
	    {  continue;  }
        
        //init new track
        tracks_all.tracks_add( track_input[i] );
    }
    
    tracks_all.tracks_map_update();
}

#endif
