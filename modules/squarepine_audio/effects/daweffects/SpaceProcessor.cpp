namespace djdawprocessor
{

SpaceProcessor::SpaceProcessor (int idNum)
    : idNumber (idNum)
{
    reset();

    NormalisableRange<float> wetDryRange = { 0.f, 1.f };
    auto wetdry = std::make_unique<NotifiableAudioParameterFloat> ("dryWetDelay", "Dry/Wet", wetDryRange, 0.25f,
                                                                   true,// isAutomatable
                                                                   "Dry/Wet",
                                                                   AudioProcessorParameter::genericParameter,
                                                                   [] (float value, int) -> String
                                                                   {
                                                                       int percentage = roundToInt (value * 100);
                                                                       String txt (percentage);
                                                                       return txt << "%";
                                                                   });

    auto fxon = std::make_unique<NotifiableAudioParameterBool> ("fxonoff", "FX On", true, "FX On/Off ", [] (bool value, int) -> String
                                                                {
                                                                    if (value > 0)
                                                                        return TRANS ("On");
                                                                    return TRANS ("Off");
                                                                    ;
                                                                });

    NormalisableRange<float> reverbRange = { -1.0, 1.0f };
    auto reverbColour = std::make_unique<NotifiableAudioParameterFloat> ("reverb colour", "Colour/Tone", reverbRange, 0.f,
                                                                         true,// isAutomatable
                                                                         "Colour ",
                                                                         AudioProcessorParameter::genericParameter,
                                                                         [] (float value, int) -> String
                                                                         {
                                                                             String txt (std::round (100.f * value) / 100.f);
                                                                             return txt;
                                                                             ;
                                                                         });

    NormalisableRange<float> otherRange = { 0.f, 1.0f };
    auto other = std::make_unique<NotifiableAudioParameterFloat> ("length", "Length", otherRange, 0.5f,
                                                                  true,// isAutomatable
                                                                  "Length ",
                                                                  AudioProcessorParameter::genericParameter,
                                                                  [] (float value, int) -> String
                                                                  {
                                                                      int percentage = roundToInt (value * 100);
                                                                      String txt (percentage);
                                                                      return txt << "%";
                                                                  });

    wetDryParam = wetdry.get();
    wetDryParam->addListener (this);

    wetDry.setTargetValue (wetDryParam->get());

    fxOnParam = fxon.get();
    fxOnParam->addListener (this);

    reverbColourParam = reverbColour.get();
    reverbColourParam->addListener (this);

    otherParam = other.get();
    otherParam->addListener (this);

    auto layout = createDefaultParameterLayout (false);
    layout.add (std::move (fxon));
    layout.add (std::move (wetdry));
    layout.add (std::move (reverbColour));
    layout.add (std::move (other));
    appendExtraParams (layout);

    apvts.reset (new AudioProcessorValueTreeState (*this, nullptr, "parameters", std::move (layout)));

    dampLPF.setFilterType (DigitalFilter::FilterType::LPF);
    dampLPF.setFreq (13000.f);
    dampLPF.setQ (0.2f);

    setPrimaryParameter (reverbColourParam);
    setEffectiveInTimeDomain (true);
}

SpaceProcessor::~SpaceProcessor()
{
    wetDryParam->removeListener (this);
    fxOnParam->removeListener (this);
    reverbColourParam->removeListener (this);
    otherParam->removeListener (this);
}

//============================================================================== Audio processing
void SpaceProcessor::prepareToPlay (double sampleRate, int bufferSize)
{
    //reverb.reset();
    //reverb.setSampleRate (sampleRate);
    wetDry.reset (sampleRate, 0.5f);
    processedBuffer = AudioBuffer<float> (2, bufferSize);
    reverb.prepareToPlay (sampleRate, bufferSize);
    dampLPF.setFs (sampleRate);
    filter.setFs (sampleRate);
    filter.setFilterType (DigitalFilter::FilterType::PEAK);
    filter.setQ (0.5f);
}

void SpaceProcessor::processBlock (juce::AudioBuffer<float>& buffer, MidiBuffer& midiBuffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    bool bypass;
    {
        const ScopedLock sl (getCallbackLock());
        bypass = ! fxOnParam->get();
    }

    if (bypass || isBypassed())
        return;

    updateReverbParams();
    
    for (int c = 0; c < numChannels; ++c)
        processedBuffer.copyFrom(c, 0, buffer.getWritePointer (c), numSamples);
    
    filter.processBuffer (processedBuffer, midiBuffer);
    dampLPF.processBuffer (processedBuffer, midiBuffer);

    //auto chans = buffer.getArrayOfWritePointers();

    const ScopedLock sl (getCallbackLock());

    //    switch (numChannels)
    //    {
    //        case 1:
    //            reverb.processMono (chans[0], numSamples);
    //            break;
    //
    //        case 2:
    //            reverb.processStereo (chans[0], chans[1], numSamples);
    //            break;
    //
    //        default:
    //            break;
    //    }
    float wet;
    float dry;
    for (int s = 0; s < numSamples; ++s)
    {
        wet = wetDry.getNextValue();
        dry = 1.f - wet;
        for (int c = 0; c < numChannels; ++c)
        {
            float x = processedBuffer.getWritePointer (c)[s];
            float y = reverb.processSample (x, c);
            processedBuffer.getWritePointer (c)[s] = wet * y;
            buffer.getWritePointer (c)[s] *= dry;
        }
    }
    
    for (int c = 0; c < numChannels; ++c)
        buffer.addFrom (c, 0, processedBuffer.getWritePointer (c), numSamples);
}

const String SpaceProcessor::getName() const { return TRANS ("Space"); }
/** @internal */
Identifier SpaceProcessor::getIdentifier() const { return "Space" + String (idNumber); }
/** @internal */
bool SpaceProcessor::supportsDoublePrecisionProcessing() const { return false; }
//============================================================================== Parameter callbacks
void SpaceProcessor::parameterValueChanged (int id, float value)
{
    if (id == 1)
    {
        setBypass (value > 0);
    }
    if (id == 2)
    {
        wetDry.setTargetValue (value);
    }
    if (id == 3)// Color
    {
        if (value > 0)
        {
            filter.setFreq (5000.f);
            filter.setAmpdB (value * 6.f);
        }
        else
        {
            filter.setFreq (500.f);
            filter.setAmpdB (-value * 6.f);
        }
    }
}

void SpaceProcessor::releaseResources()
{
    //const ScopedLock sl (getCallbackLock());
    //reverb.reset();
}

void SpaceProcessor::updateReverbParams()
{
//    Reverb::Parameters localParams;

//    localParams.roomSize = otherParam->get();
//    localParams.damping = 0.2f;//1.f - reverbColourParam->get();
//    localParams.wetLevel = wetDryParam->get();
//    localParams.dryLevel = 1.f - wetDryParam->get();
//    if (abs (reverbColourParam->get()) < 0.01f)
//    {
//        localParams.wetLevel = 0.f;
//        localParams.dryLevel = 1.f;
//    }
//
//    localParams.width = 1;
//    localParams.freezeMode = 0;

    {
        const ScopedLock sl (getCallbackLock());
        //reverb.setParameters (localParams);
        float val = otherParam->get();
        
        reverb.setFeedbackGain (val);
    }
}

}
