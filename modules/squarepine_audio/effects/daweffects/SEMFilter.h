// This model is based on the Virtual Analog analysis
// by Will Pirkle in Ch 7 of Designing Synthesizer Software textbook

namespace djdawprocessor
{

class SEMLowPassFilter
{
public:
    void prepareToPlay (double Fs, int)
    {
        sampleRate = (float) Fs;
        normFreqSmooth.reset (sampleRate, 0.0001f);
        resSmooth.reset (sampleRate, 0.0001f);
        updateCoefficients();
    }

    void setNormFreq (float newNormFreq)
    {
        if (targetFreq != newNormFreq)
        {
            targetFreq = newNormFreq;
            normFreqSmooth.setTargetValue (targetFreq);
            updateCoefficients();
        }
    }

    // Allowable range from 0.01f to ~10
    void setQValue (float q)
    {
        if (targetRes != q)
        {
            targetRes = jlimit (0.01f, 10.f, q);
            resSmooth.setTargetValue (targetRes);
            updateCoefficients();
        }
    }

    float processSample (float x, int channel)
    {
        resSmooth.getNextValue();
        normFreqSmooth.getNextValue();
        // Filter Prep
        float yhp = (x - rho * s1[channel] - s2[channel]) * alpha0;

        float x1 = alpha1 * yhp;
        float y1 = s1[channel] + x1;
        float ybp = std::tanh (1.f * y1);
        s1[channel] = x1 + ybp;

        float x2 = alpha1 * ybp;
        float ylp = s2[channel] + x2;
        s2[channel] = x2 + ylp;

        return ylp;
    }
private:
    SmoothedValue<float, ValueSmoothingTypes::Linear> normFreqSmooth { 1.0f };
    SmoothedValue<float, ValueSmoothingTypes::Linear> resSmooth { 0.7071f };

    float targetFreq = 0.0f;
    float targetRes = 0.1f;

    float sampleRate;
    float R;// transformed bandwidth "Q"
    float g;
    float G;
    float alpha;
    float alpha0;
    float alpha1;
    float rho;
    float s1[2] = { 0.0f };
    float s2[2] = { 0.0f };
public:
    void updateCoefficients()
    {
        R = 1.f / (2.f * resSmooth.getNextValue());
        float normFreq = normFreqSmooth.getNextValue();

        float freqHz = 2.f * std::powf (10.f, 3.f * normFreq + 1.f);
        freqHz = jmin (freqHz, (sampleRate / 2.f) * 0.95f);
        float wd = 2.f * (float) M_PI * freqHz;
        float T = 1.f / sampleRate;
        float wa = (2.f / T) * std::tan (wd * T / 2.f);// Warping for BLT
        g = wa * T / 2.f;
        G = g / (1.f + g);
        alpha = G;
        alpha0 = 1.f / (1.f + 2.f * R * g + g * g);
        alpha1 = g;
        rho = 2.f * R + g;
    }
};

class SEMHighPassFilter
{
public:
    void prepareToPlay (double Fs, int)
    {
        sampleRate = (float) Fs;
        normFreqSmooth.reset (sampleRate, 0.0001);
        resSmooth.reset (sampleRate, 0.0001);
        updateCoefficients();
    }

    void setNormFreq (float newNormFreq)
    {
        if (targetFreq != newNormFreq)
        {
            targetFreq = newNormFreq;
            normFreqSmooth.setTargetValue (targetFreq);
            updateCoefficients();
        }
    }

    // Allowable range from 0.01f to ~10
    void setQValue (float q)
    {
        if (targetRes != q)
        {
            targetRes = jlimit (0.01f, 10.f, q);

            resSmooth.setTargetValue (targetRes);
            updateCoefficients();
        }
    }

    float processSample (float x, int channel)
    {
        resSmooth.getNextValue();
        normFreqSmooth.getNextValue();

        resSmooth.getNextValue();
        normFreqSmooth.getNextValue();
        // Filter Prep
        float yhp = (x - rho * s1[channel] - s2[channel]) * alpha0;

        float x1 = alpha1 * yhp;
        float y1 = s1[channel] + x1;
        float ybp = std::tanh (1.f * y1);
        s1[channel] = x1 + ybp;

        float x2 = alpha1 * ybp;
        float ylp = s2[channel] + x2;
        s2[channel] = x2 + ylp;

        return yhp;
    }
private:
    SmoothedValue<float, ValueSmoothingTypes::Linear> normFreqSmooth { 0.0f };
    SmoothedValue<float, ValueSmoothingTypes::Linear> resSmooth { 0.7071f };

    float targetFreq = 0.0f;
    float targetRes = 0.1f;

    float sampleRate;
    float R;// transformed bandwidth "Q"
    float g;
    float G;
    float alpha;
    float alpha0;
    float alpha1;
    float rho;
    float s1[2] = { 0.0f };
    float s2[2] = { 0.0f };
public:
    void updateCoefficients()
    {
        R = 1.f / (2.f * resSmooth.getNextValue());
        float normFreq = normFreqSmooth.getNextValue();

        float freqHz = 2.f * std::powf (10.f, 3.f * normFreq + 1.f);
        freqHz = jmin (freqHz, (sampleRate / 2.f) * 0.95f);
        float wd = 2.f * (float) M_PI * freqHz;
        float T = 1.f / sampleRate;
        float wa = (2.f / T) * std::tan (wd * T / 2.f);// Warping for BLT
        g = wa * T / 2.f;
        G = g / (1.f + g);
        alpha = G;
        alpha0 = 1.f / (1.f + 2.f * R * g + g * g);
        alpha1 = g;
        rho = 2.f * R + g;
    }
};

// Added this class temporarily for the HPF. Eventually we will want to use a better model of the SEM HPF
class DigitalFilter
{
public:
    enum FilterType
    {
        LPF,
        HPF,
        BPF1,
        BPF2,
        NOTCH,
        LSHELF,
        HSHELF,
        PEAK,
        APF
    };

    void setFilterType (FilterType filterTypeParam)
    {
        this->filterType = filterTypeParam;
    }

    void processBuffer (juce::AudioBuffer<float>& buffer, MidiBuffer&)
    {
        for (int c = 0; c < buffer.getNumChannels(); ++c)
        {
            for (int n = 0; n < buffer.getNumSamples(); ++n)
            {
                float x = buffer.getWritePointer (c)[n];
                float y = processSample (x, c);
                buffer.getWritePointer (c)[n] = y;
            }
        }
    }

