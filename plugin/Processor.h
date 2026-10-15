// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "StaticEQ.h"
#include "Spectrum.h"
#include "SpectralEngine.h"
#include "PhaseEngine.h"

class EQProcessor final : public juce::AudioProcessor, private juce::AudioProcessorValueTreeState::Listener, private juce::AsyncUpdater {
public:
    EQProcessor();
    ~EQProcessor() override;
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Aester EQ"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 2; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    void updateTrackProperties(const TrackProperties& props) override {if(props.name){const juce::ScopedLock lock(trackLock);trackName=*props.name;}}
    juce::String instanceName() const {const juce::ScopedLock lock(trackLock);return trackName;}
    static juce::String bandID(int band, const char* field);
    distillazione::BandSettings settings(int band) const;
    bool phaseReady()const{return phase.isReady();}
    bool splitBand(int);
    void copyBand(int,const distillazione::BandSettings&);
    std::array<std::atomic<float>,24> dynamicAmount{};
    std::array<std::atomic<float>,24> detectorLevel{};
    float spectralResponseAt(double hz) const {return spectralModes.load()!=0?spectral.responseAt(hz):0;}
    void setValue(const juce::String&, float);
    juce::AudioProcessorValueTreeState parameters;
    SpectrumTap spectrum,sidechainSpectrum;
    std::atomic<float> inputPeak{0}, outputPeak{0};
    std::array<std::atomic<float>,2> channelOutputPeak{},meterPeak{};
private:
    mutable juce::CriticalSection trackLock;
    juce::String trackName{"Aester EQ"};
    void parameterChanged(const juce::String&, float) override;
    void handleAsyncUpdate() override;
    void refreshProcessingMode();
    std::atomic<unsigned> spectralModes{0};
    std::atomic<int> processingLatency{0};
    bool spectralWasActive=false;
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    struct Raw {
        std::atomic<float> *on,*shape,*frequency,*gain,*q,*slope,*mode,*placement,*threshold,*range,*attack,*release,*external,*freeTrigger,*triggerLow,*triggerHigh;
    };
    struct Runtime {
        std::array<std::array<distillazione::DigitalSectionState,8>,2> states;
        std::array<std::array<distillazione::DigitalSectionState,8>,2> previousStates;
        distillazione::SectionBank coefficients;
        distillazione::SectionBank previousCoefficients;
        juce::SmoothedValue<double,juce::ValueSmoothingTypes::Multiplicative> frequency,q;
        juce::SmoothedValue<double> gain,wet,shapeMix;
        std::array<distillazione::DigitalSectionState,2> detectorStates{};
        std::array<std::array<distillazione::DigitalSectionState,8>,2> dynamicStates{};
        distillazione::DigitalSection detector{};
        std::array<std::array<distillazione::DigitalSectionState,2>,2> freeStates{};
        distillazione::DigitalSection triggerLowFilter{},triggerHighFilter{};
        bool freeTrigger=false;
        double triggerLow=20,triggerHigh=20000;
        distillazione::SectionBank dynamicFilter{};
        double envelope=0,change=0,attackCoefficient=0,releaseCoefficient=0,threshold=-24,range=-6;
        int mode=0,placement=0;
        bool wasActive=false;
        bool external=false;
        int shape=0,slope=1;
    };
    SpectralEngine spectral;
    PhaseEngine phase;
    std::array<Raw,24> raw;
    std::array<Runtime,24> bands;
    std::atomic<float> *outputRaw,*bypassRaw,*balanceRaw,*balanceModeRaw,*invertRaw,*phaseRaw,*resolutionRaw,*mixRaw;
    juce::SmoothedValue<double> output, bypass,balance,polarity,blend;
    double rate=48000;
    int coefficientTick=0,lastPhaseMode=0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQProcessor)
};
