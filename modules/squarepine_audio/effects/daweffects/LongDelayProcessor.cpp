namespace djdawprocessor
{

LongDelayProcessor::LongDelayProcessor (int idNum)
    : idNumber (idNum)
{
    reset();

    NormalisableRange<float> wetDryRange = { 0.f, 1.f };
    auto wetdry = std::make_unique<NotifiableAudioParameterFloat> ("dryWetDelay", "Dry/Wet", wetDryRange, 1.f,
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

    NormalisableRange<float> timeRange = { 200.f, 900.0f };
    auto time = std::make_unique<NotifiableAudioParameterFloat> ("delayTime", "Delay Time", timeRange, 300.f,
                                                                 true,// isAutomatable
                                                                 "Delay Time",
                                                                 AudioProcessorParameter::genericParameter,
                                                                 [] (float value, int) -> String
                                                                 {
                                                                     String txt (roundToInt (value));
                                                                     return txt << "ms";
                                                                     ;
                                                                 });

    NormalisableRange<float> feedbackRange = { 0.f, 1.0f };
    auto feedback = std::make_unique<NotifiableAudioParameterFloat> ("feedback", "Feedback", feedbackRange, 0.5f,
                                                                     true,// isAutomatable
                                                                     "Feedback ",
                                                                     AudioProcessorParameter::genericParameter,
                                                                     [] (float value, int) -> String
                                                                     {
                                                                         int percentage = roundToInt (value * 100);
                                                                         String txt (percentage);
                                                                         return txt << "%";
                                                                     });

    wetDryParam = wetdry.get();
    wetDryParam->addListener (this);

    fxOnParam = fxon.get();
    fxOnParam->addListener (this);

//    colourParam = colour.get();
//    colourParam->addListener (this);

    feedbackParam = feedback.get();
    feedbackParam->addListener (this);

    timeParam = time.get();
    timeParam->addListener (this);

    auto layout = createDefaultParameterLayout (false);
    layout.add (std::move (fxon));
    layout.add (std::move (wetdry));
    //layout.add (std::move (colour));
    layout.add (std::move (time));
    layout.add (std::move (feedback));
    
    appendExtraParams (layout);
    apvts.reset (new AudioProcessorValueTreeState (*this, nullptr, "parameters", std::move (layout)));

    setPrimaryParameter (wetDryParam);
    
    delayTime.setTargetValue (timeParam->get());
    wetDry.setTargetValue (wetDryParam->get());
}

LongDelayProcessor::~LongDelayProcessor()
{
    wetDryParam->removeListener (this);
    fxOnParam->removeListener (this);
    //colourParam->removeListener (this);
    feedbackParam->removeListener (this);
    timeParam->removeListener (this);
}

//============================================================================== Audio processing
void LongDelayProcessor::prepareToPlay (double sampleRate, int)
{
    Fs = static_cast<float> (sampleRate);
    delayUnit.setFs (Fs);
    wetDry.reset (Fs, 0.5f);
    delayTime.reset (Fs, 0.5f);
    delayUnit.setDelaySamples (delayTime.getNextValue()/1000.f * Fs);
}
void LongDelayProcessor::processBlock (juce::AudioBuffer<float>& buffer, MidiBuffer&)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    bool bypass;
    float feedback;
    {
        const ScopedLock sl (getCallbackLock());
        bypass = !fxOnParam->get();
        feedback = feedbackParam->get() * 0.75f; // max feedback gain is 0.75
        float timeMS = timeParam->get();
        float samplesOfDelay = timeMS/1000.f * Fs;
        delayTime.setTargetValue (samplesOfDelay);
        delayUnit.setDelaySamples (delayTime.getNextValue());
    }
    
    if (bypass)
        return;
    
    float dry, wet, x, y;
    
    
    for (int s = 0; s < numSamples; ++s)
    {
        wet = wetDry.getNextValue();
        delayTime.getNextValue(); // continue smoothing
        dry = 1.f - wet;
        for (int c = 0; c < numChannels; ++c)
        {
            x = buffer.getWritePointer (c)[s];
            z[c] = delayUnit.processSample (x + feedback * z[c], c);
            y = (z[c] * wet) + (x * dry);
            buffer.getWritePointer (c)[s] = y;
        }
    }
}

const String LongDelayProcessor::getName() const { return TRANS ("Long Delay"); }
/** @internal */
Identifier LongDelayProcessor::getIdentifier() const { return "Long Delay" + String (idNumber); }
/** @internal */
bool LongDelayProcessor::supportsDoublePrecisionProcessing() const { return false; }
//============================================================================== Parameter callbacks
void LongDelayProcessor::parameterValueChanged (int paramIndex, float value)
{
    const ScopedLock sl (getCallbackLock());
    switch (paramIndex)
    {
        case (1):
        {
            // fx on/off (handled in processBlock)
            break;
        }
        case (2):
        {
            wetDry.setTargetValue (value);
            //
            break;
        }
        case (3):
        {
            
            float samplesOfDelay = value/1000.f * Fs;
            delayTime.setTargetValue (samplesOfDelay);
            delayUnit.setDelaySamples (samplesOfDelay);
            
            break;
        }
        case (4):
        {
        
            break;
        }
    }
}

}
