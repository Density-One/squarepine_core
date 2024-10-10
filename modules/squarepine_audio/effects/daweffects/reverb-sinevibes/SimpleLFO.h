/*
 
 SimpleLFO.h
 
 (c) Artemiy Pavlov
 Sinevibes (www.sinevibes.com)
 
 */

#ifndef SimpleLFO_h
#define SimpleLFO_h

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <cstring>

#ifndef SinCosTable_h
#include "SinCosTable.h"
#endif

class SimpleLFO
{
    
private:
    
    float phase;
    float dt;
    int targetRestartFrame;
    
    SinCosTable mSinCosTable;
    
public:
    
    SimpleLFO()
    {
        
        phase = 0.f;
        dt = 1.f / 44100.f;
        targetRestartFrame = -1;
        
    }
    
    void setSampleRate( float inSampleRate )
    {
        
        dt = 1.f / inSampleRate;
        
    }
    
    void restartAtFrame( int inFrame )
    {
        
        targetRestartFrame = inFrame;
        
    }
    
    void convertRateToFrequency( float inRate, float inTempo, float *outFrequency, int blockSize ) const
    {
        
        float frequency = inRate * 240.f / inTempo;
        
        for( int frame = 0; frame < blockSize; frame++ )
        {
            outFrequency[frame] = frequency;
        }
        
    }
    
    void generateBlock( float *inFrequency, float *inPhaseShift, float *outBlock, int blockSize )
    {
        
        for( int frame = 0; frame < blockSize; frame++ )
        {
            
            // reset phase at target frame
            
            if( frame == targetRestartFrame )
            {
                
                phase = 0.f;
                targetRestartFrame = -1;
                
            }
            
            // apply phase shift
            
            float shiftedPhase = phase;
            
            if( inPhaseShift != NULL )
            {
                
                shiftedPhase += inPhaseShift[frame];
                
                if( shiftedPhase >= 1.f )
                    shiftedPhase -= 1.f;
                
            }
            
            // generate output waveform
            
            outBlock[frame] = 2.f * M_PI * shiftedPhase;
                          
            // advance & wrap phase
            
            phase += inFrequency[frame] * dt;
            
            if( phase >= 1.f )
                phase -= 1.f;
            else if( phase < 0.f )
                phase += 1.f;
            
        }
        
        // convert phase to sine
        
        mSinCosTable.sinBlock( outBlock, outBlock, blockSize );
        
    }
    
};

#endif /* SimpleLFO_h */
