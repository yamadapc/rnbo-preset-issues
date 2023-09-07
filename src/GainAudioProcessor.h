//
// Created by Pedro Tacla Yamada on 8/9/2023.
//

#pragma once

#include <RNBO.h>
#include <RNBO_JuceAudioProcessor.h>
#include <juce_audio_processors/juce_audio_processors.h>

class GainAudioProcessor : public RNBO::JuceAudioProcessor {
public:
  GainAudioProcessor(const nlohmann::json& patcherDescription,
                     const nlohmann::json& presets,
                     const RNBO::BinaryData& data);

  static GainAudioProcessor* createDefault();

private:
};
