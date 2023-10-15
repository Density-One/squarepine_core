/*
  ==============================================================================

    FDN.cpp
    Created: 18 May 2021 1:17:02am
    Author:  Eric Tarr

  ==============================================================================
*/

namespace djdawprocessor
{

FDN::FDN()
{
    for (int i = 0; i < NUMDELAYLINES; i++)
    {
        lfoDelayL[i] = new LFODelay (delayMsL[i], modRate[i]);
        lfoDelayR[i] = new LFODelay (delayMsR[i], modRate[i]);
        damp[i] = new DampeningFilter (DAMPAMOUNT);
    }
    
}

// Destructor
FDN::~FDN()
{
    for (int i = 0; i < NUMDELAYLINES; i++)
    {
        delete lfoDelayL[i];
        delete lfoDelayR[i];
        delete damp[i];
    }
}

float FDN::processSample (float x, int channel)
{
    float inDL[NUMDELAYLINES];
    float outDL[NUMDELAYLINES];
    float y = 0;
    for (int n = 0; n < NUMDELAYLINES; n++)
    {
        // Input to delay lines
        inDL[n] = x + fb[n][channel];
        // Output of delay lines
        if (channel == 0)
        {
            outDL[n] = lfoDelayL[n]->processSample (inDL[n], channel);
        }
        else
        {
            outDL[n] = lfoDelayR[n]->processSample (inDL[n], channel);
        }
        y += outDL[n];
    }

    // Value to combine parallel delay lines
    y *= 1.f / (float) NUMDELAYLINES;

    for (int r = 0; r < NUMDELAYLINES; r++)
    {
        float val = 0.f;
        for (int c = 0; c < NUMDELAYLINES; c++)
        {
            val += outDL[c] * matrix[r][c];
        }
        val *= feedbackGain;
        val = damp[r]->process (val, channel);
        fb[r][channel] =  tanh (0.5f * val); // prevent instability/ringing
    }

    return y;
}

void FDN::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    Fs = static_cast<float> (sampleRate);
    for (int n = 0; n < NUMDELAYLINES; n++)
    {
        lfoDelayL[n]->prepare (Fs);
        lfoDelayR[n]->prepare (Fs);
    }
    setModDepth(modDepth);
}

void FDN::setFeedbackGain (float newFeedbackGain)
{
    if (feedbackGain != newFeedbackGain)
    {
        feedbackGain = newFeedbackGain;
    }
}

void FDN::setModDepth (float newModDepth)
{
    //if (modDepth != newModDepth)
    //{
        modDepth = newModDepth;
        for (int n = 0; n < NUMDELAYLINES; n++)
        {
            lfoDelayL[n]->setModDepth (newModDepth);
            lfoDelayR[n]->setModDepth (newModDepth);
        }
    //}
}

}
