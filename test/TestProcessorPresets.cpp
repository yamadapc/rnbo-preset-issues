//
// Created by Pedro Tacla Yamada on 8/9/2023.
//

#include "GainAudioProcessor.h"
#include <catch2/catch_test_macros.hpp>

// First, boilerplate to declare unique_ptr processor for our tests
static std::unique_ptr<GainAudioProcessor> createProcessor() {
  return std::unique_ptr<GainAudioProcessor>{
    GainAudioProcessor::createDefault()};
}

// Declare a helper function for getting the state from the processor as a
// parsed JSON object. This will help make tests readable.
static nlohmann::json getJSONState(GainAudioProcessor& gainAudioProcessor) {
  juce::MemoryBlock memoryBlock;
  gainAudioProcessor.getStateInformation(memoryBlock);
  return nlohmann::json::parse(memoryBlock.toString().toStdString());
}

TEST_CASE("GainAudioProcessor updates state after changing preset") {
  juce::MessageManager::getInstance();
  juce::MessageManagerLock messageManagerLock;
  REQUIRE(messageManagerLock.lockWasGained());
  auto gainAudioProcessor = createProcessor();
  auto& rnbo = gainAudioProcessor->getRnboObject();
  auto audioBuffer = juce::AudioBuffer<float>(2, 512);
  auto midiBuffer = juce::MidiBuffer();
  auto gainIndex = rnbo.getParameterIndexForID("gain");

  // Push a block through
  gainAudioProcessor->prepareToPlay(44100, 512);
  gainAudioProcessor->processBlock(audioBuffer, midiBuffer);

  // This being set to -1 is an issue on iOS AUM host -
  // https://github.com/Cycling74/rnbo.example.juce/issues/18
  REQUIRE(gainAudioProcessor->getCurrentProgram() == -1);

  // Get the current state
  auto currentState = getJSONState(*gainAudioProcessor);
  // The current state should have gain set to 1
  nlohmann::json expectedState = {{"__presetid", "rnbo"},
                                  {"gain", {{"value", 1.0}}}};
  REQUIRE(nlohmann::to_string(currentState) ==
          nlohmann::to_string(expectedState));

  SECTION("Confusion 1: Setting the preset does not do anything if audio "
          "processing isn't running (that is okay)") {
    // Let's set the preset to preset '1', which should have gain set to 0
    REQUIRE(gainAudioProcessor->getNumPrograms() == 2);
    REQUIRE(gainAudioProcessor->getProgramName(1) == "Gain = 0");

    // Set the preset
    gainAudioProcessor->setCurrentProgram(1);
    // The preset is SET
    REQUIRE(gainAudioProcessor->getCurrentProgram() == 1);

    // But state is not updated
    auto newState = getJSONState(*gainAudioProcessor);
    nlohmann::json newExpectedState = {{"__presetid", "rnbo"},
                                       {"gain", {{"value", 0.0}}}};
    // This is DIFFERENT but that is expected
    REQUIRE(nlohmann::to_string(newState) !=
            nlohmann::to_string(newExpectedState));
  }

  SECTION("If we process some audio, the preset changes") {
    // Let's set the preset to preset '1', which should have gain set to 0
    REQUIRE(gainAudioProcessor->getNumPrograms() == 2);
    REQUIRE(gainAudioProcessor->getProgramName(1) == "Gain = 0");
    gainAudioProcessor->setCurrentProgram(1);
    REQUIRE(gainAudioProcessor->getCurrentProgram() == 1);

    gainAudioProcessor->processBlock(audioBuffer, midiBuffer);

    auto newState = getJSONState(*gainAudioProcessor);
    nlohmann::json newExpectedState = {{"__presetid", "rnbo"},
                                       {"gain", {{"value", 0.0}}}};
    REQUIRE(nlohmann::to_string(newState) ==
            nlohmann::to_string(newExpectedState));
  }

  SECTION("If we change the state the JSON state is updated") {
    REQUIRE(gainAudioProcessor->getCurrentProgram() == -1);
    auto& parameters = gainAudioProcessor->getParameters();
    parameters[gainIndex]->setValueNotifyingHost(0.4f);
    gainAudioProcessor->processBlock(audioBuffer, midiBuffer);

    auto newState = getJSONState(*gainAudioProcessor);
    REQUIRE(static_cast<float>(newState["gain"]["value"]) - 0.4 < 0.001);
    REQUIRE(static_cast<float>(rnbo.getParameterValue(gainIndex)) - 0.4 <
            0.001);
  }

  SECTION("If we restore a JSON state onto a processor, its state is updated "
          "this seems like a bug, see getParameterValue is not updated") {
    nlohmann::json stateToRestore = {{"__presetid", "rnbo"},
                                     {"gain", {{"value", 0.8}}}};
    REQUIRE(gainAudioProcessor->getCurrentProgram() == -1);

    auto initialState = getJSONState(*gainAudioProcessor);
    REQUIRE(static_cast<float>(initialState["gain"]["value"]) - 1.0 < 0.001);
    REQUIRE(static_cast<float>(rnbo.getParameterValue(gainIndex)) - 1.0 <
            0.001);

    auto stateToRestoreStr = nlohmann::to_string(stateToRestore);
    gainAudioProcessor->setStateInformation(stateToRestoreStr.c_str(),
                                            stateToRestoreStr.length());
    gainAudioProcessor->processBlock(audioBuffer, midiBuffer);

    auto newState = getJSONState(*gainAudioProcessor);
    REQUIRE(static_cast<float>(newState["gain"]["value"]) - 0.8 < 0.001);
    REQUIRE(static_cast<float>(rnbo.getParameterValue(gainIndex)) - 0.8 <
            0.001);
  }
}