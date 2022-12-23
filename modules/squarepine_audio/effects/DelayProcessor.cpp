FractionalDelay::FractionalDelay()
{
}
// Destructor
FractionalDelay::~FractionalDelay()
{
}

float FractionalDelay::processSample (float x, int channel)
{
    smoothDelay[channel] = 0.999f * smoothDelay[channel] + 0.001f * delay;

    if (smoothDelay[channel] < 1.f)
    {
        return x;
    }
    else
    {
        // Delay Buffer
        // "delay" can be fraction
        int d1 = (int) floor (smoothDelay[channel]);
        int d2 = d1 + 1;
        float g2 = smoothDelay[channel] - (float) d1;
        float g1 = 1.0f - g2;

        int indexD1 = index[channel] - d1;
        if (indexD1 < 0)
        {
            indexD1 += MAX_BUFFER_SIZE;
        }

        int indexD2 = index[channel] - d2;
        if (indexD2 < 0)
        {
            indexD2 += MAX_BUFFER_SIZE;
        }

        float y = g1 * delayBuffer[indexD1][channel] + g2 * delayBuffer[indexD2][channel];

        delayBuffer[index[channel]][channel] = x;

        if (index[channel] < MAX_BUFFER_SIZE - 1)
        {
            index[channel]++;
        }
        else
        {
            index[channel] = 0;
        }

        return y;
    }
}

void FractionalDelay::setFs (float _Fs)
{
    this->Fs = _Fs;
}

void FractionalDelay::setDelaySamples (float _delay)
{
    if (delay >= 1.f)
    {
        delay = _delay;
    }
    else
    {
        delay = 0.f;
    }
}

void FractionalDelay::clearDelay()
{
    for (int i = 0; i < MAX_BUFFER_SIZE; ++i)
    {
        for (int c = 0; c < 2; ++c)
        {
            delayBuffer[i][c] = 0;
        }
    }
}

DelayProcessor::DelayProcessor (int idNum): InsertProcessor (true),
                                            idNumber (idNum)
{
    reset();

    NormalisableRange<float> wetDryRange = { 0.f, 1.f };
    auto wetdry = std::make_unique<NotifiableAudioParameterFloat> ("dryWetDelay", "Dry/Wet", wetDryRange, 0.5f,
                                                                   true,// isAutomatable
                                                                   "Dry/Wet",
                                                                   AudioProcessorParameter::genericParameter,
                                                                   [] (float value, int) -> String {
                                                                       int percentage = roundToInt (value * 100);
                                                                       String txt (percentage);
                                                                       return txt << "%";
                                                                   });

    StringArray options { "1/16", "1/8", "1/4", "1/2", "1", "2", "4", "8", "16" };
    auto beat = std::make_unique<AudioParameterChoice> ("beat", "Beat Division", options, 3);

    NormalisableRange<float> timeRange = { 1.f, 4000.0f };
    auto time = std::make_unique<NotifiableAudioParameterFloat> ("delayTime", "Delay Time", timeRange, 200.f,
                                                                 true,// isAutomatable
                                                                 "Delay Time",
                                                                 AudioProcessorParameter::genericParameter,
                                                                 [] (float value, int) -> String {
                                                                     String txt (roundToInt (value));
                                                                     return txt << "ms";
                                                                     ;
                                                                 });

    NormalisableRange<float> beatRange = { 0.f, 8.0 };
    auto other = std::make_unique<NotifiableAudioParameterFloat> ("x Pad", "X Pad Division", beatRange, 3,
                                                                  false,// isAutomatable
                                                                  "X Pad Division ",
                                                                  AudioProcessorParameter::genericParameter,
                                                                  [] (float value, int) -> String {
                                                                      int val = roundToInt (value);
                                                                      String txt;
                                                                      switch (val)
                                                                      {
                                                                          case 0:
                                                                              txt = "1/16";
                                                                              break;
                                                                          case 1:
                                                                              txt = "1/8";
                                                                              break;
                                                                          case 2:
                                                                              txt = "1/4";
                                                                              break;
                                                                          case 3:
                                                                              txt = "1/2";
                                                                              break;
                                                                          case 4:
                                                                              txt = "1";
                                                                              break;
                                                                          case 5:
                                                                              txt = "2";
                                                                              break;
                                                                          case 6:
                                                                              txt = "4";
                                                                              break;
                                                                          case 7:
                                                                              txt = "8";
                                                                              break;
                                                                          case 8:
                                                                              txt = "16";
                                                                              break;
                                                                          default:
                                                                              txt = "1";
                                                                              break;
                                                                      }

                                                                      return txt;
                                                                  });

    NormalisableRange<float> feedbackRange = { 0.f, 1.f };
    auto feedback = std::make_unique<NotifiableAudioParameterFloat> ("feedback", "Feedback", feedbackRange, 0.5f,
                                                                     true,// isAutomatable
                                                                     "Feedback",
                                                                     AudioProcessorParameter::genericParameter,
                                                                     [] (float value, int) -> String {
                                                                         int percentage = roundToInt (value * 100);
                                                                         String txt (percentage);
                                                                         return txt << "%";
                                                                     });

    auto sync = std::make_unique<SwitchableTimeParameter>();

    auto test = std::make_unique<NotifiableAudioParameterBool> ("t",
                                                                "t",
                                                                true,
                                                                "Test param",
                                                                true,
                                                                [] (bool value, int) -> String {
                                                                    if (value > 0)
                                                                        return TRANS ("On");
                                                                    return TRANS ("Off");
                                                                    ;
                                                                });

    delayUnit.setDelaySamples (200 * 48);
    wetDry.setTargetValue (0.5);
    delayTime.setTargetValue (200 * 48);

    beatParam = beat.get();
    beatParam->addListener (this);

    wetDryParam = wetdry.get();

    delayTimeParam = time.get();
    delayTimeParam->addListener (this);

    xPadParam = other.get();
    xPadParam->addListener (this);

    feedbackParam = feedback.get();
    feedbackParam->addListener (this);

    syncParam = sync.get();
    syncParam->addListener (this);

    testParam = test.get();
    testParam->addListener (this);

    auto layout = createDefaultParameterLayout (false);
    setupDefaultParametersAndCallbacks (layout);

    layout.add (std::move (wetdry));
    addParameterWithCallback (wetDryParam, [&] (float& value) {
        wetDry.setTargetValue (jlimit (0.f, 1.f, value));
    });

    layout.add (std::move (beat));
    addParameterWithCallback (beatParam, [&] (float&) {
        DBG ("Beat");
    });

    layout.add (std::move (time));
    addParameterWithCallback (delayTimeParam, [&] (float& value) {//delay time
        auto range = delayTimeParam->getNormalisableRange().getRange();
        auto valMS = (float) jlimit ((int) range.getStart(), (int) range.getEnd(), roundToInt (value));
        auto t = (getSampleRate() / 1000) * valMS;
        delayTime.setTargetValue ((float) t);
    });

    layout.add (std::move (other));
    addParameterWithCallback (xPadParam, [&] (float&) {
      
    });

    layout.add (std::move (feedback));
    addParameterWithCallback (feedbackParam, [&] (float&) {
      
    });

    // sync->addChildrenToLayout (layout);

    apvts.reset (new AudioProcessorValueTreeState (*this, nullptr, "parameters", std::move (layout)));

    setPrimaryParameter (wetDryParam);
    
    ParameterLinker timeParamLinker;
    
    timeParamLinker.setPrimaryParameter(delayTimeParam);
    timeParamLinker.addParameterToLink(beatParam);
    
    linkParameters(timeParamLinker);
    
    
}

