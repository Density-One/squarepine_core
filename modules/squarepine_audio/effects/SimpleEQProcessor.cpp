//==============================================================================
class SimpleEQProcessor::InternalFilter final : public AudioProcessorParameter::Listener
{
public:
    InternalFilter (FilterType filterType,
                    NotifiableAudioParameterFloat* g,
                    NotifiableAudioParameterFloat* c,
                    NotifiableAudioParameterFloat* r) :
        type (filterType),
        gain (g),
        cutoff (c),
        resonance (r)
    {
        updateParams();

        gain->addListener (this);
        cutoff->addListener (this);
        resonance->addListener (this);
    }

    //==============================================================================
    void prepare (double newSampleRate, int bufferSize, int numChannels)
    {
        sampleRate = newSampleRate;

        const dsp::ProcessSpec spec
        {
            sampleRate,
            (uint32) jmax (0, bufferSize),
            (uint32) jmax (0, numChannels)
        };

        floatProcessor.prepare (spec);
        doubleProcessor.prepare (spec);

        updateParams();
    }

    void reset()
    {
        floatProcessor.reset();
        doubleProcessor.reset();
    }

    void process (juce::AudioBuffer<float>& buffer, bool wholeProcIsBypassed)
    {
        process (floatProcessor, buffer, wholeProcIsBypassed);
    }

    void process (juce::AudioBuffer<double>& buffer, bool wholeProcIsBypassed)
    {
        process (doubleProcessor, buffer, wholeProcIsBypassed);
    }

    void updateParams()
    {
        updateParamsFor (floatProcessor);
        updateParamsFor (doubleProcessor);
    }

    void parameterValueChanged (int, float) override    { updateParams(); }
    void parameterGestureChanged (int, bool) override   { }

    //==============================================================================
    const FilterType type = FilterType::bandpass;
    double sampleRate = 44100.0;
    ProcessorDuplicator<float> floatProcessor;
    ProcessorDuplicator<double> doubleProcessor;

    // NB: These are owned by the parent EQ processor.
    NotifiableAudioParameterFloat* gain = nullptr;
    NotifiableAudioParameterFloat* cutoff = nullptr;
    NotifiableAudioParameterFloat* resonance = nullptr;

private:
    //==============================================================================
    template<typename SampleType>
    void updateParamsFor (ProcessorDuplicator<SampleType>& processor)
    {
        const auto g = (SampleType) gain->get();
        const auto c = (SampleType) cutoff->get();
        const auto r = (SampleType) resonance->get();

        using Coeffs = Coefficients<SampleType>;
        typename Coeffs::Ptr coeffs;

        switch (type)
        {
            case FilterType::lowpass:   coeffs = Coeffs::makeLowShelf (sampleRate, c, r, g); break;
            case FilterType::highpass:  coeffs = Coeffs::makeHighShelf (sampleRate, c, r, g); break;
            case FilterType::bandpass:  coeffs = Coeffs::makePeakFilter (sampleRate, c, r, g); break;

            default:
                jassertfalse;
            break;
        };

        *processor.state = *coeffs;
    }

    template<typename SampleType>
    void process (ProcessorDuplicator<SampleType>& processor,
                  juce::AudioBuffer<SampleType>& buffer,
                  bool wholeProcIsBypassed)
    {
        dsp::AudioBlock<SampleType> block (buffer);
        Context<SampleType> context (block);
        context.isBypassed = wholeProcIsBypassed;

        processor.process (context);
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InternalFilter)
};

//==============================================================================
SimpleEQProcessor::SimpleEQProcessor() :
    InternalProcessor (false)
{
    apvts.reset (new AudioProcessorValueTreeState (*this, nullptr, "parameters", createParameterLayout()));
}

SimpleEQProcessor::~SimpleEQProcessor()
{
}

//==============================================================================
void SimpleEQProcessor::prepareToPlay (const double newSampleRate, const int bufferSize)
{
    setRateAndBufferSizeDetails (newSampleRate, bufferSize);

    const auto numChans = jmax (getTotalNumInputChannels(), getTotalNumOutputChannels());

    const ScopedLock sl (getCallbackLock());

    for (auto* f : filters)
        f->prepare (newSampleRate, bufferSize, numChans);
}

