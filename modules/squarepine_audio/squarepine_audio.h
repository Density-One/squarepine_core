#ifndef SQUAREPINE_AUDIO_H
#define SQUAREPINE_AUDIO_H

/** BEGIN_JUCE_MODULE_DECLARATION

    ID:                 squarepine_audio
    vendor:             SquarePine
    version:            0.0.1
    name:               SquarePine Audio
    description:        A great backbone for any typical audio project.
    website:            https://www.squarepine.io
    license:            ISC
    minimumCppStandard: 17
    dependencies:       squarepine_core juce_dsp

    END_JUCE_MODULE_DECLARATION
*/
//==============================================================================
#include <squarepine_core/squarepine_core.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/** Config: SQUAREPINE_USE_R8BRAIN

    Enable this to be able use r8brain as an audio resampler.
*/
#ifndef SQUAREPINE_USE_R8BRAIN
#define SQUAREPINE_USE_R8BRAIN 0
#endif

/** Config: SQUAREPINE_USE_ELASTIQUE

    Enable this to use zplane's Elastique.

    Only available with MSVC and macOS.
*/
#ifndef SQUAREPINE_USE_ELASTIQUE
#define SQUAREPINE_USE_ELASTIQUE 0
#endif

/** Config: SQUAREPINE_USE_AUFTAKT
    Enable this to help you use zplane's Auftakt.

    Only available with MSVC and macOS.
*/
#ifndef SQUAREPINE_USE_AUFTAKT
#define SQUAREPINE_USE_AUFTAKT 0
#endif

/** Config: SQUAREPINE_USE_REX_AUDIO_FORMAT

    Enable this to be able to read Propellerheads' REX audio files.

    Only available with MSVC (not MinGW) and macOS (Intel desktop).
*/
#ifndef SQUAREPINE_USE_REX_AUDIO_FORMAT
#define SQUAREPINE_USE_REX_AUDIO_FORMAT 0
#endif

//==============================================================================
// Incomplete support right now...
#undef SQUAREPINE_USE_R8BRAIN

#if SQUAREPINE_USE_R8BRAIN
#include <r8brain/CDSPBlockConvolver.h>
#include <r8brain/CDSPFIRFilter.h>
#include <r8brain/CDSPFracInterpolator.h>
#include <r8brain/CDSPProcessor.h>
#include <r8brain/CDSPRealFFT.h>
#include <r8brain/CDSPResampler.h>
#include <r8brain/CDSPSincFilterGen.h>
#include <r8brain/fft4g.h>
#include <r8brain/HeapFifo.h>
#include <r8brain/r8bbase.h>
#include <r8brain/r8bconf.h>
#include <r8brain/r8butil.h>
#endif

//==============================================================================
#include "linkers/ZplaneHelpers.h"