    void processToOutputBuffer (juce::AudioBuffer<float>& inBuffer, juce::AudioBuffer<float>& outBuffer)
    {
        for (int c = 0; c < inBuffer.getNumChannels(); ++c)
        {
            for (int n = 0; n < inBuffer.getNumSamples(); ++n)
            {
                float x = inBuffer.getWritePointer (c)[n];
                float y = processSample (x, c);
                outBuffer.getWritePointer (c)[n] = y;
            }
        }
    }

    float processSample (float x, int channel)
    {
        performSmoothing();

        // Output, processed sample (Direct Form 1)
        float y = b0 * x + b1 * x1[channel] + b2 * x2[channel]
                  + (-a1) * y1[channel] + (-a2) * y2[channel];

        x2[channel] = x1[channel];// store delay samples for next process step
        x1[channel] = x;
        y2[channel] = y1[channel];
        y1[channel] = y;

        return y;
    }
    void setNormFreq (float normFreq)
    {
        float newFreq = 2.f * std::powf (10.f, 3.f * normFreq + 1.f);
        freqTarget = jlimit (20.0f, 20000.0f, newFreq);
        freqTarget = jmin (freqTarget, (Fs / 2.f) * 0.95f);
        updateCoefficients();
    }
    void setFs (double newFs)
    {
        Fs = static_cast<float> (newFs);
        updateCoefficients();// Need to update if Fs changes
    }

    void setFreq (float newFreq)
    {
        freqTarget = jlimit (20.0f, 20000.0f, newFreq);
        freqTarget = jmin (freqTarget, (Fs / 2.f) * 0.95f);
        updateCoefficients();
    }

    void setQValue (float newQ)
    {
        qTarget = jlimit (0.1f, 10.0f, newQ);
        updateCoefficients();
    }

    void setAmpdB (float newAmpdB)
    {
        ampdB = newAmpdB;
    }
private:
    FilterType filterType = LPF;

    float Fs = 48000.0;// Sampling Rate

    // Variables for User to Modify Filter
    float freqTarget = 20.0f;// frequency in Hz
    float freqSmooth = 20.0f;
    float qTarget = 0.7071f;// Q => [0.1 - 10]
    float qSmooth = 0.7071f;
    float ampdB = 0.0f;// Amplitude on dB scale

    // Variables for Biquad Implementation
    // 2 channels - L,R : Up to 2nd Order
    float x1[2] = { 0.0f };// 1 sample of delay feedforward
    float x2[2] = { 0.0f };// 2 samples of delay feedforward
    float y1[2] = { 0.0f };// 1 sample of delay feedback
    float y2[2] = { 0.0f };// 2 samples of delay feedback

    // Filter coefficients
    float b0 = 1.0f;// initialized to pass signal
    float b1 = 0.0f;// without filtering
    float b2 = 0.0f;
    float a0 = 1.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;

    int smoothingCount = 0;
    const int SAMPLESFORSMOOTHING = 256;
    void performSmoothing()
    {
        float alpha = 0.9999f;
        freqSmooth = alpha * freqSmooth + (1.f - alpha) * freqTarget;
        qSmooth = alpha * qSmooth + (1.f - alpha) * qTarget;

        smoothingCount++;
        if (smoothingCount >= SAMPLESFORSMOOTHING)
        {
            updateCoefficients();
            smoothingCount = 0;
        }
    }
public:
    void updateCoefficients()
    {
        float A = std::pow (10.0f, ampdB / 40.0f);// Linear amplitude

        // Normalize frequency
        float f_PI = static_cast<float> (M_PI);
        float w0 = (2.0f * f_PI) * freqSmooth / Fs;

        // Bandwidth/slope/resonance parameter
        float alpha = std::sin (w0) / (2.0f * qSmooth);

        float cw0 = std::cos (w0);
        switch (filterType)
        {
            case LPF:
            {
                a0 = 1.0f + alpha;
                float B0 = (1.0f - cw0) / 2.0f;
                b0 = B0 / a0;
                float B1 = 1.0f - cw0;
                b1 = B1 / a0;
                float B2 = (1.0f - cw0) / 2.0f;
                b2 = B2 / a0;
                float A1 = -2.0f * cw0;
                a1 = A1 / a0;
                float A2 = 1.0f - alpha;
                a2 = A2 / a0;
                break;
            }
            case HPF:
            {
                a0 = 1.0f + alpha;
                float B0 = (1.0f + cw0) / 2.0f;
                b0 = B0 / a0;
                float B1 = -(1.0f + cw0);
                b1 = B1 / a0;
                float B2 = (1.0f + cw0) / 2.0f;
                b2 = B2 / a0;
                float A1 = -2.0f * cw0;
                a1 = A1 / a0;
                float A2 = 1.0f - alpha;
                a2 = A2 / a0;
                break;
            }
            case BPF1:
            {
                float sw0 = std::sin (w0);
                a0 = 1.0f + alpha;
                float B0 = sw0 / 2.0f;
                b0 = B0 / a0;
                float B1 = 0.0f;
                b1 = B1 / a0;
                float B2 = -sw0 / 2.0f;
                b2 = B2 / a0;
                float A1 = -2.0f * cw0;
                a1 = A1 / a0;
                float A2 = 1.0f - alpha;
                a2 = A2 / a0;
                break;
            }
            case BPF2:
            {
                a0 = 1.0f + alpha;
                float B0 = alpha;
                b0 = B0 / a0;
                float B1 = 0.0f;
                b1 = B1 / a0;
                float B2 = -alpha;
                b2 = B2 / a0;
                float A1 = -2.0f * cw0;
                a1 = A1 / a0;
                float A2 = 1.0f - alpha;
                a2 = A2 / a0;

                break;
            }
            case NOTCH:
            {
                a0 = 1.0f + alpha;
                float B0 = 1.0f;
                b0 = B0 / a0;
                float B1 = -2.0f * cw0;
                b1 = B1 / a0;
                float B2 = 1.0f;
                b2 = B2 / a0;
                float A1 = -2.0f * cw0;
                a1 = A1 / a0;
                float A2 = 1.0f - alpha;
                a2 = A2 / a0;
                break;
            }
            case LSHELF:
            {
                float sqA = std::sqrt (A);
                a0 = (A + 1.0f) + (A - 1.0f) * cw0 + 2.0f * sqA * alpha;
                float B0 = A * ((A + 1.0f) - (A - 1.0f) * cw0 + 2.0f * sqA * alpha);
                b0 = B0 / a0;
                float B1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cw0);
                b1 = B1 / a0;
                float B2 = A * ((A + 1.0f) - (A - 1.0f) * cw0 - 2.0f * sqA * alpha);
                b2 = B2 / a0;
                float A1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cw0);
                a1 = A1 / a0;
                float A2 = (A + 1.0f) + (A - 1.0f) * cw0 - 2.0f * sqA * alpha;
                a2 = A2 / a0;

                break;
            }

