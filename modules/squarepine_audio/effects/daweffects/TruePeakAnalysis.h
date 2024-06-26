//
//  TruePeakAnalysis.h
//
// Based on Recommendation ITU-R BS.1770-4
// Algorithms to measure audio programme loudness and true-peak audio level
// Implements a polyphase filter for 4x oversampling

namespace djdawprocessor
{

class TruePeakAnalysis
{
public:
    // Primary function to use
    void fillTruePeakFrameBuffer (AudioBuffer<float>& inputBuffer, AudioBuffer<float>& truePeakFrameBuffer, const int numChannels, const int numSamples, bool latencyCompensation = true)
    {
        if (numChannels == 2)
        {
            float xL;
            float xR;
            for (int n = 0; n < numSamples; ++n)
            {
                xL = inputBuffer.getWritePointer (0)[n];
                xR = inputBuffer.getWritePointer (1)[n];
                tpAnalysisBufferLeft[tpIndex] = xL;
                tpAnalysisBufferRight[tpIndex] = xR;
                float detectSample;

                detectSample = analyzePhase1 (tpAnalysisBufferLeft, tpIndex);
                detectSample = maxMagnitude (detectSample, analyzePhase2 (tpAnalysisBufferLeft, tpIndex));
                detectSample = maxMagnitude (detectSample, analyzePhase3 (tpAnalysisBufferLeft, tpIndex));
                detectSample = maxMagnitude (detectSample, analyzePhase4 (tpAnalysisBufferLeft, tpIndex));

                detectSample = maxMagnitude (detectSample, analyzePhase1 (tpAnalysisBufferRight, tpIndex));
                detectSample = maxMagnitude (detectSample, analyzePhase2 (tpAnalysisBufferRight, tpIndex));
                detectSample = maxMagnitude (detectSample, analyzePhase3 (tpAnalysisBufferRight, tpIndex));
                detectSample = maxMagnitude (detectSample, analyzePhase4 (tpAnalysisBufferRight, tpIndex));

                truePeakFrameBuffer.getWritePointer (0)[n] = detectSample;

                tpIndex++;
                if (tpIndex >= FILTER_ORDER)
                    tpIndex = 0;

                if (latencyCompensation)
                {
                    inputBuffer.getWritePointer (0)[n] = latencyBuffer[latIndex][0];
                    inputBuffer.getWritePointer (1)[n] = latencyBuffer[latIndex][1];
                    latencyBuffer[latIndex][0] = xL;
                    latencyBuffer[latIndex][1] = xR;

                    latIndex++;
                    if (latIndex >= LATENCY)
                        latIndex = 0;
                }
            }
        }
        else
        {
            // MONO
            float x;
            float detectSample;

            for (int n = 0; n < numSamples; ++n)
            {
                x = inputBuffer.getWritePointer (0)[n];

                tpAnalysisBufferLeft[tpIndex] = x;

                detectSample = analyzePhase1 (tpAnalysisBufferLeft, tpIndex);
                detectSample = maxMagnitude (detectSample, analyzePhase2 (tpAnalysisBufferLeft, tpIndex));
                detectSample = maxMagnitude (detectSample, analyzePhase3 (tpAnalysisBufferLeft, tpIndex));
                detectSample = maxMagnitude (detectSample, analyzePhase4 (tpAnalysisBufferLeft, tpIndex));

                truePeakFrameBuffer.getWritePointer (0)[n] = detectSample;

                tpIndex++;
                if (tpIndex >= FILTER_ORDER)
                    tpIndex = 0;

                if (latencyCompensation)
                {
                    inputBuffer.getWritePointer (0)[n] = latencyBuffer[latIndex][0];
                    latencyBuffer[latIndex][0] = x;

                    latIndex++;
                    if (latIndex >= LATENCY)
                        latIndex = 0;
                }
            }
        }
    }

    void reset()
    {
        for (int n = 0; n < FILTER_ORDER; ++n)
        {
            tpAnalysisBufferLeft[n] = 0.f;
            tpAnalysisBufferRight[n] = 0.f;
        }
    }
private:
    enum FILTER_PHASE
    {
        PHASE1,
        PHASE2,
        PHASE3,
        PHASE4
    };