//==============================================================================
namespace sp
{
using namespace juce;

//==============================================================================
/** Shorthand for creating shared AudioProcessor instances. */
using AudioProcessorPtr = std::shared_ptr<AudioProcessor>;
/** Shorthand for creating weak pointers to AudioProcessor instances. */
using AudioProcessorWeakPtr = std::weak_ptr<AudioProcessor>;

/** Shorthand for creating shared AudioDeviceManager instances. */
using AudioDeviceManagerPtr = std::shared_ptr<AudioDeviceManager>;
/** Shorthand for creating weak pointers to AudioDeviceManager instances. */
using AudioDeviceManagerWeakPtr = std::shared_ptr<AudioDeviceManager>;

//==============================================================================
/** */
String getInternalProcessorTypeName();

//==============================================================================
#include "core/AudioBufferView.h"
#include "core/AudioBufferFIFO.h"
#include "core/AudioUtilities.h"
#include "core/ChildProcessPluginScanner.h"
#include "core/InternalAudioPluginFormat.h"
#include "core/InternalProcessor.h"
#include "core/EffectProcessor.h"
#include "core/EffectProcessorFactory.h"
#include "core/EffectProcessorChain.h"
#include "core/LastKnownPluginDetails.h"
#include "core/MetadataUtilities.h"
#include "core/MIDIChannel.h"
#include "codecs/REXAudioFormat.h"
#include "devices/DummyAudioIODevice.h"
#include "devices/DummyAudioIODeviceCallback.h"
#include "devices/DummyAudioIODeviceType.h"
#include "devices/MediaDevicePoller.h"
#include "dsp/BasicDither.h"
#include "dsp/DistortionFunctions.h"
#include "dsp/EnvelopeFollower.h"
#include "dsp/LFO.h"
#include "dsp/AntiAliasFilter.h"
#include "dsp/DownSampling2Stage.h"
#include "dsp/UpSampling2Stage.h"
#include "dsp/PositionedImpulseResponse.h"
#include "dsp/PitchDelay.h"
#include "dsp/PitchShifter.h"
#include "effects/PhaseIncrementer.h"
#include "effects/ADSRProcessor.h"
#include "effects/BitCrusherProcessor.h"
#include "effects/ChorusProcessor.h"
#include "effects/DitherProcessor.h"
#include "effects/HissingProcessor.h"
#include "effects/JUCEReverbProcessor.h"
#include "effects/LevelsProcessor.h"
#include "effects/LFOProcessor.h"
#include "effects/MuteProcessor.h"
#include "effects/PanProcessor.h"
#include "effects/PolarityInversionProcessor.h"
#include "effects/SimpleDistortionProcessor.h"
#include "effects/SimpleEQProcessor.h"
#include "effects/StereoWidthProcessor.h"
#include "graphics/GraphObserver.h"
#include "graphics/Meter.h"
#include "graphics/ProgramAudioProcessorEditor.h"
#include "music/Chord.h"
#include "music/Genre.h"
#include "music/Pitch.h"
#include "music/Scale.h"
#include "resamplers/Resampler.h"
#include "resamplers/ResamplingAudioFormatReader.h"
#include "resamplers/ResamplingProcessor.h"
#include "resamplers/Stretcher.h"
#include "time/TimeHelpers.h"
#include "time/TimeFormat.h"
#include "time/DecimalTime.h"
#include "time/SMPTETime.h"
#include "time/Tempo.h"
#include "time/TimeSignature.h"
#include "time/MBTTime.h"
#include "time/TimeKeeper.h"
#include "wrappers/AudioSourceProcessor.h"
#include "wrappers/AudioTransportProcessor.h"
//==============================================================================
#include "effects/daweffects/reverb-sinevibes/DampingFilter.h"
#include "effects/daweffects/reverb-sinevibes/InlineMath.h"
#include "effects/daweffects/reverb-sinevibes/ChaosLFO.h"
#include "effects/daweffects/reverb-sinevibes/LowCutFilter.h"
#include "effects/daweffects/reverb-sinevibes/SimpleDelay.h"
#include "effects/daweffects/reverb-sinevibes/SimpleLFO.h"
#include "effects/daweffects/reverb-sinevibes/SinCosTable.h"
#include "effects/daweffects/reverb-sinevibes/MatrixReverb8x8.h"
#include "effects/daweffects/reverb-sinevibes/MatrixReverb4x4.h"
//==============================================================================

#include "effects/daweffects/NotifiableAudioParameterFloat.h"
#include "effects/daweffects/NotifiableAudioParameterBool.h"
#include "effects/daweffects/SwitchableTimeParameter.h"
#include "effects/daweffects/GainProcessor.h"
#include "effects/daweffects/SEMFilter.h"
#include "effects/daweffects/CrossoverFilter.h"
#include "effects/daweffects/BandProcessor.h"
#include "effects/daweffects/DelayProcessor.h"
#include "effects/daweffects/CrushProcessor.h"
#include "effects/daweffects/InsertProcessor.h"
#include "effects/daweffects/EchoProcessor.h"
#include "effects/daweffects/PingPongProcessor.h"
#include "effects/daweffects/SpiralProcessor.h"
#include "effects/daweffects/ShimmerProcessor.h"
#include "effects/daweffects/ReverbProcessor.h"
#include "effects/daweffects/LFOFilterProcessor.h"
#include "effects/daweffects/FlangerProcessor.h"
#include "effects/daweffects/PhaserProcessor.h"
#include "effects/daweffects/PitchProcessor.h"
#include "effects/daweffects/SlipRollProcessor.h"
#include "effects/daweffects/RollProcessor.h"
#include "effects/daweffects/TransEffectProcessor.h"
#include "effects/daweffects/HelixProcessor.h"
#include "effects/daweffects/VinylBreakProcessor.h"
#include "effects/daweffects/BitCrusherProcessor.h"
#include "effects/daweffects/NoiseProcessor.h"
#include "effects/daweffects/SweepProcessor.h"
#include "effects/daweffects/V10SendProcessor.h"
#include "effects/daweffects/ShortDelayProcessor.h"
#include "effects/daweffects/LongDelayProcessor.h"
#include "effects/daweffects/DubEchoProcessor.h"
#include "effects/daweffects/SpaceProcessor.h"
#include "effects/daweffects/EffectiveTempoProcessor.h"
#include "effects/daweffects/VariableBPMProcessor.h"
#include "effects/daweffects/EQProcessor.h"
#include "effects/daweffects/TruePeakAnalysis.h"
#include "effects/daweffects/LimiterProcessor.h"

}

