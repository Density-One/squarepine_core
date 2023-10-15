/*
 =============================================================================
 
 FDN.
 Created: 18 May 2021 1:17:02a
 Author:  Eric Tarr
 
 =============================================================================
 */

//#include <cmath>
//using namespace std;
namespace djdawprocessor
{

class FDN
{
public:
    FDN();

    ~FDN();

    float processSample (float x, int channel);

    void prepareToPlay (double sampleRate, int samplesPerBlock);

    void setFeedbackGain (float newFeedbackGain);

    void setModDepth (float newModDepth);
private:
    float Fs = 48000.f;

    float feedbackGain = 0.f;

    static const int NUMDELAYLINES = 16;

    LFODelay* lfoDelayL[NUMDELAYLINES];
    LFODelay* lfoDelayR[NUMDELAYLINES];
    float modDepth = 0.01f;
    float fb[NUMDELAYLINES][2] = { { 0.f } };

//    // 8x8 Options
//    float delayMsL[NUMDELAYLINES] = { 233.f, 181.f, 131.f, 83.f, 31.f, 19.f, 11.f, 5.f };
//    float delayMsR[NUMDELAYLINES] = { 229.f, 191.f, 139.f, 89.f, 37.f, 23.f, 13.f, 7.f };
//    float modRate[NUMDELAYLINES] =  { 0.2f, 0.3f, 0.5f, 0.7f, 0.8f, 1.1f, 1.3f, 1.7f };
    
    // Hadamard Matrix 8x8
    // | A A |
    // |-A A |
//    float matrix[NUMDELAYLINES][NUMDELAYLINES] = {
//        { 00.f, 01.f, 01.f, 01.f, 01.f, 01.f, 01.f, 01.f},
//        { -1.f, 00.f, -1.f, 01.f, -1.f, 01.f, -1.f, 01.f},
//        { -1.f, -1.f, 00.f, 01.f, -1.f, -1.f, 01.f, 01.f},
//        { 01.f, -1.f, -1.f, 00.f, 01.f, -1.f, -1.f, 01.f},
//        { -1.f, -1.f, -1.f, -1.f, 00.f, 01.f, 01.f, 01.f},
//        { 01.f, -1.f, 01.f, -1.f, -1.f, 00.f, -1.f, 01.f},
//        { 01.f, 01.f, -1.f, -1.f, -1.f, -1.f, 00.f, 01.f},
//        { -1.f, 01.f, 01.f, -1.f, 01.f, -1.f, -1.f, 00.f},
//    };
    
    // Householder Matrix 8x8
    // | A A |
    // | A A |
//    float matrix[NUMDELAYLINES][NUMDELAYLINES] = {
//        { 00.f, -1.f, -1.f, -1.f, 01.f, -1.f, -1.f, -1.f},
//        { -1.f, 00.f, -1.f, -1.f, -1.f, 01.f, -1.f, -1.f},
//        { -1.f, -1.f, 00.f, -1.f, -1.f, -1.f, 01.f, -1.f},
//        { -1.f, -1.f, -1.f, 00.f, -1.f, -1.f, -1.f, 01.f},
//        { 01.f, -1.f, -1.f, -1.f, 00.f, -1.f, -1.f, -1.f},
//        { -1.f, 01.f, -1.f, -1.f, -1.f, 00.f, -1.f, -1.f},
//        { -1.f, -1.f, 01.f, -1.f, -1.f, -1.f, 00.f, -1.f},
//        { -1.f, -1.f, -1.f, 01.f, -1.f, -1.f, -1.f, 00.f},
//    };
    // Stautner Puckett Matrix 8x8
    // | A -A |
    // | A  A |
//    float matrix[NUMDELAYLINES][NUMDELAYLINES] = {
//        { 00.f, 01.f, 01.f, 00.f, 00.f, -1.f, -1.f, 00.f},
//        { -1.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, 01.f},
//        { 01.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, 01.f},
//        { 00.f, 01.f, -1.f, 00.f, 00.f, -1.f, 01.f, 00.f},
//        { 00.f, 01.f, 01.f, 00.f, 00.f, 01.f, 01.f, 00.f},
//        { -1.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, -1.f},
//        { 01.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, -1.f},
//        { 00.f, 01.f, -1.f, 00.f, 00.f, 01.f, -1.f, 00.f},
//    };
    
    // 16x16 Options
    float delayMsL[NUMDELAYLINES] = { 233.f, 181.f, 131.f, 83.f, 51.f, 31.f, 19.f, 11.f,
                                      187.f, 141.f, 67.f, 112.f, 99.f, 133.f, 68.f, 111.f};
    
    float delayMsR[NUMDELAYLINES] = { 229.f, 191.f, 139.f, 89.f, 73.f, 37.f, 23.f, 13.f,
                                     178.f, 91.f, 76.f, 212.f, 116.f, 68.f, 40.f, 93.f};
    
