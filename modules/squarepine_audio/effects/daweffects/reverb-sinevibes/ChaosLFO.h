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
    
    float frequency;
    float phase;
    
    float memory;
    float slope;
    
    float value1;
    float value2;
    float delta;
    
    bool needsToReset;
    
public:
    
    ChaosLFO()
    {
        
        dt = 1.f / 44100.f;
        
        frequency = 1.f;
        phase = 0.f;
        
        memory = 0.f;
        slope = 0.f;
        
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
        
        frequency = inFrequency;
        
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
                
                phase = 0.f;
                
                memory = 2.f * (float)rand()/RAND_MAX - 1.f;
                
                float target = 2.f * (float)rand()/RAND_MAX - 1.f;
                slope = ( target - memory ) * frequency * dt;
                  
                needsToReset = false;
                
            }
            
            phase += frequency * dt;
                                        
            memory += slope;
            
            value1 += delta * ( memory - value1 );
            value2 += delta * ( value1 - value2 );
            
            outBlock[frame] = value2;
            
            if( phase >= 1.f )
            {
                phase -= 1.f;
                
                float target = 2.f * (float)rand()/RAND_MAX - 1.f;
                slope = ( target - memory ) * frequency * dt;
            }
                
        }
        
    }
    
};

#endif