            case HSHELF:
            {
                float sqA = std::sqrt (A);
                a0 = (A + 1.0f) - (A - 1.0f) * cw0 + 2.0f * sqA * alpha;
                float B0 = A * ((A + 1.0f) + (A - 1.0f) * cw0 + 2.0f * sqA * alpha);
                b0 = B0 / a0;
                float B1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cw0);
                b1 = B1 / a0;
                float B2 = A * ((A + 1.0f) + (A - 1.0f) * cw0 - 2.0f * sqA * alpha);
                b2 = B2 / a0;
                float A1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cw0);
                a1 = A1 / a0;
                float A2 = (A + 1.0f) - (A - 1.0f) * cw0 - 2.0f * sqA * alpha;
                a2 = A2 / a0;

                break;
            }

            case PEAK:

            {
                a0 = 1.0f + alpha / A;
                float B0 = 1.0f + alpha * A;
                b0 = B0 / a0;
                float B1 = -2.0f * cw0;
                b1 = B1 / a0;
                float B2 = 1.0f - alpha * A;
                b2 = B2 / a0;
                float A1 = -2.0f * cw0;
                a1 = A1 / a0;
                float A2 = 1.0f - alpha / A;
                a2 = A2 / a0;

                break;
            }

            case APF:
            {
                a0 = 1.0f + alpha;
                float B0 = 1.0f - alpha;
                b0 = B0 / a0;
                float B1 = -2.0f * cw0;
                b1 = B1 / a0;
                float B2 = 1.0f + alpha;
                b2 = B2 / a0;
                float A1 = -2.0f * cw0;
                a1 = A1 / a0;
                float A2 = 1.0f - alpha;
                a2 = A2 / a0;

                break;
            }
        }
    }
};

// The SEMFilter in the DJDAW is a combination of both a LPF and HPF
// By changing the value of the cut-off parameter to be above 0 (halfway),
// the filter becomes a HPF. If the value is below 0, it is a LPF
class SEMFilter final : public InternalProcessor,
                        public AudioProcessorParameter::Listener
{
public:
    // These values represent 100hz and 8000hz respectively
    inline static const NormalisableRange<float> freqRange = { -0.7670f, 0.867365f };

    SEMFilter (int idNum = 1)
        : idNumber (idNum)
    {
        reset();
        auto normFreq = std::make_unique<NotifiableAudioParameterFloat> ("freqSEM", "Frequency", freqRange, 0.0f,
                                                                         true,// isAutomatable
                                                                         "Cut-off",
                                                                         AudioProcessorParameter::genericParameter,
                                                                         [] (float value, int) -> String
                                                                         {
                                                                             if (abs (value - 0.0f) < 0.0001f)
                                                                                 return "BYP";

                                                                             if (value < 0.0f)
                                                                             {
                                                                                 float posFreq = value + 1.f;
                                                                                 float freqHz = 2.f * std::powf (10.f, 3.f * posFreq + 1.f);
                                                                                 // For now, use int for min
                                                                                 if (static_cast<int> (freqHz) == 100)
                                                                                     freqHz = 100;
                                                                                 return String (freqHz, 0);
                                                                             }
                                                                             else
                                                                             {
                                                                                 float freqHz = 2.f * std::powf (10.f, 3.f * value + 1.f);
                                                                                 // For now, use int for max
                                                                                 if (static_cast<int> (freqHz) == 8000)
                                                                                     freqHz = 8000;
                                                                                 return String (freqHz, 0);
                                                                             }
                                                                         });

        NormalisableRange<float> qRange = { 0.0f, 10.f };
        auto res = std::make_unique<NotifiableAudioParameterFloat> ("resSEM", "resonance", qRange, 0.7071f,
                                                                    true,// isAutomatable
                                                                    "Q",
                                                                    AudioProcessorParameter::genericParameter,
                                                                    [] (float value, int) -> String
                                                                    {
                                                                        if (approximatelyEqual (value, 0.1f))
                                                                            return "0.1";

                                                                        if (approximatelyEqual (value, 10.f))
                                                                            return "10";

                                                                        return String (value, 1);
                                                                    });

        hpf.setFilterType (DigitalFilter::FilterType::HPF);

        setPrimaryParameter (normFreqParam);
        normFreqParam = normFreq.get();
        normFreqParam->addListener (this);

        resParam = res.get();
        resParam->addListener (this);

        auto layout = createDefaultParameterLayout (false);
        layout.add (std::move (normFreq));
        layout.add (std::move (res));
        apvts.reset (new AudioProcessorValueTreeState (*this, nullptr, "parameters", std::move (layout)));
    }

    ~SEMFilter() override
    {
        normFreqParam->removeListener (this);
        resParam->removeListener (this);
    }

    void prepareToPlay (double Fs, int bufferSize) override
    {
        const ScopedLock sl (getCallbackLock());
        lpf.prepareToPlay (Fs, bufferSize);
        hpf.setFs (Fs);
        mixLPF.reset (Fs, 0.001f);
        mixHPF.reset (Fs, 0.001f);
        setRateAndBufferSizeDetails (Fs, bufferSize);
    }

    //==============================================================================
    /** @internal */
    const String getName() const override { return TRANS ("SEM Filter"); }
    /** @internal */
    Identifier getIdentifier() const override { return "SEM Filter" + String (idNumber); }
    /** @internal */
    bool supportsDoublePrecisionProcessing() const override { return false; }

    void parameterValueChanged (int paramNum, float value) override
    {
        const ScopedLock sl (getCallbackLock());
        if (paramNum == 1)
        {
            // Frequency change
            lpf.setNormFreq (jmin (1.f, value + 1.f));
            auto hv = jmax (0.0001f, value);
            hpf.setNormFreq (hv);
            // if cutoff is set to bypass mode, switch off both processing
            // TODO: use approximatelyEqual when it's finally fixed
            if (abs (value) < 0.0001f)
            {
                mixLPF.setTargetValue (0.f);
                mixHPF.setTargetValue (0.f);
            }
            else if (value < 0.f)
            {
                mixLPF.setTargetValue (1.f);
                mixHPF.setTargetValue (0.f);
            }
            else if (value > 0.f)
            {
                mixLPF.setTargetValue (0.f);
                mixHPF.setTargetValue (1.f);
            }
            // This should be around 200 hz
            if (value < 0.335f)
            {
                updateHighPassQ (hpQ, jmap (value, 0.001f, 0.335f, 0.001f, 1.f));
            }
        }
        else
        {// Resonance change
            value = jmax (value, 0.01f);
            lpf.setQValue (value);
            updateHighPassQ (value, hpCoeff);
        }
    }

    void updateHighPassQ (float value, float coefficient)
    {
        value = jlimit (0.01f, 10.f, value);
        hpCoeff = jlimit (0.001f, 1.f, coefficient);
        hpQ = jmap (value, 0.01f, 10.f, 0.01f, 10.0f);
        hpf.setQValue (hpQ * hpCoeff);
    }

    void parameterGestureChanged (int, bool) override {}

    void processBlock (juce::AudioBuffer<float>& buffer, MidiBuffer&) override { process (buffer); }
    //void processBlock (juce::AudioBuffer<double>& buffer, MidiBuffer&) override  { process (buffer); }

    void process (juce::AudioBuffer<float>& buffer)
    {
        // If the device is turned off, don't process
        if (isBypassed())
            return;

        // If the current value is 0.0, also bypass
        if (abs (normFreqParam->get()) < 0.0001f)
            return;

        lpf.updateCoefficients();
        hpf.updateCoefficients();

        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        const ScopedLock sl (getCallbackLock());

        float x, y, mix, hpv;
        for (int c = 0; c < numChannels; ++c)
        {
            for (int s = 0; s < numSamples; ++s)
            {
                mix = mixLPF.getNextValue();
                x = buffer.getWritePointer (c)[s];
                y = (1.f - mix) * x + mix * lpf.processSample (x, c);

                mix = mixHPF.getNextValue();
                hpv = (float) hpf.processSample (y, c);
                y = (1.f - mix) * y + mix * hpv;
                buffer.getWritePointer (c)[s] = y;
            }
        }
    }

    void setNormFreq (float newNormFreq)
    {
        normFreqParam->setValueNotifyingHost (newNormFreq);
        lpf.setNormFreq (newNormFreq);
        hpf.setNormFreq (newNormFreq);

        if (newNormFreq < 0.f)
        {
            mixLPF.setTargetValue (1.f);
            mixHPF.setTargetValue (0.f);
        }
        else if (newNormFreq > 0.f)
        {
            mixLPF.setTargetValue (0.f);
            mixHPF.setTargetValue (1.f);
        }
        else
        {
            mixLPF.setTargetValue (0.f);
            mixHPF.setTargetValue (0.f);
        }
    }

    // Allowable range from 0.01f to ~10
    void setQValue (float)
    {
        //        resParam->setValueNotifyingHost (q);
        //        lpf.setQValue (q);
        //        hpf1.setQValue (q);
        //        hpf2.setQValue (q);
    }
    void setID (int idNum)
    {
        idNumber = idNum;
    }
private:
    //==============================================================================
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    NotifiableAudioParameterFloat* normFreqParam = nullptr;
    NotifiableAudioParameterFloat* resParam = nullptr;

    SmoothedValue<float, ValueSmoothingTypes::Linear> mixLPF { 0.0f };
    SmoothedValue<float, ValueSmoothingTypes::Linear> mixHPF { 0.0f };

    int idNumber = 1;

    SEMLowPassFilter lpf;
    DigitalFilter hpf;

    float hpQ = 0.707f;
    float hpCoeff = 1.0f;
};

