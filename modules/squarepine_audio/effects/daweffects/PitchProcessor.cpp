namespace djdawprocessor
{

PitchProcessor::PitchProcessor (int idNum)
    : idNumber (idNum)
{
    reset();

    NormalisableRange<float> wetDryRange = { 0.f, 1.f };
    auto wetdry = std::make_unique<NotifiableAudioParameterFloat> ("dryWet", "Dry/Wet", wetDryRange, 0.5f,
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

    NormalisableRange<float> pitchRange = { -50.f, 100.0f };
    auto pitch = std::make_unique<NotifiableAudioParameterFloat> ("pitch", "Pitch", pitchRange, 0.f,
                                                                  true,// isAutomatable
                                                                  "Pitch ",
                                                                  AudioProcessorParameter::genericParameter,
                                                                  [] (float value, int) -> String
                                                                  {
                                                                      String txt (roundToInt (value));
                                                                      return txt << "%";
                                                                      ;
                                                                  });

    wetDryParam = wetdry.get();
    wetDryParam->addListener (this);

    fxOnParam = fxon.get();
    fxOnParam->addListener (this);

    pitchParam = pitch.get();
    pitchParam->addListener (this);

    auto layout = createDefaultParameterLayout (false);
    layout.add (std::move (fxon));
    layout.add (std::move (wetdry));
    layout.add (std::move (pitch));
    setupBandParameters (layout);
    apvts.reset (new AudioProcessorValueTreeState (*this, nullptr, "parameters", std::move (layout)));

    setPrimaryParameter (wetDryParam);
}

PitchProcessor::~PitchProcessor()
{
    wetDryParam->removeListener (this);
    fxOnParam->removeListener (this);
    pitchParam->removeListener (this);
}

//============================================================================== Audio processing
void PitchProcessor::prepareToPlay (double Fs, int bufferSize)
{
    BandProcessor::prepareToPlay (Fs, bufferSize);

    pitchShifter.setFs (static_cast<float> (Fs));
    pitchShifter.setPitch (12.f);

    effectBuffer = AudioBuffer<float> (2, bufferSize);

#if SQUAREPINE_USE_ELASTIQUE

    const auto mode = useElastiquePro
                          ? CElastiqueProV3If::kV3Pro
                          : CElastiqueProV3If::kV3Eff;

    elastique = zplane::createElastiquePtr (bufferSize, 2, Fs, mode);

    if (elastique == nullptr)
    {
        jassertfalse;// Something failed...
    }

    elastique->Reset();

    auto pitchFactor = (float) std::clamp (2.0, 0.25, 4.0);// 2.0 = up an octave (double frequency)
    auto localRatio = (float) std::clamp (1.0, 0.01, 10.0);
    zplane::isValid (elastique->SetStretchPitchQFactor (localRatio, pitchFactor, useElastiquePro));

    outputBuffer = AudioBuffer<float> (2, bufferSize);

#endif
}
void PitchProcessor::processAudioBlock (juce::AudioBuffer<float>& buffer, MidiBuffer&)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    float wet;
    bool bypass;
    {
        const ScopedLock sl (getCallbackLock());
        wet = wetDryParam->get();
        bypass = ! fxOnParam->get();
    }

    if (bypass || isBypassed())
        return;

    fillMultibandBuffer (buffer);
    //
    const auto numSamplesToRead = elastique->GetFramesNeeded (static_cast<int> (numSamples));

    effectBuffer.setSize (2, numSamplesToRead, false, true, true);

    for (int c = 0; c < numChannels; ++c)
    {
        int index = numSamplesToRead - numSamples;
        for (int n = 0; n < numSamples; ++n)
        {
            float x = multibandBuffer.getWritePointer (c)[n];
            multibandBuffer.getWritePointer (c)[n] = (1.f - wetSmooth[c]) * x;

            float y = wetSmooth[c] * x;

            effectBuffer.getWritePointer (c)[index] = y;
            buffer.getWritePointer (c)[index] *= (1.f - wetSmooth[c]);

            wetSmooth[c] = 0.999f * wetSmooth[c] + 0.001f * wet;

            ++index;
        }
    }

    auto inChannels = effectBuffer.getArrayOfReadPointers();
    auto outChannels = outputBuffer.getArrayOfWritePointers();
    zplane::isValid (elastique->ProcessData ((float**) inChannels, numSamplesToRead, (float**) outChannels));

    const ScopedLock sl (getCallbackLock());

    for (int c = 0; c < numChannels; ++c)
        buffer.addFrom (c, 0, outputBuffer.getWritePointer (c), numSamples);
}

const String PitchProcessor::getName() const { return TRANS ("Pitch"); }
/** @internal */
Identifier PitchProcessor::getIdentifier() const { return "Pitch" + String (idNumber); }
/** @internal */
bool PitchProcessor::supportsDoublePrecisionProcessing() const { return false; }
//============================================================================== Parameter callbacks
void PitchProcessor::parameterValueChanged (int paramIndex, float value)
{
    //If the beat division is changed, the delay time should be set.
    //If the X Pad is used, the beat div and subsequently, time, should be updated.

    //Subtract the number of new parameters in this processor
    //If the beat division is changed, the delay time should be set.
    //If the X Pad is used, the beat div and subsequently, time, should be updated.
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
            //wetDry.setTargetValue (value);
            //
            break;
        }
        case (3):
        {
            float pitchFactor = 1.f + value / 100.f;
            elastique->SetStretchPitchQFactor (1.f, pitchFactor, useElastiquePro);

            break;
        }
    }
}

}
