/** Use this processor to add periodic hissing over the provided audio. */
class HissingProcessor final : public InternalProcessor
{
public:
    /** Constructor */
    HissingProcessor() = default;

    //==============================================================================
    /** @internal */
    Identifier getIdentifier() const override { return NEEDS_TRANS ("Periodic Hisser"); }
    /** @internal */
    void prepareToPlay (double, int) override;
    /** @internal */
    void processBlock (juce::AudioBuffer<float>&, MidiBuffer&) override;

private:
    //==============================================================================
    int blockCounter = 0, blocksBetweenHisses = 0, maxBlocksBetweenHisses = 0, blocksPerHiss = 0;
    double level = 0;
    juce::Random random;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HissingProcessor)
};

