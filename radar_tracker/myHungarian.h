//https://www.boost.org/doc/libs/1_71_0/libs/numeric/ublas/doc/index.html
//usage: //~ double path_cost = myHunerianAssociator( test, solutions);

#ifndef MYHUNGARIAN_H_
#define MYHUNGARIAN_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>

using namespace boost::numeric::ublas;

void display( matrix<double> &input )
{
    for(unsigned r = 0; r < input.size1(); ++ r)
    {
        for(unsigned c = 0; c < input.size2(); ++ c)
        {
            printf("%5.f, ", input(r, c));
        }
        
        printf("\n");
    }
	
	printf("\n");
}

//print out array value
void display_1d( std::vector<uint32_t> &input )
{
    for(int x = 0; x < input.size(); x++)
    {
        printf("%4d, ", input[x]);
    }
    
    printf("\n");
}

//set all array value to 0, 1d
void array_1d_set_all( std::vector<uint32_t> &input, uint32_t value )
{
    for(int x = 0; x < input.size(); x++)
    {
        input[x] = value;
    }
}

//find minimum value and the index in an array, 1d, non zeros
void array_1d_min( std::vector<uint32_t> &input, uint32_t &index, uint32_t &minimum)
{
    for(int x = 0; x < input.size(); x++)
    {
        if(input[x] == 0)
        {  continue; }
        
        if(input[x] < minimum)
        {
            minimum = input[x];
            index = x;
        }
    }
}

uint32_t array_1d_count_1( std::vector<uint32_t> &input)
{
    uint32_t count = 0;
    
    //for each row, count number of 0s
    for(int r = 0; r < input.size(); r++)
    {
        if( input[r] == 1 )
        {
            count++;
        }
    }
    
    return count;
}



//search matrix for the value, return ture if found
uint8_t array_2d_search( matrix<double> &input, double value)
{
    for(int r = 0; r < input.size1(); r++)
    {
        for(int c = 0; c < input.size2(); c++)
        {
            if(input(r,c) == value)
            {  return 1;  }
        }
    }
    
    return 0;
}

//set all original value in matrix to target
void array_2d_set_to( matrix<double> &input, double original, double target)
{
    for(int r = 0; r < input.size1(); r++)
    {
        for(int c = 0; c < input.size2(); c++)
        {
            if(input(r,c) == original)
            {  input(r,c) = target;  }
        }
    }
}

//set all matrix value to random number
void array_2d_random( matrix<double> &input )
{
    for(unsigned r = 0; r < input.size1(); ++ r)
    {
        for(unsigned c = 0; c < input.size2(); ++ c)
        {
            input(r, c) = rand()%100;
        }
    }
}

//count the number of 0s
void array_2d_count_0( matrix<double> &input, std::vector<uint32_t> &num_0_r, std::vector<uint32_t> &num_0_c )
{
    //for each row, count number of 0s
    for(int r = 0; r < input.size1(); r++)
    {
        //count the number of 0s
        for(int c = 0; c < input.size2(); c++)
        {
            if( input(r,c) == 0 )
            {
                num_0_r[r]++;
            }
        }
    }
    
    // now do the same with column
    for(int c = 0; c < input.size2(); c++)
    {
        //find the minimum value 
        for(int r = 0; r < input.size1(); r++)
        {
            if( input(r,c) == 0 )
            {
                num_0_c[c]++;
            }
        }
    }
}
    
//minus the minimum value for each row and column
void array_2d_minus_mini( matrix<double> &input )
{
    double minimum = 0;
    
    //for each row, find minimum
    for(int r = 0; r < input.size1(); r++)
    {
        //get current row
        matrix_row< matrix<double> > current_row(input, r);
        
        //find minimum number
        auto result = std::min_element(current_row.begin(), current_row.end());
        
        minimum = *result;
        
        //minus the minimun number for each row
        for( auto &i : current_row )
        {
            i -= minimum;
        }
    }
    
    //now do the same with column
    for(int c = 0; c < input.size2(); c++)
    {
        //get current row
        matrix_column< matrix<double> > current_column(input, c);
        
        //find minimum number
        auto result = std::min_element(current_column.begin(), current_column.end());
        
        minimum = *result;
        
        //minus the minimun number for each row
        for( auto &i : current_column )
        {
            i -= minimum;
        }
    }
}