    static const int FILTER_ORDER = 12;
    float tpAnalysisBufferLeft[FILTER_ORDER] = { 0.f };
    float tpAnalysisBufferRight[FILTER_ORDER] = { 0.f };
    int tpIndex = 0;
    static const int LATENCY = 6;// True Peak Analysis introduces 6 samples of latency from filtering
    float latencyBuffer[LATENCY][2] = { { 0.f } };
    int latIndex = 0;

    // Functions for analyzing
    float analyzePhase1 (float* buffer, int circIndex)
    {
        return filter (buffer, circIndex, FILTER_PHASE::PHASE1);
    }
    float analyzePhase2 (float* buffer, int circIndex)
    {
        return filter (buffer, circIndex, FILTER_PHASE::PHASE2);
    }
    float analyzePhase3 (float* buffer, int circIndex)
    {
        return filter (buffer, circIndex, FILTER_PHASE::PHASE3);
    }
    float analyzePhase4 (float* buffer, int circIndex)
    {
        return filter (buffer, circIndex, FILTER_PHASE::PHASE4);
    }

    float filter (float* buffer, int circIndex, FILTER_PHASE num)
    {
        float y = 0;
        if (num == FILTER_PHASE::PHASE1)
        {
            for (int n = 0; n < FILTER_ORDER; ++n)
            {
                y += buffer[circIndex] * phase1[n];
                circIndex--;
                if (circIndex < 0)
                {
                    circIndex = FILTER_ORDER - 1;
                }
            }
        }
        if (num == FILTER_PHASE::PHASE2)
        {
            for (int n = 0; n < FILTER_ORDER; ++n)
            {
                y += buffer[circIndex] * phase2[n];
                circIndex--;
                if (circIndex < 0)
                {
                    circIndex = FILTER_ORDER - 1;
                }
            }
        }
        if (num == FILTER_PHASE::PHASE3)
        {
            for (int n = 0; n < FILTER_ORDER; ++n)
            {
                y += buffer[circIndex] * phase3[n];
                circIndex--;
                if (circIndex < 0)
                {
                    circIndex = FILTER_ORDER - 1;
                }
            }
        }
        if (num == FILTER_PHASE::PHASE4)
        {
            for (int n = 0; n < FILTER_ORDER; ++n)
            {
                y += buffer[circIndex] * phase4[n];
                circIndex--;
                if (circIndex < 0)
                {
                    circIndex = FILTER_ORDER - 1;
                }
            }
        }
        return y;
    }

    float maxMagnitude (float x, float y)
    {
        if (std::abs (x) > std::abs (y))
        {
            return x;
        }
        return y;
    }
    float phase1[FILTER_ORDER] = { -0.0083007812500,
                                   0.0148925781250,
                                   -0.0266113281250,
                                   0.0476074218750,
                                   -0.1022949218750,
                                   0.9721679687500,
                                   0.1373291015625,
                                   -0.0594482421875,
                                   0.0332031250000,
                                   -0.0196533203125,
                                   0.0109863281250,
                                   0.0017089843750 };

    float phase2[FILTER_ORDER] = { -0.0189208984375,
                                   0.0330810546875,
                                   -0.0582275390625,
                                   0.1015625000000,
                                   -0.2003173828125,
                                   0.7797851562500,
                                   0.4650878906250,
                                   -0.1665039062500,
                                   0.0891113281250,
                                   -0.0517578125000,
                                   0.0292968750000,
                                   -0.0291748046875 };

    float phase3[FILTER_ORDER] = { -0.0291748046875,
                                   0.0292968750000,
                                   -0.0517578125000,
                                   0.0891113281250,
                                   -0.1665039062500,
                                   0.4650878906250,
                                   0.7797851562500,
                                   -0.2003173828125,
                                   0.1015625000000,
                                   -0.0582275390625,
                                   0.0330810546875,
                                   -0.0189208984375 };

    float phase4[FILTER_ORDER] = { 0.0017089843750,
                                   0.0109863281250,
                                   -0.0196533203125,
                                   0.0332031250000,
                                   -0.0594482421875,
                                   0.1373291015625,
                                   0.9721679687500,
                                   -0.1022949218750,
                                   0.0476074218750,
                                   -0.0266113281250,
                                   0.0148925781250,
                                   -0.0083007812500 };
};

}
