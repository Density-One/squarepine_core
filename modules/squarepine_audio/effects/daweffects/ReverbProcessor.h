namespace djdawprocessor
{
/// wet/dry is true blend (increase + decrease)
/// Time is listed as percentage, 50% default.
/// Reverb sounds fairly bright (X-pad adds LPF for low settings and HPF for high settings)

class ReverbProcessor final : public BandProcessor
{
public:
    //Constructor with ID
    ReverbProcessor (int idNum = 1);
    ~ReverbProcessor() override;

    //============================================================================== Audio processing
    void prepareToPlay (double Fs, int bufferSize) override;
    void processAudioBlock (juce::AudioBuffer<float>& buffer, MidiBuffer&) override;
    /** @internal */
    void releaseResources() override;
    //============================================================================== House keeping
    const String getName() const override;
    /** @internal */
    Identifier getIdentifier() const override;
    /** @internal */
    bool supportsDoublePrecisionProcessing() const override;
    //============================================================================== Parameter callbacks
    void parameterValueChanged (int paramNum, float value) override;
    void parameterGestureChanged (int, bool) override {}
private:
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    NotifiableAudioParameterFloat* filterParam = nullptr;
    NotifiableAudioParameterFloat* timeParam = nullptr;
    NotifiableAudioParameterFloat* wetDryParam = nullptr;
    AudioParameterBool* fxOnParam = nullptr;

    NotifiableAudioParameterFloat* decayParam = nullptr;
    NotifiableAudioParameterFloat* sizeParam = nullptr;
    NotifiableAudioParameterFloat* preDelayParam = nullptr;
    NotifiableAudioParameterFloat* modFrequencyParam = nullptr;
    NotifiableAudioParameterFloat* modDepthParam = nullptr;
    NotifiableAudioParameterFloat* lowDampParam = nullptr;
    NotifiableAudioParameterFloat* highDampParam = nullptr;

    MatrixReverb8x8 matrixReverb;
    AudioBuffer<float> reverbInputBuffer;
    AudioBuffer<float> reverbOutputBuffer;

    std::vector<float> preDelayVector, sizeVector, decayVector, modFrequencyVector, modDepthVector, lowDampVector, highDampVector;


    void updateReverbParams (int numSamples);
    int maxBlockSize = 16384 * 4;

    int idNumber = 1;
    DigitalFilter hpf;
    DigitalFilter lpf;
};

}