///-------------------------------------------------------
///-------------------------------------------------------
///-------------------------------------------------------
///-------------------------------------------------------
///-------------------------------------------------------
///-------------------------------------------------------

struct Biquad
{
    double A, w0, alpha, cw, sw, Fs, f0, dBgain, a0, a1, a2, b0, b1, b2;
    double x1, x2, y1, y2;
public:
    typedef std::complex<double> Complex;
    Biquad (double sampleRate)
        : A (0),
          w0 (0),
          alpha (0),
          cw (0),
          sw (0),
          Fs (sampleRate),
          f0 (0),
          dBgain (0),
          a0 (0),
          a1 (0),
          a2 (0),
          b0 (0),
          b1 (0),
          b2 (0),
          x1 (0),
          x2 (0),
          y1 (0),
          y2 (0)
    {
    }

    void set_LPF (double frequency, double Q)
    {
        A = 0;
        f0 = frequency;
        w0 = 2.0 * MathConstants<double>::pi * (f0 / Fs);
        cw = cos (w0);
        sw = sin (w0);
        alpha = sin (w0) / (2.0 * Q);
        b0 = (1.0 - cw) / 2.0;
        b1 = 1.0 - cw;
        b2 = (1.0 - cw) / 2.0;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cw;
        a2 = 1.0 - alpha;
    }
    void set_HPF (double frequency, double Q)
    {
        A = 0;
        f0 = frequency;
        w0 = 2.0 * MathConstants<double>::pi * (f0 / Fs);
        cw = cos (w0);
        sw = sin (w0);
        alpha = sin (w0) / (2.0 * Q);
        b0 = (1.0 + cw) / 2.0;
        b1 = -(1.0 + cw);
        b2 = (1.0 + cw) / 2.0;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cw;
        a2 = 1.0 - alpha;
    }
    void set_BPF (double frequency, double Q)
    {
        A = 0;
        f0 = frequency;
        w0 = 2.0 * MathConstants<double>::pi * (f0 / Fs);
        cw = cos (w0);
        sw = sin (w0);
        alpha = sin (w0) / (2.0 * Q);
        //    b0 = Q*alpha;
        b0 = sw / float (2.0);
        b1 = 0.0;
        b2 = -Q * alpha;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cw;
        a2 = 1.0 - alpha;
    }
    void set_BPF_const_gain (double frequency, double Q)
    {
        A = 0;
        f0 = frequency;
        w0 = 2.0 * MathConstants<double>::pi * (f0 / Fs);
        cw = cos (w0);
        sw = sin (w0);
        alpha = sin (w0) / (2.0 * Q);
        b0 = alpha;
        b1 = 0;
        b2 = -alpha;
        a0 = 1 + alpha;
        a1 = -2 * cw;
        a2 = 1 - alpha;
    }
    void set_BAND_REJECT (double frequency, double Q)
    {
        A = 0;
        f0 = frequency;
        w0 = 2.0 * MathConstants<double>::pi * (f0 / Fs);
        cw = cos (w0);
        sw = sin (w0);
        alpha = sin (w0) / (2.0 * Q);
        b0 = 1;
        b1 = -2 * cw;
        b2 = 1;
        a0 = 1 + alpha;
        a1 = -2 * cw;
        a2 = 1 - alpha;
    }
    void set_ALL (double frequency, double Q)
    {
        A = 0;
        f0 = frequency;
        w0 = 2.0 * MathConstants<double>::pi * (f0 / Fs);
        cw = cos (w0);
        sw = sin (w0);
        alpha = sin (w0) / (2.0 * Q);
        b0 = 1 - alpha;
        b1 = -2 * cw;
        b2 = 1 + alpha;
        a0 = 1 + alpha;
        a1 = -2 * cw;
        a2 = 1 - alpha;
    }
    void set_LOWSHELF (double frequency, double S)
    {
        A = 0;
        f0 = frequency;
        w0 = 2.0 * MathConstants<double>::pi * (f0 / Fs);
        cw = cos (w0);
        sw = sin (w0);
        alpha = sw / 2 * sqrt ((A + 1 / A) * (1 / S - 1) + 2);
        b0 = A * ((A + 1) - (A - 1) * cw + 2 * sqrt (A) * alpha);
        b1 = 2 * A * ((A - 1) - (A + 1) * cw);
        b2 = A * ((A + 1) - (A - 1) * cw - 2 * sqrt (A) * alpha);
        a0 = (A + 1) + (A - 1) * cw + 2 * sqrt (A) * alpha;
        a1 = -2 * ((A - 1) + (A + 1) * cw);
        a2 = (A + 1) + (A - 1) * cw - 2 * sqrt (A) * alpha;
    }
    void set_HIGHSHELF (double frequency, double S)
    {
        A = 0;
        f0 = frequency;
        w0 = 2.0 * MathConstants<double>::pi * (f0 / Fs);
        cw = cos (w0);
        sw = sin (w0);
        alpha = sw / 2 * sqrt ((A + 1 / A) * (1 / S - 1) + 2);
        b0 = A * ((A + 1) + (A - 1) * cw + 2 * sqrt (A) * alpha);
        b1 = -2 * A * ((A - 1) + (A + 1) * cw);
        b2 = A * ((A + 1) + (A - 1) * cw - 2 * sqrt (A) * alpha);
        a0 = (A + 1) - (A - 1) * cw + 2 * sqrt (A) * alpha;
        a1 = 2 * ((A - 1) - (A + 1) * cw);
        a2 = (A + 1) - (A - 1) * cw - 2 * sqrt (A) * alpha;
    }

