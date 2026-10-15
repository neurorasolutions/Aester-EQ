// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#pragma once
#include "Processor.h"
#include <optional>
#include <juce_gui_extra/juce_gui_extra.h>

class AesterLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    AesterLookAndFeel();
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
    void drawToggleButton(juce::Graphics&,juce::ToggleButton&,bool,bool) override;
    void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawLinearSlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider::SliderStyle,juce::Slider&) override;
};

// Relative drags use a fixed coordinate origin even when the panel follows the band.
class FollowingPanelSlider final : public juce::Slider {
public:
    void mouseDown(const juce::MouseEvent& e) override {
        dragOrigin=getPosition().toFloat();juce::Slider::mouseDown(e);
    }
    void mouseDrag(const juce::MouseEvent& e) override {
        juce::Slider::mouseDrag(e.withNewPosition(e.position+getPosition().toFloat()-dragOrigin));
    }
private:
    juce::Point<float> dragOrigin;
};

class RangeRingSlider final : public juce::Slider {
public:
    bool hitTest(int x,int y) override {if(getSliderStyle()!=juce::Slider::RotaryHorizontalVerticalDrag)return juce::Slider::hitTest(x,y);const auto r=std::hypot(float(x)-getWidth()*.5f,float(y)-getHeight()*.5f);return r>getWidth()*.38f&&r<getWidth()*.51f;}
};

class EQEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit EQEditor(EQProcessor&);
    ~EQEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void updateAnalysis(){updateMeters();if(!analyzerFrozen){spectrum.update(eqProcessor.spectrum);if(showSidechain)sidechainDisplay.update(eqProcessor.sidechainSpectrum);}threshold.getProperties().set("detectorDb",eqProcessor.detectorLevel[size_t(selected)].load());repaint();}
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&) override;
private:
    class BandCard;
    std::array<std::unique_ptr<BandCard>,24> bandCards;
    int bandPage=0;
    juce::TextButton previousPage{"<"},nextPage{">"};
    juce::Label pageLabel;
    class OutputPanel final:public juce::Component {public:void paint(juce::Graphics& g)override{g.setColour(juce::Colour(0xff25212d));g.fillRoundedRectangle(getLocalBounds().toFloat(),9);g.setColour(juce::Colour(0xff68606e));g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1),9,1);}};
    OutputPanel outputPanel;
    juce::Slider outputKnob,outputBalance;
    juce::ComboBox outputBalanceMode;
    juce::ToggleButton outputInvert{"INV"};
    juce::Label outputGainLabel,outputPanLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputKnobAttachment,outputBalanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> outputBalanceModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> outputInvertAttachment;
    juce::TooltipWindow tooltips{this,650};
    bool outputSticky=false;
    void updateMeters();
    std::array<float,2> meterDisplayDb{-100,-100};
    double lastMeterTime=0;
    AesterLookAndFeel style;
    EQProcessor& eqProcessor;
    juce::ComboBox bandPicker,shape,slope,mode,placement;
    juce::ToggleButton externalKey{"EXT"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> externalAttachment;
    juce::ToggleButton active{"Active"},bypass{"Bypass"};
    SpectrumDisplay spectrum,sidechainDisplay;
    juce::TextButton freeze{"Freeze"};
    bool analyzerFrozen=false;
    juce::Slider wetMix;
    juce::Label wetMixLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    bool showSidechain=false;
    int analyzerMode=3;
    float analyzerTilt=4.5f;
    juce::TextButton processingButton{"Live"},resolutionButton{"Medium"};
    juce::TextButton slopeButton,triggerButton{"Band"};
    juce::TextButton analyzer{"Analyzer: Pre+Post"},shapeButton,modeSettings{""},nodeActions{"v"};
    juce::TextButton range{"12 dB"},undo{""},redo{""},ab{"A/B"},copyAB{"Copy"},presetMenu{"Default Setting"},help{"Help"},resizeEditor{"[]"};
    bool largeEditor=false;
    std::vector<std::vector<float>> history;
    std::vector<float> stateA,stateB,observedState;
    int historyPosition=0,stableTicks=0;
    bool editingB=false;
    std::vector<float> captureState() const;
    void restoreState(const std::vector<float>&);
    void commitHistory();
    void showPresetMenu();
    juce::TextButton add{"Add band"},remove{"Remove"},reset{"Reset EQ"},save{"Save preset"},load{"Load preset"};
    FollowingPanelSlider frequency;
    juce::Slider gain,q,output,threshold,attack,release;
    RangeRingSlider dynamicRange;
    juce::Label thresholdLabel,rangeLabel,attackLabel,releaseLabel;
    bool expanded=false;
    juce::Label frequencyLabel,gainLabel,qLabel,shapeLabel,slopeLabel,outputLabel;
    using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> frequencyAttachment,gainAttachment,qAttachment,outputAttachment;
    std::unique_ptr<ComboAttachment> shapeAttachment,slopeAttachment;
    std::unique_ptr<ButtonAttachment> activeAttachment,bypassAttachment;
    std::unique_ptr<SliderAttachment> thresholdAttachment,rangeAttachment,attackAttachment,releaseAttachment;
    std::unique_ptr<ComboAttachment> modeAttachment,placementAttachment;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::Rectangle<float> plot,panel;
    float displayRange=12;
    int selected=0;
    bool dragging=false;
    std::optional<distillazione::BandSettings> copiedBand;
    juce::Image shapeIcon(int) const;
    juce::PopupMenu shapeMenu(int baseID) const;
    void showShapeMenu();
    void showBandMenu();
    void updateNodeAction();
    void select(int);
    void addBand(float,float);
    void preset(bool saving);
    void timerCallback() override;
    float xFor(double) const;
    float yFor(double) const;
    double hzFor(float) const;
    double dbFor(float) const;
    juce::Colour colour(int) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQEditor)
};
