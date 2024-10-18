/*
 
 NaturalReverb8x8.h
 
 (c) Artemiy Pavlov
 Sinevibes (www.sinevibes.com)
 
 */

#ifndef NaturalReverb8x8_h
#define NaturalReverb8x8_h

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <cstring>

//#ifndef AllPassFilter_h
//#include "AllPassFilter.h"
//#endif

#ifndef DampingFilter_h
#include "DampingFilter.h"
#endif

#ifndef SimpleDelay_h
#include "SimpleDelay.h"
#endif

#ifndef SimpleLFO_h
#include "SimpleLFO.h"
#endif

#ifndef ChaosLFO_h
#include "ChaosLFO.h"
#endif

#ifndef DELAY_TIME_ARRAY
#define DELAY_TIME_ARRAY

static float delayTime[168] = { 0.002f, 0.003f, 0.005f, 0.007f, 0.011f, 0.013f, 0.017f, 0.019f, 0.023f, 0.029f, 0.031f, 0.037f, 0.041f, 0.043f, 0.047f, 0.053f, 0.059f, 0.061f, 0.067f, 0.071f, 0.073f, 0.079f, 0.083f, 0.089f, 0.097f, 0.101f, 0.103f, 0.107f, 0.109f, 0.113f, 0.127f, 0.131f, 0.137f, 0.139f, 0.149f, 0.151f, 0.157f, 0.163f, 0.167f, 0.173f, 0.179f, 0.181f, 0.191f, 0.193f, 0.197f, 0.199f, 0.211f, 0.223f, 0.227f, 0.229f, 0.233f, 0.239f, 0.241f, 0.251f, 0.257f, 0.263f, 0.269f, 0.271f, 0.277f, 0.281f, 0.283f, 0.293f, 0.307f, 0.311f, 0.313f, 0.317f, 0.331f, 0.337f, 0.347f, 0.349f, 0.353f, 0.359f, 0.367f, 0.373f, 0.379f, 0.383f, 0.389f, 0.397f, 0.401f, 0.409f, 0.419f, 0.421f, 0.431f, 0.433f, 0.439f, 0.443f, 0.449f, 0.457f, 0.461f, 0.463f, 0.467f, 0.479f, 0.487f, 0.491f, 0.499f, 0.503f, 0.509f, 0.521f, 0.523f, 0.541f, 0.547f, 0.557f, 0.563f, 0.569f, 0.571f, 0.577f, 0.587f, 0.593f, 0.599f, 0.601f, 0.607f, 0.613f, 0.617f, 0.619f, 0.631f, 0.641f, 0.643f, 0.647f, 0.653f, 0.659f, 0.661f, 0.673f, 0.677f, 0.683f, 0.691f, 0.701f, 0.709f, 0.719f, 0.727f, 0.733f, 0.739f, 0.743f, 0.751f, 0.757f, 0.761f, 0.769f, 0.773f, 0.787f, 0.797f, 0.809f, 0.811f, 0.821f, 0.823f, 0.827f, 0.829f, 0.839f, 0.853f, 0.857f, 0.859f, 0.863f, 0.877f, 0.881f, 0.883f, 0.887f, 0.907f, 0.911f, 0.919f, 0.929f, 0.937f, 0.941f, 0.947f, 0.953f, 0.967f, 0.971f, 0.977f, 0.983f, 0.991f, 0.997f };

#endif

class NaturalReverb8x8_Core
{
    
private:
    
    int maxBlockSize;
    
    SimpleDelay mDelay[8];
//    AllPassFilter mAllPassFilter[8];
    DampingFilter mDampingFilter;
    ChaosLFO mScatterLFO[8];
    
    float input[8];
    float output[8];
    float feedback[8];
    
    float *noise[8];
    
public:
    
    NaturalReverb8x8_Core()
    {
        
        maxBlockSize = 16384;
        
        for( int i = 0; i < 8; i++ )
        {
            noise[i] = NULL;
        }
        
    }
    
    ~NaturalReverb8x8_Core()
    {
        
        for( int i = 0; i < 8; i++ )
        {
            free( noise[i] );
        }
        
    }
    
    void setMaxBlockSize( int inMaxBlockSize )
    {
        
        if( inMaxBlockSize > 0 )
            maxBlockSize = inMaxBlockSize;
        
    }
    
    void setSampleRate( float inSampleRate )
    {
        
        for( int i = 0; i < 8; i++ )
        {
            mDelay[i].setMaxDelayTime( 0.5f );
            mDelay[i].setSampleRate( inSampleRate );
            
//            mAllPassFilter[i].setSampleRate( inSampleRate );
            
            mScatterLFO[i].setSampleRate( inSampleRate );
            mScatterLFO[i].setFrequency( 2.5f );
            
            noise[i] = (float*)malloc( maxBlockSize * sizeof(float) );
        }
        
        mDampingFilter.setSampleRate( inSampleRate );
        
        reset();
        
    }
    