uint8_t myCheckPermutation( matrix<double> &input )
{
    uint32_t index_0_count = 0;
    uint8_t  result  = 1;
    
    // if there is only one -2(dominal 0) in each row, then its a permutation matrix
    // otherwise it is not
    for( int r = 0; r < input.size1(); r++)
    {
        //reset flags for each row
        index_0_count = 0;
            
        //count the number of -2 in each row
        for(int c = 0; c < input.size2(); c++)
        {
            if( input(r,c) == -2 )
            {
                index_0_count++;
            }
        }
        
        //check if only one -2 found
        if(index_0_count != 1)
        {
            //printf("r = %d, idx_count = %d", r, index_0_count);
            result = 0;
            break;
        }
    }
    
    //~ printf("\ncheck 1 = \n");
    //~ array_2d_display(input);
    //~ printf("\n");
    
    return result;
}


//check if the input matrix is(can be) permutation matrix or not
//the reset flag decides if -2s and -1s are set back to 0s
uint8_t myFindPermutation( matrix<double> &input, uint8_t reset)
{
    
    uint8_t result  = 1;
    uint8_t finish  = 0;
    uint8_t found_1 = 0;
    uint8_t search_direction = 0;
    uint8_t default_direction = 0;
    
    uint32_t index_0 = 0;
    uint32_t index_0_count = 0;
    
    std::vector<uint32_t>  num_0_r(input.size1());
    std::vector<uint32_t>  num_0_c(input.size2());
    
    while(finish == 0)
    {
        array_1d_set_all(num_0_r, 0);
        array_1d_set_all(num_0_c, 0);
        
        //one iteration to remove reductant 0s
        //process each row
        for( int r = 0; r < input.size1(); r++)
        {
            //reset flags for each row
            index_0 = 0;
            index_0_count = 0;
            
            //count the number of 0s in each row, and save the index for the first 0
            for(int c = 0; c < input.size2(); c++)
            {
                //save the index
                if(input(r,c) == 0)
                {
                    index_0 = c;
                    index_0_count++;
                }
                
                //skip if more than 1 0s found
                if(index_0_count > 1)
                {   break;   }
            }
            
            //if only one 0 in this line
            if(index_0_count == 1)
            {
                //mark it to -2
                input(r,index_0) = -2;
                
                //mark all 0s in the same column to -1
                for(int rr = 0; rr < input.size1(); rr++)
                {
                    if( input(rr,index_0) == 0 )
                    {
                        input(rr,index_0) = -1;
                    }
                } 
            }
        }
        
        //do the same to each column
        for( int c = 0; c < input.size2(); c++)
        {
            //reset flags for each row
            index_0 = 0;
            index_0_count = 0;
            
            //count the number of 0s in each column, and save the index for the first 0
            for(int r = 0; r < input.size1(); r++)
            {
                //save the index
                if( input(r,c) == 0 )
                {
                    index_0 = r;
                    index_0_count++;
                }
                
                //skip if more than 1 0s found
                if(index_0_count > 1)
                {   break;   }
            }
            
            //if only one 0 in this line
            if(index_0_count == 1)
            {
                //mark it to -2
                input(index_0,c) = -2;
                
                //mark all 0s in the same column to -1
                for(int cc = 0; cc < input.size2(); cc++)
                {
                    if( input(index_0,cc) == 0 )
                    {
                        input(index_0,cc) = -1;
                    }
                } 
            }
        }

        //reset flag
        index_0_count = 0;
        
        index_0_count = array_2d_search(input, 0);
        
        // if no more 0 found, then finish
        if(index_0_count == 0)
        {
            result = myCheckPermutation(input);
            
            //if no result found at search direction 0(row first)
            //then change direction to 1(column first)
            if( (result == 0)&&(default_direction == 0) )
            {
                //set the -1s, -2s back to 0 if finished
                array_2d_set_to(input, -2, 0);
                array_2d_set_to(input, -1, 0);
                
                default_direction = 1;
                
                //printf("search direction changed\n");
            }
            // result=1 or (restul=0, default_direction=1)
            else
            {
                finish = 1;
            }
        }
        else //we check if we need to continue the iterations
        {
            array_2d_count_0(input, num_0_r, num_0_c);
            
            int num_1_r = array_1d_count_1(num_0_r);
            int num_1_c = array_1d_count_1(num_0_c);
            
            // both num_1_r and num_1_c are 0, means all rows and columns
            // have more than one 0s, then its no possible to iterate through 
            // all 0s therefore in this case, multiple solution exists
            // 
            // so count all remaining 0s of each row/column,
            // find the row/column that has least 0s,
            // then mark the first 0 to -2 and continue
            if( (num_1_r == 0)&&(num_1_c == 0) )
            {
                uint32_t num_0_r_min = input.size1();
                uint32_t num_0_c_min = input.size2();
                uint32_t num_0_r_min_idx = 0;
                uint32_t num_0_c_min_idx = 0;
                
                int check_r = 0;
                int check_c = 0;
                
                //find minimum count of 0s for row/col, none 0 value
                array_1d_min(num_0_r, num_0_r_min_idx, num_0_r_min);
                array_1d_min(num_0_c, num_0_c_min_idx, num_0_c_min);
                
                //array_1d_display(num_0_r);
                //array_1d_display(num_0_c);
                
                //printf("num_0_r_min_idx = %d, num_0_r_min = %d\n", num_0_r_min_idx, num_0_r_min);
                
                //row has less 0s, so modifiy row
                if( num_0_r_min < num_0_c_min )
                {
                    search_direction == 0;
                }
                //modifiy column if column has less 0s
                else if( num_0_r_min > num_0_c_min )
                {
                    search_direction == 1;
                }
                
                //very special case !!! num_0_r_min = num_0_c_min
                //we first modify row, if no permutation found,
                //then we switch to modify column
                //this section makes the code more complex
                else if( num_0_r_min == num_0_c_min )
                {
                    search_direction = default_direction;
                }
                
                //printf("num_0_r_min = %d, num_0_c_min = %d, direction = %d, default = %d\n", num_0_r_min, num_0_c_min, search_direction, default_direction);
                
                //modifiy row
                if( search_direction == 0 )
                {
                    //find the first 0 in that row
                    for(int c = 0; c < input.size2(); c++)
                    {
                        if( input(num_0_r_min_idx,c)==0 )
                        {
                            check_r = num_0_r_min_idx;
                            check_c = c;
                            break;
                        }
                    }
                }
                //modifiy column
                else if( search_direction == 1 )
                {
                    //find the first 0 in that column
                    for(int r = 0; r < input.size1(); r++)
                    {
                        if( input(r,num_0_c_min_idx)==0 )
                        {
                            check_r = r;
                            check_c = num_0_c_min_idx;
                            break;
                        }
                    }
                }
                
                //set [check_r, check_c] to -2
                input(check_r,check_c) = -2;
                //printf("r = %d, c = %d\n", check_r, check_c);
                
                //update rows and columns
                //all 0s on the same row and column is set to -1
                for(int r = 0; r < input.size1(); r++)
                {
                    if( input(r,check_c) == 0 )
                    {
                        input(r,check_c) = -1;
                    }
                }
                
                for(int c = 0; c < input.size2(); c++)
                {
                    if( input(check_r,c) == 0 )
                    {
                        input(check_r,c) = -1;
                    }
                }
                
            }
        }
    }
    
    //check result, it is done earlier already
    //result = myCheckPermutation(input);
    
    //reset matrix to original, set all -2s and -1s to 0
    if(reset == 1)
    {
        array_2d_set_to(input, -2, 0);
        array_2d_set_to(input, -1, 0);
    }
    
    return result;
}



