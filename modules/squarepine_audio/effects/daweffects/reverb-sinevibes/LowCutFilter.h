/*
 
 LowCutFilter.h
 
 (c) Artemiy Pavlov
 Sinevibes (www.sinevibes.com)
 
 */

#ifndef LowCutFilter_h
#define LowCutFilter_h

#define _USE_MATH_DEFINES
#include <cmath>

class LowCutFilter
{
    
private:
    
    float delta, state, dt;
    
public:
    
    LowCutFilter()
    {
        
        delta = state = dt = 0.f;
                
    }
    
    void reset()
    {
        
        state = 0.f;
        
    }
    
    void setSampleRate( float inSampleRate )
    {
        
        dt = 1.f / inSampleRate;
        
        setFrequency( 40.f );
        
    }
    
    void setFrequency( float inFrequency )
    {
        
        delta = 1.f - expf( - 2.f * M_PI * inFrequency * dt );
        
    }
    
    float processFrame( float inFrame )
    {
        
        state += delta * ( inFrame - state );
        
        return inFrame - state;
        
    }
    
    void processBlock( float *inBlock, float *outBlock, int blockSize )
    {
        
        for( int frame = 0; frame < blockSize; frame++ )
        {
            
            state += delta * ( inBlock[frame] - state );
            
            outBlock[frame] =  inBlock[frame] - state;
            
        }
        
    }
    
};

#endif /* LowCutFilter_h */