    void reset()
    {
        
        for( int i = 0; i < 8; i++ )
        {
            mDelay[i].reset();
            feedback[i] = 0.f;
            
            mScatterLFO[i].reset();
        }
        
        mDampingFilter.reset();
                
    }
    
    float getBufferMax()
    {
        
        float bufferMax = 0.f;
        
        for( int i = 0; i < 8; i++ )
        {
            float value = mDelay[i].getBufferMax();
            if( value > bufferMax )
                bufferMax = value;
        }
        
        return bufferMax;
        
    }
    
    void processBlock( float *inBlock, float *inDecay, float *inScattering, float *inModulation, float *inLowDamping, float *inHighDamping, float inDelayTimeScale, int inInterpolation, float *outBlock, int blockSize )
    {
        
        // noise
        
        for( int i = 0; i < 8; i++ )
        {
            mScatterLFO[i].generateBlock( noise[i], blockSize );
        }
                
        for( int frame = 0; frame < blockSize; frame++ )
        {
            
            // delay times

            float time0 = delayTime[8];
            float time1 = delayTime[11];
            float time2 = delayTime[14];
            float time3 = delayTime[17];
            float time4 = delayTime[20];
            float time5 = delayTime[23];
            float time6 = delayTime[27];
            float time7 = delayTime[30];
            
            time0 += 0.0025f * inScattering[frame] * noise[1][frame];
            time1 += 0.0025f * inScattering[frame] * noise[1][frame];
            time2 += 0.0025f * inScattering[frame] * noise[2][frame];
            time3 += 0.0025f * inScattering[frame] * noise[3][frame];
            time4 += 0.0025f * inScattering[frame] * noise[4][frame];
            time5 += 0.0025f * inScattering[frame] * noise[5][frame];
            time6 += 0.0025f * inScattering[frame] * noise[6][frame];
            time7 += 0.0025f * inScattering[frame] * noise[7][frame];
            
            // damping filters
            
            mDampingFilter.setDamping( inLowDamping[frame], inHighDamping[frame] );
            
            for( int i = 0; i < 8; i++ )
            {
                feedback[i] = mDampingFilter.processFrame( feedback[i], i );
            }
            
            // feedback matrix
                        
            input[0] = inBlock[frame] + inDecay[frame] * ( feedback[0] + feedback[1] + feedback[2] + feedback[3] + feedback[4] + feedback[5] + feedback[6] + feedback[7] );
            input[1] = inDecay[frame] * ( feedback[0] - feedback[1] + feedback[2] - feedback[3] + feedback[4] - feedback[5] + feedback[6] - feedback[7] );
            input[2] = inDecay[frame] * ( feedback[0] + feedback[1] - feedback[2] - feedback[3] + feedback[4] + feedback[5] - feedback[6] - feedback[7] );
            input[3] = inDecay[frame] * ( feedback[0] - feedback[1] - feedback[2] + feedback[3] + feedback[4] - feedback[5] - feedback[6] + feedback[7] );
            input[4] = inDecay[frame] * ( feedback[0] + feedback[1] + feedback[2] + feedback[3] - feedback[4] - feedback[5] - feedback[6] - feedback[7] );
            input[5] = inDecay[frame] * ( feedback[0] - feedback[1] + feedback[2] - feedback[3] - feedback[4] + feedback[5] - feedback[6] + feedback[7] );
            input[6] = inDecay[frame] * ( feedback[0] + feedback[1] - feedback[2] - feedback[3] - feedback[4] - feedback[5] + feedback[6] + feedback[7] );
            input[7] = inDecay[frame] * ( feedback[0] - feedback[1] - feedback[2] + feedback[3] - feedback[4] + feedback[5] + feedback[6] - feedback[7] );
            
            // run the delays
            
            output[0] = mDelay[0].processFrame( input[0], time0 + inModulation[frame], 0.f, inInterpolation );
            output[1] = mDelay[1].processFrame( input[1], time1 - inModulation[frame], 0.f, inInterpolation );
            output[2] = mDelay[2].processFrame( input[2], time2 + inModulation[frame], 0.f, inInterpolation );
            output[3] = mDelay[3].processFrame( input[3], time3 - inModulation[frame], 0.f, inInterpolation );
            output[4] = mDelay[4].processFrame( input[4], time4 + inModulation[frame], 0.f, inInterpolation );
            output[5] = mDelay[5].processFrame( input[5], time5 - inModulation[frame], 0.f, inInterpolation );
            output[6] = mDelay[6].processFrame( input[6], time6 + inModulation[frame], 0.f, inInterpolation );
            output[7] = mDelay[7].processFrame( input[7], time7 - inModulation[frame], 0.f, inInterpolation );
            
            // all-pass filters
            
//            output[0] = mAllPassFilter[0].processFrame( output[0], 1.f/time0, 1.f );
//            output[1] = mAllPassFilter[1].processFrame( output[1], 1.f/time1, 1.f );
//            output[2] = mAllPassFilter[2].processFrame( output[2], 1.f/time2, 1.f );
//            output[3] = mAllPassFilter[3].processFrame( output[3], 1.f/time3, 1.f );
//            output[4] = mAllPassFilter[4].processFrame( output[4], 1.f/time4, 1.f );
//            output[5] = mAllPassFilter[5].processFrame( output[5], 1.f/time5, 1.f );
//            output[6] = mAllPassFilter[6].processFrame( output[6], 1.f/time6, 1.f );
//            output[7] = mAllPassFilter[7].processFrame( output[7], 1.f/time7, 1.f );
            
            // store feedback
                        
            feedback[0] = output[0];
            feedback[1] = output[1];
            feedback[2] = output[2];
            feedback[3] = output[3];
            feedback[4] = output[4];
            feedback[5] = output[5];
            feedback[6] = output[6];
            feedback[7] = output[7];
            
            // output
            
            outBlock[frame] = output[1];
//            outBlock[frame] = noise[0][frame];
                        
        }
        
    }
    
};