    void process (const float* in, float* out, int frameSize)
    {
        while (--frameSize >= 0)
        {
            double x0 = *in;
            double y0;
            y0 = (b0 / a0) * x0 + (b1 / a0) * x1 + (b2 / a0) * x2 - (a1 / a0) * y1 - (a2 / a0) * y2;
            x2 = x1;
            x1 = x0;
            y2 = y1;
            y1 = y0;
            *out = y0;
            ++in;
            ++out;
        }
    }
};

class ButterworthHighpassFilter
{
public:
    ButterworthHighpassFilter()
        : sampleRate (48000),
          cutoffFrequency (1),
          qFactor (0.707)// Default values
    {
        updateCoefficients();
    }

    ButterworthHighpassFilter (double sr, double cutoff, double q)
        : sampleRate (sr),
          cutoffFrequency (cutoff),
          qFactor (q)
    {
        updateCoefficients();
    }

    ~ButterworthHighpassFilter() {}

    void setSampleRate (double newSampleRate)
    {
        sampleRate = newSampleRate;
        updateCoefficients();
    }
    void prepareToPlay (double Fs, int)
    {
        sampleRate = (float) Fs;
        normFreqSmooth.reset (sampleRate, 0.0001);
        resSmooth.reset (sampleRate, 0.0001);
        updateCoefficients();
    }
    void setNormFreq (float newNormFreq)
    {
        if (targetFreq != newNormFreq)
        {
            targetFreq = newNormFreq;
            normFreqSmooth.setTargetValue (targetFreq);
            updateCoefficients();
        }
    }

    void setQValue (float q)
    {
        if (targetRes != q)
        {
            targetRes = q;
            resSmooth.setTargetValue (targetRes);
            updateCoefficients();
        }
    }

    float processSample (float input, int channel)
    {
        if (channel == 0)// Left channel
        {
            return processSampleInternal (input, x1[0], x2[0]);
        }
        else if (channel == 1)// Right channel
        {
            return processSampleInternal (input, x1[1], x2[1]);
        }
        else
        {
            // Invalid channel index
            return input;
        }
    }
    void updateCoefficients()
    {
        float normFreq = normFreqSmooth.getNextValue();
        double cutoffFreqHz = normFreq * (sampleRate / 2.0);
        double w0 = 2.0 * MathConstants<double>::pi * cutoffFreqHz / sampleRate;
        double alpha = sin (w0) / (2.0 * resSmooth.getNextValue());
        double cosw0 = cos (w0);

        b0 = (1.0 + cosw0) / 2.0;
        b1 = -(1.0 + cosw0);
        b2 = (1.0 + cosw0) / 2.0;
        a1 = -2.0 * cosw0;
        a2 = 1.0 - alpha;

        // Normalize the coefficients
        double a0_inv = 1.0 / (1.0 + alpha);
        b0 *= a0_inv;
        b1 *= a0_inv;
        b2 *= a0_inv;
        a1 *= a0_inv;
        a2 *= a0_inv;
    }
private:
    double sampleRate;
    double cutoffFrequency;
    double qFactor;
    double a1, a2, b0, b1, b2;
    float x1[2] = { 0.0f, 0.0f }, x2[2] = { 0.0f, 0.0f };
    float targetFreq = 0.0f;
    float targetRes = 0.1f;
    SmoothedValue<float, ValueSmoothingTypes::Linear> normFreqSmooth { 0.0f };
    SmoothedValue<float, ValueSmoothingTypes::Linear> resSmooth { 0.7071f };

    float processSampleInternal (float input, float& x1, float& x2)
    {
        // Direct Form II implementation
        double v0 = input - (x1 * a1) - (x2 * a2);
        double output = (v0 * b0) + (x1 * b1) + (x2 * b2);

        x2 = x1;
        x1 = input;

        return static_cast<float> (output);
    }
};