//this function find the minimum number of lines that covers the 0s in the matrix
void myFind0CoverLine( matrix<double> &input, std::vector<uint32_t> &index_r, std::vector<uint32_t> &index_c )
{
    uint8_t result = 1;
    uint8_t finish = 0;
    uint8_t found_1 = 0;
    
    int index_1_count = 0;
    int index_2 = 0;
    
    // index_r and index_c are flags that marks the line for the matrix
    // bool value, 0 or 1
    // index_r: [x][x][x][x]...[x]
    // index_c: [x][x][x][x]...[x]
    
    //process all 0s, set them to -2s or -1s, if permuation found, finish
    finish = myFindPermutation(input, 0);
    
    //for each row, find rows that does not contain -2(single 0s)
    //there can only be at most one -2 each row/column
    for( int r = 0; r < input.size1(); r++)
    {
        //mark the index_r[r] each time a -2 is found
        //the number of -2s does not matter
        for(int c = 0; c < input.size2(); c++)
        {
            if( input(r,c) == -2 )
            {
                index_r[r] = 1;
            }
        }
        
        //reverse index_r[r]
        if(index_r[r] == 1)       //-2s in this line
            {  index_r[r] = 0;  }
        else if(index_r[r] == 0)   //no -2 in this line
            {  index_r[r] = 1;  }
    }
    
    while(finish == 0)
    {
        //for each marked row in index_r[r]
        for(int r = 0; r < input.size1(); r++)
        {
            //skip if the row has one -2
            if(index_r[r] == 0)
            {  continue;  }
            
            index_1_count = 0;
            
            //for each -1 in the same line that has no -2
            //find the columns of each -1s, could be more than one -1
            //mark the index_c[c] of each -1 for each row, and count number of -1s
            for(int c = 0; c < input.size2(); c++)
            {
                if( input(r,c) == -1 )
                {
                    index_2 = 0;
                    
                    //reset
                    input(r,c) = 0;
                    
                    //for each -1, find -2 in the same column, can be at most one
                    //and add it to index_r
                    for(int rr = 0; rr < input.size1(); rr++)
                    {
                        //find -2 in the column c
                        if( input(rr,c) == -2 )
                        {
                            //mark in index_r
                            index_r[rr] = 1;
                            
                            //reset
                            input(rr,c) == 0;
                            index_2 = 1;
                            
                            break;
                        }
                    }
                    
                    //only set the column index if -2 found in the same column
                    //if(index_2 == 1)
                    {  index_c[c] = 1;  }
                    
                }
            }
            
        }
        
        found_1 = 0;
        
        // check input[index_r][:] for -1, if all -1 are processed, then finish
        // otherwise continue iteration
        for(int r = 0; r < input.size1(); r++)
        {
            //skip if the row is marked in index_r
            if(index_r[r] == 0)
            {  continue;  }
            
            for(int c = 0; c < input.size2(); c++)
            {
                if( input(r,c) == -1)
                {
                    found_1 = 1;
                    r = c = input.size1();
                    break;
                }
            }
        }
        
        if(found_1 == 0) {  finish = 1;  }
    }
    
    //reverse index_r[r] again
    for( int r = 0; r < input.size1(); r++)
    {
        if(index_r[r] == 1)
            {  index_r[r] = 0;  }
        else if(index_r[r] == 0)
            {  index_r[r] = 1;  }
    }
    
    //set the -1s, -2s back to 0 if finished
    array_2d_set_to(input, -2, 0);
    array_2d_set_to(input, -1, 0);
    
    
    //~ printf("\ncoverline check = \n");
    //~ array_2d_display(input);
    //~ printf("\n");
}

