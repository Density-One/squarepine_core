namespace djdawprocessor
{

/** Use this processor to scale the gain of incoming audio samples. */
class GainProcessor final : public InternalProcessor,
                            public AudioProcessorParameter::Listener
{
public:
    //==============================================================================
    /** This is the default maximum gain:
     a gain of 2.0f, being approxmiately 6 dB.
     
     And, though it should be obvious, the default minimum is 0.0f.
     */
    static constexpr auto defaultMaximumGainLinear = 2.0f;
    static constexpr auto defaultMaximumGaindB = 6.0f;
    static constexpr auto defaultMinimumGaindB = -96.f;

    //==============================================================================
    /** Constructor.
     
     @param gainRange The gain range the parameter can have.
     By default, this goes from 0.0f to defaultMaximumGainLinear.
     
     @see defaultMaximumGainLinear
     */
    GainProcessor (int idNum = 1,
                   const String& parameterName = "Gain",
                   NormalisableRange<float> gainRange = { defaultMinimumGaindB, defaultMaximumGaindB, 0.01f, 6.f });

    //==============================================================================
    /** Changes the gain.
     
     A value of 1.0 means no gain change.
     To boost, pass in values beyond 1.0.
     To cut, pass in values under 1.0.
     
     @param newGain New gain amount.
     
     @see maximumGain
     */
    void setGain (float newGain);

    /** @returns the current gain. */
    float getGain() const noexcept;

    /** @returns the minimum gain this gain processor supports.
     
     @see NormalisableRange
     */
    float getMinimumGain() const noexcept;

    /** @returns the maximum gain this gain processor supports.
     
     @see NormalisableRange
     */
    float getMaximumGain() const noexcept;

    //==============================================================================
    /** @internal */
    const String getName() const override { return TRANS (name); }
    /** @internal */
    Identifier getIdentifier() const override { return "gain" + String (idNumber); }
    /** @internal */
    bool supportsDoublePrecisionProcessing() const override { return true; }
    /** @internal */
    void prepareToPlay (double, int) override;
    /** @internal */
    void processBlock (juce::AudioBuffer<float>&, MidiBuffer&) override;
    /** @internal */
    void processBlock (juce::AudioBuffer<double>&, MidiBuffer&) override;
    /** @internal */
    void parameterValueChanged (int, float) override;
    /** @internal */
    void parameterGestureChanged (int, bool) override;
private:
    //==============================================================================
    NotifiableAudioParameterFloat* gainParameter = nullptr;
    String name;
    LinearSmoothedValue<float> floatGain { 1.0f };
    LinearSmoothedValue<double> doubleGain { 1.0 };
    const int idNumber = 1;
    //==============================================================================
    template<typename FloatType>
    void process (juce::AudioBuffer<FloatType>& buffer,
                  LinearSmoothedValue<FloatType>& value);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainProcessor)
};

}