class ButterSem final : public InternalProcessor,
                        public AudioProcessorParameter::Listener
{
public:
    // These values represent 100hz and 8000hz respectively
    inline static const NormalisableRange<float> freqRange = { -0.7670f, 0.867365f };

    ButterSem (int idNum = 1)
        : idNumber (idNum)
    {
        reset();
        // call back to convert a value to a display string in Hz
        auto stringFromValue = [] (float value, int) -> String
        {
            // TODO: use approximatelyEqual when it's finally fixed
            if (abs (value - 0.0f) < 0.0001f)
                return "BYP";

            if (value < 0.0f)
            {
                // low-pass filter
                float posFreq = value + 1.f;
                float freqHz = 2.f * std::powf (10.f, 3.f * posFreq + 1.f);
                // For now, use int for min
                if (static_cast<int> (freqHz) == 100)
                    freqHz = 100;
                return String (freqHz, 0);
            }
            else
            {
                // hi-pass filter
                float freqHz = 2.f * std::powf (10.f, 3.f * value + 1.f);
                // For now, use int for max
                if (static_cast<int> (freqHz) == 8000)
                    freqHz = 8000;
                return String (freqHz, 0);
            }
        };

        auto normFreq = std::make_unique<NotifiableAudioParameterFloat> ("freqSEM",
                                                                         "Frequency",
                                                                         freqRange,
                                                                         0.0f,
                                                                         true,// isAutomatable
                                                                         "Cut-off",
                                                                         AudioProcessorParameter::genericParameter,
                                                                         stringFromValue);

        NormalisableRange<float> qRange = { 0.0f, 10.f };
        auto res = std::make_unique<NotifiableAudioParameterFloat> ("resSEM", "resonance", qRange, 0.7071f,
                                                                    true,// isAutomatable
                                                                    "Q",
                                                                    AudioProcessorParameter::genericParameter,
                                                                    [] (float value, int) -> String
                                                                    {
                                                                        if (approximatelyEqual (value, 0.1f))
                                                                            return "0.1";

                                                                        if (approximatelyEqual (value, 10.f))
                                                                            return "10";

                                                                        return String (value, 1);
                                                                    });

        setPrimaryParameter (normFreqParam);
        normFreqParam = normFreq.get();
        normFreqParam->addListener (this);

        resParam = res.get();
        resParam->addListener (this);

        auto layout = createDefaultParameterLayout (false);
        layout.add (std::move (normFreq));
        layout.add (std::move (res));
        apvts.reset (new AudioProcessorValueTreeState (*this, nullptr, "parameters", std::move (layout)));
    }

    ~ButterSem() override
    {
        normFreqParam->removeListener (this);
        resParam->removeListener (this);
    }

    void prepareToPlay (double Fs, int bufferSize) override
    {
        const ScopedLock sl (getCallbackLock());
        lpf.prepareToPlay (Fs, bufferSize);
        hpf.prepareToPlay (Fs, bufferSize);
        mixLPF.reset (Fs, 0.001f);
        mixHPF.reset (Fs, 0.001f);
        setRateAndBufferSizeDetails (Fs, bufferSize);
    }

    //==============================================================================
    /** @internal */
    const String getName() const override { return TRANS ("Butter Sem"); }
    /** @internal */
    Identifier getIdentifier() const override { return "Butter Sem" + String (idNumber); }
    /** @internal */
    bool supportsDoublePrecisionProcessing() const override { return false; }

    void parameterValueChanged (int paramNum, float value) override
    {
        const ScopedLock sl (getCallbackLock());
        if (paramNum == 1)
        {
            // Frequency change
            lpf.setNormFreq (jmin (1.f, value + 1.f));
            hpf.setNormFreq (jmax (0.0001f, value));

            // if cutoff is set to bypass mode, switch off both processing
            // TODO: use approximatelyEqual when it's finally fixed
            if (abs (value) < 0.0001f)
            {
                mixLPF.setTargetValue (0.f);
                mixHPF.setTargetValue (0.f);
            }
            else if (value < 0.f)
            {
                mixLPF.setTargetValue (1.f);
                mixHPF.setTargetValue (0.f);
            }
            else if (value > 0.f)
            {
                mixLPF.setTargetValue (0.f);
                mixHPF.setTargetValue (1.f);
            }
        }
        else
        {// Resonance change
            lpf.setQValue (value);
            hpf.setQValue (value);
        }
    }

    void parameterGestureChanged (int, bool) override {}

    void processBlock (juce::AudioBuffer<float>& buffer, MidiBuffer&) override { process (buffer); }
    //void processBlock (juce::AudioBuffer<double>& buffer, MidiBuffer&) override  { process (buffer); }

    void process (juce::AudioBuffer<float>& buffer)
    {
        // If the device is turned off, don't process
        if (isBypassed())
            return;

        // If the current value is 0.0, also bypass
        if (abs (normFreqParam->get()) < 0.0001f)
            return;

        lpf.updateCoefficients();
        hpf.updateCoefficients();

        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        const ScopedLock sl (getCallbackLock());

        float x, y, mix, hpv;
        for (int c = 0; c < numChannels; ++c)
        {
            for (int s = 0; s < numSamples; ++s)
            {
                mix = mixLPF.getNextValue();
                x = buffer.getWritePointer (c)[s];
                y = (1.f - mix) * x + mix * lpf.processSample (x, c);

                mix = mixHPF.getNextValue();
                hpv = (float) hpf.processSample (y, c);
                y = (1.f - mix) * y + mix * hpv;
                buffer.getWritePointer (c)[s] = y;
            }
        }
    }

    void setNormFreq (float newNormFreq)
    {
        normFreqParam->setValueNotifyingHost (newNormFreq);
        lpf.setNormFreq (newNormFreq);
        hpf.setNormFreq (newNormFreq);

        if (newNormFreq < 0.f)
        {
            mixLPF.setTargetValue (1.f);
            mixHPF.setTargetValue (0.f);
        }
        else if (newNormFreq > 0.f)
        {
            mixLPF.setTargetValue (0.f);
            mixHPF.setTargetValue (1.f);
        }
        else
        {
            mixLPF.setTargetValue (0.f);
            mixHPF.setTargetValue (0.f);
        }
    }

    // Allowable range from 0.01f to ~10
    void setQValue (float q)
    {
        resParam->setValueNotifyingHost (q);
        lpf.setQValue (q);
        hpf.setQValue (q);
    }
    void setID (int idNum)
    {
        idNumber = idNum;
    }
private:
    //==============================================================================
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    NotifiableAudioParameterFloat* normFreqParam = nullptr;
    NotifiableAudioParameterFloat* resParam = nullptr;

    SmoothedValue<float, ValueSmoothingTypes::Linear> mixLPF { 0.0f };
    SmoothedValue<float, ValueSmoothingTypes::Linear> mixHPF { 0.0f };

    int idNumber = 1;

    SEMLowPassFilter lpf;
    ButterworthHighpassFilter hpf;
};