uint32_t line_num = 0;

//
int32_t myHunerianAssociator( matrix<double> &input, std::vector<uint32_t> &solutions )
{

    //copy input to a temp matrix
    matrix<double> input_tmp = input;
    
    uint8_t result  = 1;
    uint8_t finish  = 0;
    
    
    std::vector<uint32_t> index_r(input_tmp.size1());
    std::vector<uint32_t> index_c(input_tmp.size2());
    
    double cost_min = 0;
    double path_cost  = 0;
    
    array_2d_minus_mini(input_tmp);
    
    result = myFindPermutation(input_tmp, 0);
    
    //~ int t = 0;
    while(result == 0)
    {
        //~ t++;
        //~ printf("t = %d\n", t);
        
        //array_2d_display(input_tmp);
        //printf("\n\n");
        line_num = 0;
        
        //return input_tmp to normal
        array_2d_set_to(input_tmp, -2, 0);
        array_2d_set_to(input_tmp, -1, 0);
        
        //reset cover line
        array_1d_set_all(index_r, 0);
        array_1d_set_all(index_c, 0);
        
        //find minimum 0 cover line
        
        //printf("flag 1 \n");
        
        myFind0CoverLine(input_tmp, index_r, index_c);
        
        //count the number of total cover line
        //if equal to the number of row, then finish
        for(int i = 0; i < input_tmp.size1(); i++)
        {
            if(index_r[i] == 1)
            {  line_num++;  }
            
            if(index_c[i] == 1)
            {  line_num++;  }
        }
        
        if(line_num == input_tmp.size1())
        {
            printf("limit reached\n");
            
            //~ printf("\ncheckf\n");
            display(input_tmp);
            //~ printf("\n");
            //~ array_1d_display(index_r);
            //~ array_1d_display(index_c);
            
            //~ break;
            exit(1);
        }
        
        //~ array_1d_display(index_r);
        //~ array_1d_display(index_c);
        
        //~ printf("\n\n");
        
        //~ printf("\nmiddle check 1\n");
        //~ array_2d_display(input_tmp);
        //~ printf("\n");
        
        //~ if(t == 10)
        //~ { exit(1); }
        
        //set the cost_min to the first element in dataset
        for(int r = 0; r < input_tmp.size1(); r++)
        {
            //skip if the row is marked in index_r
            if(index_r[r] == 1)
            {
                continue;
            }
            
            for(int c = 0; c < input_tmp.size2(); c++)
            {
                //skip if the column is marked in index_r
                if(index_c[c] == 1)
                {
                    continue;
                }
                
                cost_min = input_tmp(r,c);
                r = c = input_tmp.size1();
                break;
            }
        }
        
        //to find the minimum cost outside the (index_r and index_c)
        for(int r = 0; r < input_tmp.size1(); r++)
        {
            //skip if the row is marked in index_r
            if(index_r[r] == 1)
            {
                continue;
            }
            
            for(int c = 0; c < input_tmp.size2(); c++)
            {
                
                //skip if the column is marked in index_r
                if(index_c[c] == 1)
                {
                    continue;
                }
                
                if(cost_min > input_tmp(r,c))
                {
                    cost_min = input_tmp(r,c);
                }
            }
        }
        
        //update distance matrix
        for(int r = 0; r < input_tmp.size1(); r++)
        {
            for(int c = 0; c < input_tmp.size2(); c++)
            {
                //all non-0-line-covered elements - cost_min
                if( (index_c[c] == 0)&&(index_r[r] == 0) )
                {  
                    input_tmp(r,c) = input_tmp(r,c) - cost_min;
                    
                }
                
                //all 0-line-covered intersections + cost_min
                if( (index_c[c] == 1)&&(index_r[r] == 1) )
                {  
                    //printf("\n+cost_min, r=%d, c=%d\n", r, c);
                    input_tmp(r,c) = input_tmp(r,c) + cost_min;
                    
                }
            }
        }
        
        //~ printf("\nmiddle check 2: cost_min = %f\n", cost_min);
        //~ array_2d_display(input_tmp);
        //~ printf("\n");
        
        result = myFindPermutation(input_tmp, 0);
        
    }
    
    //myCheckPermutation(input, 0);
    //now the position of each -2 in each row is the solution
    
    //find c for each -2, that is the solution
    for(int c = 0; c < input_tmp.size1(); c++)
    {
        for(int r = 0; r < input_tmp.size2(); r++)
        {
            if(input_tmp(r,c) == -2)
            {
                solutions[c] = r;
                break;
            }
        }
        
        //calculate path_cost
        path_cost = path_cost + input(solutions[c],c);
    }
    
    return path_cost;
}


#endif

