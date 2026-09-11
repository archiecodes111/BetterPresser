#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../DSP/CleanCompressor.h"

namespace betterpresser
{

class CompressionGraph : public juce::Component
{
public:
    explicit CompressionGraph(CleanCompressor& compressorRef);
    ~CompressionGraph() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void updateGraph();

private:
    CleanCompressor& compressor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressionGraph)
};

} // namespace betterpresser
