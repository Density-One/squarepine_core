namespace djdawprocessor
{
ReverbProcessor::ReverbProcessor (int idNum)
    : idNumber (idNum)
{
    reset();

    NormalisableRange<float> wetDryRange = { 0.f, 1.f };
    auto wetdry = std::make_unique<NotifiableAudioParameterFloat> ("dryWet", "Dry/Wet", wetDryRange, 0.25f,
                                                                   true,// isAutomatable
                                                                   "Dry/Wet",
                                                                   AudioProcessorParameter::genericParameter,
                                                                   [] (float value, int) -> String
                                                                   {
                                                                       int percentage = roundToInt (value * 100);
                                                                       String txt (percentage);
                                                                       return txt << "%";
                                                                   });
    auto fxon = std::make_unique<AudioParameterBool> ("fxonoff", "FX On", true, "FX On/Off ", [] (bool value, int) -> String
                                                      {
                                                          if (value > 0)
                                                              return TRANS ("On");
                                                          return TRANS ("Off");
                                                          ;
                                                      });

    NormalisableRange<float> filterRange = { 0.f, 1 };
    auto filterAmount = std::make_unique<NotifiableAudioParameterFloat> ("amount", "Filter Amount ", filterRange, 0.5,
                                                                         true,// isAutomatable
                                                                         "Reverb Filter Amount ",
                                                                         AudioProcessorParameter::genericParameter,
                                                                         [] (float value, int) -> String
                                                                         {
                                                                             int percentage = roundToInt (value * 100);
                                                                             String txt (percentage);
                                                                             return txt << "%";
                                                                         });

    NormalisableRange<float> timeRange = { 0, 1.0f };
    auto time = std::make_unique<NotifiableAudioParameterFloat> ("time", "Time", timeRange, 0.5f,
                                                                 true,// isAutomatable
                                                                 "Time ",
                                                                 AudioProcessorParameter::genericParameter,
                                                                 [] (float value, int) -> String
                                                                 {
                                                                     int percentage = roundToInt (value * 100);
                                                                     String txt (percentage);
                                                                     return txt << "%";
                                                                 });

    NormalisableRange<float> decayRange = { 0, 1.0f };
    auto decay = std::make_unique<NotifiableAudioParameterFloat> ("decay", "Decay", decayRange, 0.25f,
                                                                  true,// isAutomatable
                                                                  "Decay ",
                                                                  AudioProcessorParameter::genericParameter,
                                                                  [] (float value, int) -> String
                                                                  {
                                                                      int percentage = roundToInt (value * 100);
                                                                      String txt (percentage);
                                                                      return txt << "%";
                                                                  });

    NormalisableRange<float> sizeRange = { 0, 1.0f };
    auto size = std::make_unique<NotifiableAudioParameterFloat> ("size", "Size", sizeRange, 0.75f,
                                                                 true,// isAutomatable
                                                                 "Size ",
                                                                 AudioProcessorParameter::genericParameter,
                                                                 [] (float value, int) -> String
                                                                 {
                                                                     int percentage = roundToInt (value * 100);
                                                                     String txt (percentage);
                                                                     return txt << "%";
                                                                 });
    NormalisableRange<float> scatterRange = { 0, 1.0f };
    auto scatter = std::make_unique<NotifiableAudioParameterFloat> ("scattering", "Scattering", sizeRange, 0.75f,
                                                                    true,// isAutomatable
                                                                    "Scattering ",
                                                                    AudioProcessorParameter::genericParameter,
                                                                    [] (float value, int) -> String
                                                                    {
                                                                        int percentage = roundToInt (value * 100);
                                                                        String txt (percentage);
                                                                        return txt << "%";
                                                                    });
    NormalisableRange<float> preDelayRange = { 0, 1.0f };
    auto predelay = std::make_unique<NotifiableAudioParameterFloat> ("preDelay", "PreDelay", preDelayRange, 0.05f,
                                                                     true,// isAutomatable
                                                                     "PreDelay ",
                                                                     AudioProcessorParameter::genericParameter,
                                                                     [] (float value, int) -> String
                                                                     {
                                                                         return String (value) + " seconds";
                                                                     });

    NormalisableRange<float> modFreqRange = { 0.05f, 5.0f };
    auto modFreq = std::make_unique<NotifiableAudioParameterFloat> ("modFreq", "ModFreq", modFreqRange, 0.05f,
                                                                    true,// isAutomatable
                                                                    "ModFreq ",
                                                                    AudioProcessorParameter::genericParameter,
                                                                    [] (float value, int) -> String
                                                                    {
                                                                        return String (value) + " hz";
                                                                    });

    NormalisableRange<float> modDepthRange = { 0.f, 1.0f };
    auto modDepth = std::make_unique<NotifiableAudioParameterFloat> ("modDepth", "ModDepth", modDepthRange, 0.0f,
                                                                     true,// isAutomatable
                                                                     "ModFreq ",
                                                                     AudioProcessorParameter::genericParameter,
                                                                     [] (float value, int) -> String
                                                                     {
                                                                         return String (value * 100) + "%";
                                                                     });

    NormalisableRange<float> lowDampRange = { 20.f, 20000.0f };
    auto lowDamp = std::make_unique<NotifiableAudioParameterFloat> ("lowDamp", "LowDamping", lowDampRange, 10000.0f,
                                                                    true,// isAutomatable
                                                                    "LowDamping ",
                                                                    AudioProcessorParameter::genericParameter,
                                                                    [] (float value, int) -> String
                                                                    {
                                                                        return String (value) + "hz";
                                                                    });
    NormalisableRange<float> highDampRange = { 20.f, 2000.0f };
    auto highDamp = std::make_unique<NotifiableAudioParameterFloat> ("highDamp", "HighDamping", highDampRange, 100.0f,
                                                                     true,// isAutomatable
                                                                     "HighDamping ",
                                                                     AudioProcessorParameter::genericParameter,
                                                                     [] (float value, int) -> String
                                                                     {
                                                                         return String (value) + "hz";
                                                                     });

    wetDryParam = wetdry.get();
    wetDryParam->addListener (this);

    filterParam = filterAmount.get();
    filterParam->addListener (this);

    timeParam = time.get();
    timeParam->addListener (this);

    fxOnParam = fxon.get();
    fxOnParam->addListener (this);

    /*
    
    decayParam = decay.get();
    decayParam->addListener (this);

    sizeParam = size.get();
    sizeParam->addListener (this);

    preDelayParam = predelay.get();
    preDelayParam->addListener (this);

    modFrequencyParam = modFreq.get();
    modFrequencyParam->addListener (this);

    modDepthParam = modDepth.get();
    modDepthParam->addListener (this);

    lowDampParam = lowDamp.get();
    lowDampParam->addListener (this);

    highDampParam = highDamp.get();
    highDampParam->addListener (this);

    scatteringParam = scatter.get();
    scatteringParam->addListener (this);

     */
    auto layout = createDefaultParameterLayout (false);
    layout.add (std::move (fxon));
    layout.add (std::move (wetdry));
    layout.add (std::move (time));
    layout.add (std::move (filterAmount));
    /*
    layout.add (std::move (decay));
    layout.add (std::move (size));
    layout.add (std::move (scatter));
    layout.add (std::move (predelay));
    layout.add (std::move (modFreq));
    layout.add (std::move (modDepth));
    layout.add (std::move (lowDamp));
    layout.add (std::move (highDamp));
     */

    setupBandParameters (layout);
    apvts.reset (new AudioProcessorValueTreeState (*this, nullptr, "parameters", std::move (layout)));

    setPrimaryParameter (wetDryParam);

    hpf.setFilterType (DigitalFilter::FilterType::HPF);
    hpf.setFreq (200.f);
    lpf.setFilterType (DigitalFilter::FilterType::LPF);
    lpf.setFreq (10000.f);

    setEffectiveInTimeDomain (true);
    //   setIsInSteppedTimeMode (true);
}

ReverbProcessor::~ReverbProcessor()
{
    wetDryParam->removeListener (this);
    fxOnParam->removeListener (this);
    filterParam->removeListener (this);
    timeParam->removeListener (this);
}

//============================================================================== Audio processing
void ReverbProcessor::prepareToPlay (double Fs, int bufferSize)
{
    setRateAndBufferSizeDetails (Fs, bufferSize);
    BandProcessor::prepareToPlay (Fs, bufferSize);

    //matrixReverb.setMaxBlockSize (maxBlockSize);
    updateReverbParams (bufferSize);
    matrixReverb.setSampleRate (static_cast<float> (Fs));
    hpf.setFs (Fs);
    lpf.setFs (Fs);
}
void ReverbProcessor::processAudioBlock (juce::AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    ignoreUnused (midi);

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    float wetLevel = wetDryParam->get();
    float wet;
    float dry;
    bool bypass;
    {
        const ScopedLock sl (getCallbackLock());
        bypass = ! fxOnParam->get();

        wet = sin (wetLevel * juce::MathConstants<float>::pi / 2);
        dry = cos (wetLevel * juce::MathConstants<float>::pi / 2);
    }

    if (bypass || isBypassed())
        return;

    updateReverbParams (numSamples);

    fillMultibandBuffer (buffer);

    auto chans = multibandBuffer.getArrayOfWritePointers();

    const ScopedLock sl (getCallbackLock());

    matrixReverb.processBlock (chans[0], numChannels > 0 ? chans[1] : NULL, preDelayVector.data(), sizeVector.data(), decayVector.data(), scatteringVector.data(), modFrequencyVector.data(), modDepthVector.data(), lowDampVector.data(), highDampVector.data(), 0, chans[0], numChannels > 0 ? chans[1] : NULL, numSamples);

    // Reverb comes out extremely hot, duck by -6db/0.5rms
    multibandBuffer.applyGain (0.5f);

    lpf.processBuffer (multibandBuffer, midi);
    hpf.processBuffer (multibandBuffer, midi);

    buffer.applyGain (dry);
    multibandBuffer.applyGain (wet);

    for (int c = 0; c < numChannels; ++c)
        buffer.addFrom (c, 0, multibandBuffer.getWritePointer (c), numSamples);
}

const String ReverbProcessor::getName() const { return TRANS ("Reverb"); }
/** @internal */
Identifier ReverbProcessor::getIdentifier() const { return "Reverb" + String (idNumber); }
/** @internal */
bool ReverbProcessor::supportsDoublePrecisionProcessing() const { return false; }
//============================================================================== Parameter callbacks
void ReverbProcessor::parameterValueChanged (int id, float value)
{
    //If the beat division is changed, the delay time should be set.
    //If the X Pad is used, the beat div and subsequently, time, should be updated.
    // DBG (value);
    if (id == 1)
    {
        setBypass (value > 0);
    }
    if (id == 4)// filterParam
    {
        if (value > 0.5f)
        {
            float normValue = 2.f * (value - 0.5f);
            float freqHz = std::powf (10.f, 1.6f * normValue + 2.f) + 100.f;// 200 - 4000
            hpf.setFreq (freqHz);
            lpf.setFreq (10000.f);
        }
        else
        {
            float normValue = value * 2.f;
            float freqHz = std::powf (10.f, normValue + 3.f);// 10000 -> 2000
            lpf.setFreq (freqHz);
            hpf.setFreq (200.f);
        }
    }
    //Subtract the number of new parameters in this processor
    BandProcessor::parameterValueChanged (id, value);
}
void ReverbProcessor::releaseResources()
{
    const ScopedLock sl (getCallbackLock());
    matrixReverb.reset();
}

float ReverbProcessor::interpolate (float input, float x1, float x2, float y1, float y2)
{
    return y1 + (input - x1) * (y2 - y1) / (x2 - x1);
}
float ReverbProcessor::mapTime (float inputPercentage)
{
    inputPercentage = std::clamp (inputPercentage, 0.0f, 1.0f);// Ensure input is between 0 and 1

    inputPercentage = std::clamp (inputPercentage, 0.0f, 1.0f);// Ensure input is between 0 and 1

    // Normalized output values between 0 and 1 corresponding to 100, 85, 57, 30, 15, and 0
    // These values are based on analysis from recordings of the v10 at fixed positions of 0, 25, 50,75,100
    const float output100 = 1.0f;
    const float output85 = 0.85f;
    const float output57 = 0.57f;
    const float output30 = 0.30f;
    const float output15 = 0.15f;
    const float output0 = 0.0f;

    // Map specific input ranges to corresponding normalized output ranges
    if (inputPercentage == 1.0f)
        return output100;
    if (inputPercentage >= 0.75f)
        return interpolate (inputPercentage, 0.75f, 1.0f, output85, output100);
    if (inputPercentage >= 0.50f)
        return interpolate (inputPercentage, 0.50f, 0.75f, output57, output85);
    if (inputPercentage >= 0.25f)
        return interpolate (inputPercentage, 0.25f, 0.50f, output30, output57);
    if (inputPercentage >= 0.10f)
        return interpolate (inputPercentage, 0.10f, 0.25f, output15, output30);
    return interpolate (inputPercentage, 0.0f, 0.10f, output0, output15);// For inputs between 0 and 10%
}

void ReverbProcessor::updateReverbParams (int numSamples)
{
    auto updateSize = [] (std::vector<float>& vector, int sampleCount)
    {
        if (static_cast<int> (vector.size()) != sampleCount)
            vector.resize (static_cast<unsigned long> (sampleCount));
    };

    {
        auto lowDamp = 20000.f;
        auto time = timeParam->get();
        auto scattering = 0.25f;
        if (time < 0.5)
            lowDamp = 18000.f;

        if (time < 0.3)
            scattering = 0.1f;

        auto sizeDecay = mapTime (time);
        constexpr auto predelay = 0.001f;
        constexpr auto modFreq = 0.4f;
        constexpr auto modDepth = 0.1f;
        constexpr auto highDamp = 20.f;

        const ScopedLock sl (getCallbackLock());
        updateSize (preDelayVector, numSamples);
        FloatVectorOperations::fill (preDelayVector.data(), predelay, numSamples);
        updateSize (sizeVector, numSamples);
        FloatVectorOperations::fill (sizeVector.data(), sizeDecay, numSamples);
        updateSize (decayVector, numSamples);
        FloatVectorOperations::fill (decayVector.data(), sizeDecay, numSamples);
        updateSize (scatteringVector, numSamples);
        FloatVectorOperations::fill (scatteringVector.data(), scattering, numSamples);
        updateSize (modFrequencyVector, numSamples);
        FloatVectorOperations::fill (modFrequencyVector.data(), modFreq, numSamples);
        updateSize (modDepthVector, numSamples);
        FloatVectorOperations::fill (modDepthVector.data(), modDepth, numSamples);
        updateSize (lowDampVector, numSamples);
        FloatVectorOperations::fill (lowDampVector.data(), lowDamp, numSamples);
        updateSize (highDampVector, numSamples);
        FloatVectorOperations::fill (highDampVector.data(), highDamp, numSamples);
    }
}
}