    float modRate[NUMDELAYLINES] = { 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f,
                                     1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f };
    // Stautner Puckett Matrix 16x16
    // | A -A A -A |
    // | A  A A  A |
    // | A -A A -A |
    // | A  A A  A |
//    float matrix[NUMDELAYLINES][NUMDELAYLINES] = {
//        { 00.f, 01.f, 01.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, 01.f, 01.f, 00.f, 00.f, -1.f, -1.f, 00.f},
//        { -1.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, 01.f, -1.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, 01.f},
//        { 01.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, 01.f, 01.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, 01.f},
//        { 00.f, 01.f, -1.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, 01.f, -1.f, 00.f, 00.f, -1.f, 01.f, 00.f},
//        { 00.f, 01.f, 01.f, 00.f, 00.f, 01.f, 01.f, 00.f, 00.f, 01.f, 01.f, 00.f, 00.f, 01.f, 01.f, 00.f},
//        { -1.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, -1.f},
//        { 01.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, -1.f},
//        { 00.f, 01.f, -1.f, 00.f, 00.f, 01.f, -1.f, 00.f, 00.f, 01.f, -1.f, 00.f, 00.f, 01.f, -1.f, 00.f},
//        { 00.f, 01.f, 01.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, 01.f, 01.f, 00.f, 00.f, -1.f, -1.f, 00.f},
//        { -1.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, 01.f, -1.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, 01.f},
//        { 01.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, 01.f, 01.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, 01.f},
//        { 00.f, 01.f, -1.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, 01.f, -1.f, 00.f, 00.f, -1.f, 01.f, 00.f},
//        { 00.f, 01.f, 01.f, 00.f, 00.f, 01.f, 01.f, 00.f, 00.f, 01.f, 01.f, 00.f, 00.f, 01.f, 01.f, 00.f},
//        { -1.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, -1.f, -1.f, 00.f, 00.f, -1.f},
//        { 01.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, -1.f, 01.f, 00.f, 00.f, -1.f},
//        { 00.f, 01.f, -1.f, 00.f, 00.f, 01.f, -1.f, 00.f, 00.f, 01.f, -1.f, 00.f, 00.f, 01.f, -1.f, 00.f},
//    };
        
    // Hadamard Matrix 16x16
    // | A A  A A|
    // |-A A -A A|
    float matrix[NUMDELAYLINES][NUMDELAYLINES] = {
        { 00.f, 01.f, 01.f, 01.f, 01.f, 01.f, 01.f, 01.f , 00.f, 01.f, 01.f, 01.f, 01.f, 01.f, 01.f, 01.f},
        { -1.f, 00.f, -1.f, 01.f, -1.f, 01.f, -1.f, 01.f , -1.f, 00.f, -1.f, 01.f, -1.f, 01.f, -1.f, 01.f},
        { -1.f, -1.f, 00.f, 01.f, -1.f, -1.f, 01.f, 01.f , -1.f, -1.f, 00.f, 01.f, -1.f, -1.f, 01.f, 01.f},
        { 01.f, -1.f, -1.f, 00.f, 01.f, -1.f, -1.f, 01.f , 01.f, -1.f, -1.f, 00.f, 01.f, -1.f, -1.f, 01.f},
        { -1.f, -1.f, -1.f, -1.f, 00.f, 01.f, 01.f, 01.f , -1.f, -1.f, -1.f, -1.f, 00.f, 01.f, 01.f, 01.f},
        { 01.f, -1.f, 01.f, -1.f, -1.f, 00.f, -1.f, 01.f , 01.f, -1.f, 01.f, -1.f, -1.f, 00.f, -1.f, 01.f},
        { 01.f, 01.f, -1.f, -1.f, -1.f, -1.f, 00.f, 01.f , 01.f, 01.f, -1.f, -1.f, -1.f, -1.f, 00.f, 01.f},
        { -1.f, 01.f, 01.f, -1.f, 01.f, -1.f, -1.f, 00.f , -1.f, 01.f, 01.f, -1.f, 01.f, -1.f, -1.f, 00.f},
        { 00.f, 01.f, 01.f, 01.f, 01.f, 01.f, 01.f, 01.f , 00.f, 01.f, 01.f, 01.f, 01.f, 01.f, 01.f, 01.f},
        { -1.f, 00.f, -1.f, 01.f, -1.f, 01.f, -1.f, 01.f , -1.f, 00.f, -1.f, 01.f, -1.f, 01.f, -1.f, 01.f},
        { -1.f, -1.f, 00.f, 01.f, -1.f, -1.f, 01.f, 01.f , -1.f, -1.f, 00.f, 01.f, -1.f, -1.f, 01.f, 01.f},
        { 01.f, -1.f, -1.f, 00.f, 01.f, -1.f, -1.f, 01.f , 01.f, -1.f, -1.f, 00.f, 01.f, -1.f, -1.f, 01.f},
        { -1.f, -1.f, -1.f, -1.f, 00.f, 01.f, 01.f, 01.f , -1.f, -1.f, -1.f, -1.f, 00.f, 01.f, 01.f, 01.f},
        { 01.f, -1.f, 01.f, -1.f, -1.f, 00.f, -1.f, 01.f , 01.f, -1.f, 01.f, -1.f, -1.f, 00.f, -1.f, 01.f},
        { 01.f, 01.f, -1.f, -1.f, -1.f, -1.f, 00.f, 01.f , 01.f, 01.f, -1.f, -1.f, -1.f, -1.f, 00.f, 01.f},
        { -1.f, 01.f, 01.f, -1.f, 01.f, -1.f, -1.f, 00.f , -1.f, 01.f, 01.f, -1.f, 01.f, -1.f, -1.f, 00.f},
    };
    
    class DampeningFilter
    {
    public:
        DampeningFilter (float dampeningAmount)
        {
            // dampeningAmount = [0-1]
            // 0 = no dampening
            // 1 = max dampening
            b1 = 0.5f * dampeningAmount;
            b0 = 1.f - b1;
        }
        
        float process (float x, int c)
        {
            float y = b0 * x + b1 * z[c];
            z[c] = x;
            return y;
        }
    
    private:
        
        float b0 = 1.f;
        float b1 = 0.f;
        
        float z[2] = {0.f};
    };
    
    const float DAMPAMOUNT = 1.f; // [0-1]
    DampeningFilter* damp[NUMDELAYLINES];
};

}