//==============================================================================
namespace juce
{
using namespace sp;

//==============================================================================
/** */
template<>
struct VariantConverter<MeteringMode>
{
    /** */
    static MeteringMode fromVar (const var& v) { return (MeteringMode) static_cast<int> (v); }
    /** */
    static var toVar (MeteringMode mode) { return static_cast<int> (mode); }
};

//==============================================================================
/** */
template<>
struct VariantConverter<TimeSignature> final
{
    /** */
    static TimeSignature fromVar (const var& v)
    {
        if (auto* data = v.getArray())
        {
            if (data->size() == 2)
            {
                return { static_cast<int> (data->getFirst()),
                         static_cast<int> (data->getLast()) };
            }
        }

        jassertfalse;
        return {};
    }

    /** @returns the following as JSON compatible array notation:
            @code
                [ 4, 4 ]
            @endcode
        */
    static var toVar (const TimeSignature& t)
    {
        return Array<var> { t.numerator, t.denominator };
    }
};

//==============================================================================
/** */
template<>
struct VariantConverter<Tempo> final
{
    /** */
    static Tempo fromVar (const var& v) noexcept { return Tempo (static_cast<double> (v)); }
    /** */
    static var toVar (const Tempo& t) noexcept { return t.get(); }
};

//==============================================================================
/** */
template<>
struct VariantConverter<MIDIChannel> final
{
    /** */
    static MIDIChannel fromVar (const var& v) noexcept { return MIDIChannel (static_cast<int> (v)); }
    /** */
    static var toVar (const MIDIChannel& mc) noexcept { return mc.get(); }
};

//==============================================================================
/** */
template<>
struct VariantConverter<PluginDescription> final
{
    /** */
    static PluginDescription fromVar (const var& v)
    {
        if (v.isString())
        {
            MemoryOutputStream mos;
            if (Base64::convertFromBase64 (mos, v.toString()))
            {
                if (auto xml = parseXML (mos.toString()))
                {
                    PluginDescription pd;
                    if (pd.loadFromXml (*xml))
                        return pd;

                    jassertfalse;
                }
                else
                {
                    jassertfalse;
                }
            }
            else
            {
                jassertfalse;
            }
        }
        else
        {
            jassertfalse;
        }

        return {};
    }

    /** @returns a PluginDescription as Base64 XML data. */
    static var toVar (const PluginDescription& pd)
    {
        auto xml = pd.createXml();
        jassert (xml != nullptr);
        return Base64::toBase64 (xml->toString());
    }
};
}

#endif//SQUAREPINE_AUDIO_H