class DigitalSem final : public InternalProcessor,
                         public AudioProcessorParameter::Listener
{
public:
    // These values represent 100hz and 8000hz respectively
    inline static const NormalisableRange<float> freqRange = { -0.7670f, 0.867365f };

    DigitalSem (int idNum = 1)
        : idNumber (idNum)
    {
        reset();

        hpf.setFilterType (DigitalFilter::FilterType::HPF);

        // call back to convert a value to a display string in Hz
        auto stringFromValue = [] (float value, int) -> String
        {
            // TODO: use approximatelyEqual when it's finally fixed
            if (abs (value - 0.0f) < 0.0001f)
                return "BYP";

            if (value < 0.0f)
            {
                // low-pass filter
                float posFreq = value + 1.f;
                float freqHz = 2.f * std::powf (10.f, 3.f * posFreq + 1.f);
                // For now, use int for min
                if (static_cast<int> (freqHz) == 100)
                    freqHz = 100;
                return String (freqHz, 0);
            }
            else
            {
                // hi-pass filter
                float freqHz = 2.f * std::powf (10.f, 3.f * value + 1.f);
                // For now, use int for max
                if (static_cast<int> (freqHz) == 8000)
                    freqHz = 8000;
                return String (freqHz, 0);
            }
        };

        auto normFreq = std::make_unique<NotifiableAudioParameterFloat> ("freqSEM",
                                                                         "Frequency",
                                                                         freqRange,
                                                                         0.0f,
                                                                         true,// isAutomatable
                                                                         "Cut-off",
                                                                         AudioProcessorParameter::genericParameter,
                                                                         stringFromValue);

        NormalisableRange<float> qRange = { 0.0f, 10.f };
        auto res = std::make_unique<NotifiableAudioParameterFloat> ("resSEM", "resonance", qRange, 0.7071f,
                                                                    true,// isAutomatable
                                                                    "Q",
                                                                    AudioProcessorParameter::genericParameter,
                                                                    [] (float value, int) -> String
                                                                    {
                                                                        if (approximatelyEqual (value, 0.1f))
                                                                            return "0.1";

                                                                        if (approximatelyEqual (value, 10.f))
                                                                            return "10";

                                                                        return String (value, 1);
                                                                    });

        setPrimaryParameter (normFreqParam);
        normFreqParam = normFreq.get();
        normFreqParam->addListener (this);

        resParam = res.get();
        resParam->addListener (this);

        auto layout = createDefaultParameterLayout (false);
        layout.add (std::move (normFreq));
        layout.add (std::move (res));
        apvts.reset (new AudioProcessorValueTreeState (*this, nullptr, "parameters", std::move (layout)));
    }

    ~DigitalSem() override
    {
        normFreqParam->removeListener (this);
        resParam->removeListener (this);
    }

    void prepareToPlay (double Fs, int bufferSize) override
    {
        const ScopedLock sl (getCallbackLock());
        lpf.prepareToPlay (Fs, bufferSize);
        hpf.setFs (Fs);
        mixLPF.reset (Fs, 0.001f);
        mixHPF.reset (Fs, 0.001f);
        setRateAndBufferSizeDetails (Fs, bufferSize);
    }

    //==============================================================================
    /** @internal */
    const String getName() const override { return TRANS ("Digital Sem"); }
    /** @internal */
    Identifier getIdentifier() const override { return "Digital Sem" + String (idNumber); }
    /** @internal */
    bool supportsDoublePrecisionProcessing() const override { return false; }

    void parameterValueChanged (int paramNum, float value) override
    {
        const ScopedLock sl (getCallbackLock());
        if (paramNum == 1)
        {
            // Frequency change
            lpf.setNormFreq (jmin (1.f, value + 1.f));
            hpf.setNormFreq (jmax (0.0001f, value));

            // if cutoff is set to bypass mode, switch off both processing
            // TODO: use approximatelyEqual when it's finally fixed
            if (abs (value) < 0.0001f)
            {
                mixLPF.setTargetValue (0.f);
                mixHPF.setTargetValue (0.f);
            }
            else if (value < 0.f)
            {
                mixLPF.setTargetValue (1.f);
                mixHPF.setTargetValue (0.f);
            }
            else if (value > 0.f)
            {
                mixLPF.setTargetValue (0.f);
                mixHPF.setTargetValue (1.f);
            }
        }
        else
        {// Resonance change
            lpf.setQValue (value);
            hpf.setQValue (value);
        }
    }

    void parameterGestureChanged (int, bool) override {}

    void processBlock (juce::AudioBuffer<float>& buffer, MidiBuffer&) override { process (buffer); }
    //void processBlock (juce::AudioBuffer<double>& buffer, MidiBuffer&) override  { process (buffer); }

    void process (juce::AudioBuffer<float>& buffer)
    {
        // If the device is turned off, don't process
        if (isBypassed())
            return;

        // If the current value is 0.0, also bypass
        if (abs (normFreqParam->get()) < 0.0001f)
            return;

        lpf.updateCoefficients();
        hpf.updateCoefficients();

        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        const ScopedLock sl (getCallbackLock());

        float x, y, mix, hpv;
        for (int c = 0; c < numChannels; ++c)
        {
            for (int s = 0; s < numSamples; ++s)
            {
                mix = mixLPF.getNextValue();
                x = buffer.getWritePointer (c)[s];
                y = (1.f - mix) * x + mix * lpf.processSample (x, c);

                mix = mixHPF.getNextValue();
                hpv = (float) hpf.processSample (y, c);
                y = (1.f - mix) * y + mix * hpv;
                buffer.getWritePointer (c)[s] = y;
            }
        }
    }

    void setNormFreq (float newNormFreq)
    {
        normFreqParam->setValueNotifyingHost (newNormFreq);
        lpf.setNormFreq (newNormFreq);
        hpf.setNormFreq (newNormFreq);

        if (newNormFreq < 0.f)
        {
            mixLPF.setTargetValue (1.f);
            mixHPF.setTargetValue (0.f);
        }
        else if (newNormFreq > 0.f)
        {
            mixLPF.setTargetValue (0.f);
            mixHPF.setTargetValue (1.f);
        }
        else
        {
            mixLPF.setTargetValue (0.f);
            mixHPF.setTargetValue (0.f);
        }
    }

    // Allowable range from 0.01f to ~10
    void setQValue (float q)
    {
        resParam->setValueNotifyingHost (q);
        lpf.setQValue (q);
        hpf.setQValue (q);
    }
    void setID (int idNum)
    {
        idNumber = idNum;
    }
private:
    //==============================================================================
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    NotifiableAudioParameterFloat* normFreqParam = nullptr;
    NotifiableAudioParameterFloat* resParam = nullptr;

    SmoothedValue<float, ValueSmoothingTypes::Linear> mixLPF { 0.0f };
    SmoothedValue<float, ValueSmoothingTypes::Linear> mixHPF { 0.0f };

    int idNumber = 1;

    SEMLowPassFilter lpf;
    DigitalFilter hpf;
};
class DigitalSem2 final : public InternalProcessor,
                          public AudioProcessorParameter::Listener
{
public:
    // These values represent 100hz and 8000hz respectively
    inline static const NormalisableRange<float> freqRange = { -0.7670f, 0.867365f };

    DigitalSem2 (int idNum = 1)
        : idNumber (idNum)
    {
        reset();

        hpf.setFilterType (DigitalFilter::FilterType::HPF);
        lpf.setFilterType (DigitalFilter::FilterType::LPF);

        // call back to convert a value to a display string in Hz
        auto stringFromValue = [] (float value, int) -> String
        {
            // TODO: use approximatelyEqual when it's finally fixed
            if (abs (value - 0.0f) < 0.0001f)
                return "BYP";

            if (value < 0.0f)
            {
                // low-pass filter
                float posFreq = value + 1.f;
                float freqHz = 2.f * std::powf (10.f, 3.f * posFreq + 1.f);
                // For now, use int for min
                if (static_cast<int> (freqHz) == 100)
                    freqHz = 100;
                return String (freqHz, 0);
            }
            else
            {
                // hi-pass filter
                float freqHz = 2.f * std::powf (10.f, 3.f * value + 1.f);
                // For now, use int for max
                if (static_cast<int> (freqHz) == 8000)
                    freqHz = 8000;
                return String (freqHz, 0);
            }
        };

        auto normFreq = std::make_unique<NotifiableAudioParameterFloat> ("freqSEM",
                                                                         "Frequency",
                                                                         freqRange,
                                                                         0.0f,
                                                                         true,// isAutomatable
                                                                         "Cut-off",
                                                                         AudioProcessorParameter::genericParameter,
                                                                         stringFromValue);

        NormalisableRange<float> qRange = { 0.0f, 10.f };
        auto res = std::make_unique<NotifiableAudioParameterFloat> ("resSEM", "resonance", qRange, 0.7071f,
                                                                    true,// isAutomatable
                                                                    "Q",
                                                                    AudioProcessorParameter::genericParameter,
                                                                    [] (float value, int) -> String
                                                                    {
                                                                        if (approximatelyEqual (value, 0.1f))
                                                                            return "0.1";

                                                                        if (approximatelyEqual (value, 10.f))
                                                                            return "10";

                                                                        return String (value, 1);
                                                                    });

        setPrimaryParameter (normFreqParam);
        normFreqParam = normFreq.get();
        normFreqParam->addListener (this);

        resParam = res.get();
        resParam->addListener (this);

        auto layout = createDefaultParameterLayout (false);
        layout.add (std::move (normFreq));
        layout.add (std::move (res));
        apvts.reset (new AudioProcessorValueTreeState (*this, nullptr, "parameters", std::move (layout)));
    }

    ~DigitalSem2() override
    {
        normFreqParam->removeListener (this);
        resParam->removeListener (this);
    }

    void prepareToPlay (double Fs, int bufferSize) override
    {
        const ScopedLock sl (getCallbackLock());
        lpf.setFs (Fs);
        hpf.setFs (Fs);
        mixLPF.reset (Fs, 0.001f);
        mixHPF.reset (Fs, 0.001f);
        setRateAndBufferSizeDetails (Fs, bufferSize);
    }

    //==============================================================================
    /** @internal */
    const String getName() const override { return TRANS ("Digital Sem Two"); }
    /** @internal */
    Identifier getIdentifier() const override { return "Digital Sem Two" + String (idNumber); }
    /** @internal */
    bool supportsDoublePrecisionProcessing() const override { return false; }

    void parameterValueChanged (int paramNum, float value) override
    {
        const ScopedLock sl (getCallbackLock());
        if (paramNum == 1)
        {
            // Frequency change
            lpf.setNormFreq (jmin (1.f, value + 1.f));
            hpf.setNormFreq (jmax (0.0001f, value));

            // if cutoff is set to bypass mode, switch off both processing
            // TODO: use approximatelyEqual when it's finally fixed
            if (abs (value) < 0.0001f)
            {
                mixLPF.setTargetValue (0.f);
                mixHPF.setTargetValue (0.f);
            }
            else if (value < 0.f)
            {
                mixLPF.setTargetValue (1.f);
                mixHPF.setTargetValue (0.f);
            }
            else if (value > 0.f)
            {
                mixLPF.setTargetValue (0.f);
                mixHPF.setTargetValue (1.f);
            }
        }
        else
        {// Resonance change
            lpf.setQValue (value);
            hpf.setQValue (value);
        }
    }

    void parameterGestureChanged (int, bool) override {}

    void processBlock (juce::AudioBuffer<float>& buffer, MidiBuffer&) override { process (buffer); }
    //void processBlock (juce::AudioBuffer<double>& buffer, MidiBuffer&) override  { process (buffer); }

    void process (juce::AudioBuffer<float>& buffer)
    {
        // If the device is turned off, don't process
        if (isBypassed())
            return;

        // If the current value is 0.0, also bypass
        if (abs (normFreqParam->get()) < 0.0001f)
            return;

        lpf.updateCoefficients();
        hpf.updateCoefficients();

        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        const ScopedLock sl (getCallbackLock());

        float x, y, mix, hpv;
        for (int c = 0; c < numChannels; ++c)
        {
            for (int s = 0; s < numSamples; ++s)
            {
                mix = mixLPF.getNextValue();
                x = buffer.getWritePointer (c)[s];
                y = (1.f - mix) * x + mix * lpf.processSample (x, c);

                mix = mixHPF.getNextValue();
                hpv = (float) hpf.processSample (y, c);
                y = (1.f - mix) * y + mix * hpv;
                buffer.getWritePointer (c)[s] = y;
            }
        }
    }

    void setNormFreq (float newNormFreq)
    {
        normFreqParam->setValueNotifyingHost (newNormFreq);
        lpf.setNormFreq (newNormFreq);
        hpf.setNormFreq (newNormFreq);

        if (newNormFreq < 0.f)
        {
            mixLPF.setTargetValue (1.f);
            mixHPF.setTargetValue (0.f);
        }
        else if (newNormFreq > 0.f)
        {
            mixLPF.setTargetValue (0.f);
            mixHPF.setTargetValue (1.f);
        }
        else
        {
            mixLPF.setTargetValue (0.f);
            mixHPF.setTargetValue (0.f);
        }
    }

    // Allowable range from 0.01f to ~10
    void setQValue (float q)
    {
        resParam->setValueNotifyingHost (q);
        lpf.setQValue (q);
        hpf.setQValue (q);
    }
    void setID (int idNum)
    {
        idNumber = idNum;
    }
private:
    //==============================================================================
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    NotifiableAudioParameterFloat* normFreqParam = nullptr;
    NotifiableAudioParameterFloat* resParam = nullptr;

    SmoothedValue<float, ValueSmoothingTypes::Linear> mixLPF { 0.0f };
    SmoothedValue<float, ValueSmoothingTypes::Linear> mixHPF { 0.0f };

    int idNumber = 1;

    DigitalFilter lpf;
    DigitalFilter hpf;
};
}
