/*
 
 ChaosLFO.h
 
 ©2020 Artemiy Pavlov
 Sinevibes (wwww.sinevibes.com)
 
 */

#ifndef ChaosLFO_h
#define ChaosLFO_h

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <cstring>

class ChaosLFO
{
    
private:
    
    float dt;
    
    float frequency1;
    float frequency2;
    float phase1;
    float phase2;
    
    float memory1;
    float memory2;
    float slope1;
    float slope2;
    
    float value1;
    float value2;
    float delta;
    
    bool needsToReset;
    
public:
    
    ChaosLFO()
    {
        
        dt = 1.f / 44100.f;
        
        frequency1 = 1.f;
        frequency2 = 1.f;
        phase1 = 0.f;
        phase2 = 0.f;
        
        memory1 = 0.f;
        memory2 = 0.f;
        slope1 = 0.f;
        slope2 = 0.f;
        
        value1 = 0.f;
        value2 = 0.f;
        delta = 0.f;
        
        needsToReset = true;
        
    }
    
    void setSampleRate( float inSampleRate )
    {
        
        dt = 1.f / inSampleRate;
        
        reset();
        
    }
    
    void setFrequency( float inFrequency )
    {
        
        frequency1 = inFrequency;
        frequency2 = 5.f * inFrequency;
        
        delta = 1.f - std::expf( - M_PI * inFrequency * dt );
        
    }
        
    void reset()
    {
        
        needsToReset = true;
        
    }
            
    void generateBlock( float *outBlock, int blockSize )
    {
        
        for( int frame = 0; frame < blockSize; frame++ )
        {
        
            if( needsToReset )
            {
                
                phase1 = 0.f;
                phase2 = 0.f;
                
                memory1 = 2.f * (float)rand()/RAND_MAX - 1.f;
                memory2 = 2.f * (float)rand()/RAND_MAX - 1.f;
                
                float target1 = 2.f * (float)rand()/RAND_MAX - 1.f;
                slope1 = ( target1 - memory1 ) * frequency1 * dt;
                float target2 = 2.f * (float)rand()/RAND_MAX - 1.f;
                slope2 = ( target2 - memory2 ) * frequency2 * dt;
                  
                needsToReset = false;
                
            }
            
            phase1 += frequency1 * dt;
            phase2 += frequency2 * dt;
                                        
            memory1 += slope1;
            memory2 += slope2;
            
            value1 += delta * ( 0.35f * memory1 + 0.65f * memory2 - value1 );
            value2 += delta * ( value1 - value2 );
            
            outBlock[frame] = value2;
            
            if( phase1 >= 1.f )
            {
                phase1 -= 1.f;
                
                float target1 = 2.f * (float)rand()/RAND_MAX - 1.f;
                slope1 = ( target1 - memory1 ) * frequency1 * dt;
            }
            
            if( phase2 >= 1.f )
            {
                phase2 -= 1.f;
                
                float target2 = 2.f * (float)rand()/RAND_MAX - 1.f;
                slope2 = ( target2 - memory2 ) * frequency2 * dt;
            }
                
        }
        
    }
    
};

#endif
