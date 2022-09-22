#if SQUAREPINE_USE_CUESDK

/** */
class CueSDKDemo final : public DemoBase
{
public:
    /** */
    CueSDKDemo (SharedObjects& sharedObjs) :
        DemoBase (sharedObjs, NEEDS_TRANS ("CueSDK Demo"))
    {
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CueSDKDemo)
};

#endif // SQUAREPINE_USE_CUESDK
