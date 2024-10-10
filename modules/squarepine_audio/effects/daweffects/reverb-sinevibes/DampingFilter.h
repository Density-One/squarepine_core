/*
 
 DampingFilter.h
 
 (c) Artemiy Pavlov
 Sinevibes (www.sinevibes.com)
 
 */

#ifndef DampingFilter_h
#define DampingFilter_h

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <cstring>

class OnePoleLowPass
{
    
private:
    
    float dt;
    
    float delta;
    
    float value[16];
    
public:
    
    OnePoleLowPass()
    {
        
        dt = 0.f;
        
        delta = 0.f;
        
        reset();
        
    }
    
    void setSampleRate( float inSampleRate )
    {
        
        dt = 2.f * M_PI / inSampleRate;
        
    }
    
    void setFrequency( float inFrequency )
    {
        
        delta = 1.f - expf( - inFrequency * dt );
        
    }
    
    void reset()
    {
        
        for( int i = 0; i < 16; i++ )
        {
            value[i] = 0.f;
        }
        
    }
    
    float processFrame( float inFrame, int inIndex )
    {
                
        value[inIndex] += delta * ( inFrame - value[inIndex] );
        
        return value[inIndex];
        
    }
    
};

class OnePoleHighPass
{
    
private:
    
    float dt;
    
    float delta;
    
    float value[16];
    
public:
    
    OnePoleHighPass()
    {
        
        dt = 0.f;
        
        delta = 0.f;
        
        reset();
        
    }
    
    void setSampleRate( float inSampleRate )
    {
        
        dt = 2.f * M_PI / inSampleRate;
        
    }
    
    void setFrequency( float inFrequency )
    {
        
        delta = 1.f - expf( - inFrequency * dt );
        
    }
    
    void reset()
    {
        
        for( int i = 0; i < 16; i++ )
        {
            value[i] = 0.f;
        }
        
    }
    
    float processFrame( float inFrame, int inIndex )
    {
                
        value[inIndex] += delta * ( inFrame - value[inIndex] );
        
        return inFrame - value[inIndex];
        
    }
    
};

class DampingFilter
{
    
private:
    
    OnePoleLowPass mLPF;
    OnePoleHighPass mHPF;
    
    float lowDamping;
    float highDamping;
    
public:
    
    DampingFilter()
    {
        
        lowDamping = 0.f;
        highDamping = 0.f;
        
    }
    
    void setSampleRate( float inSampleRate )
    {
        
        mLPF.setSampleRate( inSampleRate );
        mHPF.setSampleRate( inSampleRate );
        
    }
    
    void reset()
    {
        
        mLPF.reset();
        mHPF.reset();
        
    }
    
    void setDamping( float inLowDamping, float inHighDamping )
    {
        
        if( lowDamping != inLowDamping )
        {
            mLPF.setFrequency( inLowDamping );
            lowDamping = inLowDamping;
        }
        
        if( highDamping != inHighDamping )
        {
            mHPF.setFrequency( inHighDamping );
            highDamping = inHighDamping;
        }
        
    }
        
    float processFrame( float inFrame, int inIndex )
    {
                
        // high-pass filter
        
        float output = mHPF.processFrame( inFrame, inIndex );
        
        // low-pass filter
        
        output = mLPF.processFrame( output, inIndex );
        
        // output
        
        return output;
        
    }
    
};

#endif /* DampingFilter_h */
