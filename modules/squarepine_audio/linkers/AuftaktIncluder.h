//==============================================================================
#if SQUAREPINE_USE_AUFTAKT

namespace zplane
{

//==============================================================================
inline void logAuftaktInfo()
{
    auto newLine = juce::newLine;

    juce::String info;
    info
        << newLine
        << "--------------------------------------------------" << newLine << newLine
        << "=== Auftakt Information ===" << newLine << newLine
        << "Build Date: " << zplane::aufTAKT::getBuildDate() << newLine
        //<< "Version: " << CaufTAKT_If::GetVersion () << newLine
        << newLine
        << newLine
        << "--------------------------------------------------" << newLine;

    juce::Logger::writeToLog (info);
}
}

#endif