class NaturalReverb8x8
{
    
private:
    
    NaturalReverb8x8_Core mNaturalReverb8x8L;
    NaturalReverb8x8_Core mNaturalReverb8x8R;
    
    SimpleLFO mLFO;
    
    SimpleDelay mPreDelayL;
    SimpleDelay mPreDelayR;
    
    int maxBlockSize;
    
    float *decay;
    float *modulation;
    
public:
    
    NaturalReverb8x8()
    {

        maxBlockSize = 16384;
        
        decay = NULL;
        modulation = NULL;
        
    }
    
    ~NaturalReverb8x8()
    {
        
        free( decay );
        free( modulation );
        
    }
    
    void setMaxBlockSize( int inMaxBlockSize )
    {
        
        if( inMaxBlockSize > 0 )
            maxBlockSize = inMaxBlockSize;
        
        mNaturalReverb8x8L.setMaxBlockSize( maxBlockSize );
        mNaturalReverb8x8R.setMaxBlockSize( maxBlockSize );
        
    }
    
    void setSampleRate( float inSampleRate )
    {
        
        mNaturalReverb8x8L.setSampleRate( inSampleRate );
        mNaturalReverb8x8R.setSampleRate( inSampleRate );
        
        mPreDelayL.setSampleRate( inSampleRate );
        mPreDelayR.setSampleRate( inSampleRate );
        
        mLFO.setSampleRate( inSampleRate );
        
        decay = (float*)malloc( maxBlockSize * sizeof(float) );
        modulation = (float*)malloc( maxBlockSize * sizeof(float) );
        
    }
    
    void reset()
    {
        
        mNaturalReverb8x8L.reset();
        mNaturalReverb8x8R.reset();
        
        mPreDelayL.reset();
        mPreDelayR.reset();
        
    }
    
    float getBufferMax()
    {
        
        return mNaturalReverb8x8L.getBufferMax() + mNaturalReverb8x8R.getBufferMax();
        
    }
    
    void processBlock( float *inBlockL, float *inBlockR, float *inPreDelayTime, float *inDecay, float *inScattering, float *inModFrequency, float *inModDepth, float *inLowDamping, float *inHighDamping, int inInterpolation, float *outBlockL, float *outBlockR, int blockSize )
    {
        
        if( blockSize > maxBlockSize )
            return;
        
        // mono or stereo?
        
        bool stereo = true;
        
        if( inBlockR == NULL || outBlockR == NULL )
            stereo = false;
        
        // run the pre-delay
        
        mPreDelayL.processBlock( inBlockL, inPreDelayTime, NULL, 1, outBlockL, blockSize );
        
        if( stereo )
        {
            mPreDelayR.processBlock( inBlockR, inPreDelayTime, NULL, 1, outBlockR, blockSize );
        }
        
        // curve and limit decay
        
        for( int frame = 0; frame < blockSize; frame++ )
        {
            float x = 0.25f + 0.75f * inDecay[frame];
            x = 1.f - x;
            decay[frame] = 0.3535f * ( 1.f - x * x * x );
        }
        
        // modulation
        
        mLFO.generateBlock( inModFrequency, NULL, modulation, blockSize );
        
        for( int frame = 0; frame < blockSize; frame++ )
        {
            modulation[frame] *= 0.001f * inModDepth[frame];
        }
        
        // run the reverb
        
        mNaturalReverb8x8L.processBlock( outBlockL, decay, inScattering, modulation, inLowDamping, inHighDamping, +0.0001f, inInterpolation, outBlockL, blockSize );
        
        if( stereo )
            mNaturalReverb8x8R.processBlock( outBlockR, decay, inScattering, modulation, inLowDamping, inHighDamping, -0.0001f, inInterpolation, outBlockR, blockSize );
        
    }
    
};

#endif /* NaturalReverb8x8_h */
