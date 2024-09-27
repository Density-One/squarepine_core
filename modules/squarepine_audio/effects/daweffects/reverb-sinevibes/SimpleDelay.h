/*
 
 SimpleDelay.h
 
 (c) Artemiy Pavlov
 Sinevibes (www.sinevibes.com)
 
 */

#ifndef SimpleDelay_h
#define SimpleDelay_h

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <cstring>

#ifndef InlineMath_h
#include "InlineMath.h"
#endif

#ifndef LowCutFilter_h
#include "LowCutFilter.h"
#endif

class SimpleDelay
{
    
private:
    
    float *buffer;
    int bufferSize;
    
    int writeFrame;
    
    float sampleRate;
    
    int maxDelaySamples;
    float maxDelayTime;
    
    float feedback;
        
    bool lowCutFilterEnabled;
    LowCutFilter mLowCutFilter;
    
public:
    
    SimpleDelay()
    {
        
        buffer = NULL;
        bufferSize = 0;
        
        writeFrame = 0;
        
        sampleRate = 0.f;
        
        maxDelaySamples = 0;
        maxDelayTime = 1.f;
        
        feedback = 0.f;
        
        lowCutFilterEnabled = false;
        
    }
    
    ~SimpleDelay()
    {
        
#ifdef USE_EXTERNAL_MEMORY_ALLOCATION
        
#else
        
        free( buffer );
        
#endif
        
        bufferSize = 0;
        
    }
    
#ifdef USE_EXTERNAL_MEMORY_ALLOCATION
    
    void setMemory( float *inBuffer, int inBufferSize )
    {
        
        buffer = inBuffer;
        bufferSize = inBufferSize;
        maxDelaySamples = bufferSize - 2;
        
    }
    
#else
    
    void setMaxDelayTime( float inMaxDelayTime )
    {
        
        maxDelayTime = inMaxDelayTime;
        
    }
    
#endif
    
    void setSampleRate( float inSampleRate )
    {
        
        sampleRate = inSampleRate;
        
#ifdef USE_EXTERNAL_MEMORY_ALLOCATION
        
#else
        
        maxDelaySamples = maxDelayTime * sampleRate;
        bufferSize = maxDelaySamples + 2;
        
        buffer = (float*)malloc( sizeof(float) * bufferSize );
        
#endif
        
        mLowCutFilter.setSampleRate( sampleRate );
        
        reset();
        
    }
    
    void enableLowCutFilter()
    {
        
        lowCutFilterEnabled = true;
        
    }
    
    float getMaxDelayTime()
    {
        
        return maxDelaySamples / sampleRate;
        
    }
    
    void reset()
    {
        
        writeFrame = 0;
        
        feedback = 0.f;
        
        if( buffer != NULL )
        {
        
            for( int i = 0; i < bufferSize; i++ )
            {
                buffer[i] = 0.f;
            }
            
        }
        
    }
    
    float getBufferMax()
    {
        
        float bufferMax = 0.f;
        
        for( int i = 0; i < bufferSize; i++ )
        {
            float value = fabsf( buffer[i] );
            if( value > bufferMax )
                bufferMax = value;
        }
        
        return bufferMax;
        
    }
    
    float processFrame( float inFrame, float inTime, float inFeedback, int inInterpolation )
    {
        
        if( buffer == NULL )
            return 0.f;
        
        buffer[writeFrame] = inFrame;
        
        if( inFeedback != 0.f )
            buffer[writeFrame] += inFeedback * feedback;
        
        float delaySamples = sampleRate * inTime;
        
        if( delaySamples < 0.f )
            delaySamples = 0.f;
        else if( delaySamples > (float)maxDelaySamples )
            delaySamples = (float)maxDelaySamples;
        
        float readPosition = (float)writeFrame - delaySamples;
        
        if( readPosition < 0.f )
            readPosition += bufferSize;
        
        int index = readPosition;
        float d = readPosition - index;
        
        float sample;
        
        if( inInterpolation == 0 )
        {
            
            // linear interpolation
        
            int x0 = index - 1;
            if( x0 < 0 )
                x0 += bufferSize;
            
            int x1 = index;
            if( x1 >= bufferSize )
                x1 -= bufferSize;
                    
            sample = inlineLinearInterpolation( buffer[x0], buffer[x1], d );

        } else {
            
            // spline interpolation
            
            int x0 = index - 1;
            if( x0 < 0 )
                x0 += bufferSize;
            
            int x1 = index;
            if( x1 >= bufferSize )
                x1 -= bufferSize;
            
            int x2 = index + 1;
            if( x2 >= bufferSize )
                x2 -= bufferSize;
            
            int x3 = index + 2;
            if( x3 >= bufferSize )
                x3 -= bufferSize;
            
            sample = inlineSplineInterpolation( buffer[x0], buffer[x1], buffer[x2], buffer[x3], d );
            
        }
        
        if( lowCutFilterEnabled )
            sample = mLowCutFilter.processFrame( sample );
        
        feedback = sample;
        
        writeFrame++;
        
        if( writeFrame >= bufferSize )
            writeFrame -= bufferSize;
        
        return sample;
        
    }
    
    void processBlock( float *inBlock, float *inTime, float *inFeedback, int inInterpolation, float *outBlock, int blockSize )
    {
        
        if( buffer == NULL )
            return;
        
        for( int frame = 0; frame < blockSize; frame++ )
        {
            
            buffer[writeFrame] = inBlock[frame];
            
            if( inFeedback != NULL )
                buffer[writeFrame] += feedback * inFeedback[frame];
            
            float delaySamples = sampleRate * inTime[frame];
            
            if( delaySamples < 0.f )
                delaySamples = 0.f;
            else if( delaySamples > (float)maxDelaySamples )
                delaySamples = (float)maxDelaySamples;
            
            float readPosition = (float)writeFrame - delaySamples;
            
            if( readPosition < 0.f )
                readPosition += bufferSize;
            
            int index = readPosition;
            float d = readPosition - index;
            
            float sample;
            
            if( inInterpolation == 0 )
            {
                
                // linear interpolation
            
                int x0 = index - 1;
                if( x0 < 0 )
                    x0 += bufferSize;
                
                int x1 = index;
                if( x1 >= bufferSize )
                    x1 -= bufferSize;
                        
                sample = inlineLinearInterpolation( buffer[x0], buffer[x1], d );

            } else {
                
                // spline interpolation
                
                int x0 = index - 1;
                if( x0 < 0 )
                    x0 += bufferSize;
                
                int x1 = index;
                if( x1 >= bufferSize )
                    x1 -= bufferSize;
                
                int x2 = index + 1;
                if( x2 >= bufferSize )
                    x2 -= bufferSize;
                
                int x3 = index + 2;
                if( x3 >= bufferSize )
                    x3 -= bufferSize;
                
                sample = inlineSplineInterpolation( buffer[x0], buffer[x1], buffer[x2], buffer[x3], d );
                
            }
            
            if( lowCutFilterEnabled )
                sample = mLowCutFilter.processFrame( sample );
            
            feedback = sample;
            
            writeFrame++;
            
            if( writeFrame >= bufferSize )
                writeFrame -= bufferSize;
            
            outBlock[frame] = sample;
            
        }
        
    }
    
};

#endif /* SimpleDelay_h */