void SimpleEQProcessor::processBlock (juce::AudioBuffer<float>& buffer, MidiBuffer&)    { process (buffer); }
void SimpleEQProcessor::processBlock (juce::AudioBuffer<double>& buffer, MidiBuffer&)   { process (buffer); }

template<typename SampleType>
void SimpleEQProcessor::process (juce::AudioBuffer<SampleType>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    const bool isWholeProcBypassed = isBypassed()
                                  || buffer.hasBeenCleared()
                                  || numChannels <= 0
                                  || numSamples <= 0;

    const ScopedLock sl (getCallbackLock());

    for (auto* f : filters)
        f->process (buffer, isWholeProcBypassed);
}

//==============================================================================
AudioProcessorValueTreeState::ParameterLayout SimpleEQProcessor::createParameterLayout()
{
    struct Config final
    {
        String name;
        FilterType type = FilterType::bandpass;
        float frequency = 0.0f;
    };

    const Config configs[] =
    {
        { NEEDS_TRANS ("HighShelf"), FilterType::highpass, 13000 },
        { NEEDS_TRANS ("BandPass"), FilterType::bandpass, 1200 },
        { NEEDS_TRANS ("LowShelf"), FilterType::lowpass, 70 }
    };

    filters.ensureStorageAllocated (numElementsInArray (configs));

    auto layout = createDefaultParameterLayout();
    const std::function<float(float, float, float)> from0To1 = [] (float start, float end, float value) -> float
    {
        if (value < 0.5f)
            return (0.5f - value) / 0.5f * start;
    
        return (value - 0.5f) / 0.5f * end;
    };
    
    const std::function<float(float, float, float)> to0To1 = [] (float start, float end, float db) -> float
    {
        if (db < 0.0f)
            return (1.0f + db / start) * 0.5f;
    
        return (db / end) * 0.5f + 0.5f;
    };
    
    for (const auto& c : configs)
    {
        const auto minimum = c.type == FilterType::bandpass ? -25.0f : -100.0f;
        const auto maximum = c.type == FilterType::bandpass ? 10.0f : 6.0f;
        NormalisableRange<float> decibelRange =
        {   minimum,
            maximum,
            from0To1,
            to0To1
        };
        auto gain = std::make_unique<NotifiableAudioParameterFloat> (String ("gainXYZ").replace ("XYZ", c.name),
                                                                     TRANS ("Gain (XYZ)").replace ("XYZ", TRANS (c.name)),
                                                                     decibelRange,
                                                                     0.0f, // default db
                                                                     true,
                                                                     getName(),
                                                                     AudioProcessorParameter::genericParameter,
                                                                     [] (float value, int) -> String
                                                                     {
                                                                          if (approximatelyEqual (value, 0.0f))
                                                                              return "0 dB";

                                                                          return Decibels::toString (value);
                                                                     });

        auto cutoff = std::make_unique<NotifiableAudioParameterFloat> (String ("cutoffXYZ").replace ("XYZ", c.name),
                                                                       TRANS ("Cutoff (XYZ)").replace ("XYZ", TRANS (c.name)),
                                                                       20.0f,
                                                                       20000.0f,
                                                                       c.frequency,
                                                                       false);

        auto resonance = std::make_unique<NotifiableAudioParameterFloat> (String ("qXYZ").replace ("XYZ", c.name),
                                                                          TRANS ("Q (XYZ)").replace ("XYZ", TRANS (c.name)),
                                                                          0.00001f,
                                                                          10.0f,
                                                                          1.0f / MathConstants<float>::sqrt2,
                                                                          false);

        filters.add (new InternalFilter (c.type, gain.get(), cutoff.get(), resonance.get()));

        layout.add (std::move (gain));
        layout.add (std::move (cutoff));
        layout.add (std::move (resonance));
    }

    filters.minimiseStorageOverheads();

    return layout;
}
