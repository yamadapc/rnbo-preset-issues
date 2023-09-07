//
// Created by Pedro Tacla Yamada on 8/9/2023.
//

#include "GainAudioProcessor.h"
#include "rnbo_description.h"

// create an instance of our custom plugin, optionally set description, presets
// and binary data (datarefs)
GainAudioProcessor* GainAudioProcessor::createDefault() {
  nlohmann::json patcher_desc, presets;

#ifdef RNBO_BINARY_DATA_STORAGE_NAME
  extern RNBO::BinaryDataImpl::Storage RNBO_BINARY_DATA_STORAGE_NAME;
  RNBO::BinaryDataImpl::Storage dataStorage = RNBO_BINARY_DATA_STORAGE_NAME;
#else
  RNBO::BinaryDataImpl::Storage dataStorage;
#endif
  RNBO::BinaryDataImpl data(dataStorage);

#ifdef RNBO_INCLUDE_DESCRIPTION_FILE
  patcher_desc = RNBO::patcher_description;
  presets = RNBO::patcher_presets;
#endif
  juce::Logger::writeToLog("Created PD1 processor");

  return new GainAudioProcessor(patcher_desc, presets, data);
}

GainAudioProcessor::GainAudioProcessor(const nlohmann::json& patcherDescription,
                                       const nlohmann::json& presets,
                                       const RNBO::BinaryData& data)
    : RNBO::JuceAudioProcessor(patcherDescription, presets, data) {
}