DelayProcessor::~DelayProcessor()
{
    wetDryParam->removeListener (this);
    beatParam->removeListener (this);
    xPadParam->removeListener (this);
    delayTimeParam->removeListener (this);
    feedbackParam->removeListener (this);
}

//============================================================================== Audio processing
void DelayProcessor::prepareToPlay (double Fs, int bufferSize)
{
    const ScopedLock lock (getCallbackLock());
    InsertProcessor::prepareToPlay (Fs, bufferSize);

    delayUnit.setFs ((float) Fs);
    wetDry.reset (Fs, 0.001f);
    delayTime.reset (Fs, 0.001f);
    setRateAndBufferSizeDetails (Fs, bufferSize);
}
void DelayProcessor::processAudioBlock (juce::AudioBuffer<float>& buffer, MidiBuffer&)
{
    //TODO
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    const ScopedLock lock (getCallbackLock());

    float dry, wet, x, y, z;
    for (int s = 0; s < numSamples; ++s)
    {
        delayUnit.setDelaySamples (delayTime.getNextValue());
        wet = wetDry.getNextValue();
        dry = 1.f - wet;
        for (int c = 0; c < numChannels; ++c)
        {
            x = buffer.getWritePointer (c)[s];
            z = delayUnit.processSample (x, c);
            y = (z * wet) + (x * dry);
            delayUnit.processSample (z * feedbackParam->get(), c);
            buffer.getWritePointer (c)[s] = y;
        }
    }
}
//============================================================================== House keeping
const String DelayProcessor::getName() const { return TRANS ("Delay"); }
/** @internal */
Identifier DelayProcessor::getIdentifier() const { return "Delay" + String (idNumber); }
/** @internal */
bool DelayProcessor::supportsDoublePrecisionProcessing() const { return false; }
//============================================================================== Parameter callbacks
void DelayProcessor::parameterValueChanged (int paramNum, float value)
{
    const ScopedLock sl (getCallbackLock());

    InsertProcessor::parameterValueChanged (paramNum, value);
}
void DelayProcessor::releaseResources()
{
    delayUnit.clearDelay();
}
