// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "Editor.h"
#include <cmath>

namespace {
AesterLookAndFeel& menuStyle(){static AesterLookAndFeel look;return look;}
const juce::StringArray shapeNames{"Bell","Low Shelf","Low Cut","High Shelf","High Cut","Notch","Band Pass","Tilt Shelf","Flat Tilt","All Pass"};
juce::Image curveIcon(int shape,juce::Colour colour) {
    juce::Image image(juce::Image::ARGB,64,32,true);juce::Graphics g(image);juce::Path path;
    for(int i=0;i<=60;++i){double x=double(i)/60,y=0,t=(x-.5)*6;
        switch(shape){case 0:y=std::exp(-t*t)*.8;break;case 1:y=.65/(1+std::exp(t*3));break;case 2:y=-.4*std::log1p(std::exp(-t*3));break;case 3:y=.65/(1+std::exp(-t*3));break;case 4:y=-.4*std::log1p(std::exp(t*3));break;case 5:y=-.95*std::exp(-t*t*5);break;case 6:y=-.85*(1-std::exp(-t*t));break;case 7:case 8:y=-.65*std::tanh(t);break;case 9:y=.7*std::sin(x*juce::MathConstants<double>::twoPi);break;default:break;}
        float px=2+float(i),py=18-float(y)*13;if(i==0)path.startNewSubPath(px,py);else path.lineTo(px,py);
    }g.setColour(colour);g.strokePath(path,juce::PathStrokeType(1.8f));return image;
}
}
void AesterLookAndFeel::drawToggleButton(juce::Graphics& g,juce::ToggleButton& button,bool over,bool down) {
    if(button.getButtonText().isNotEmpty()){LookAndFeel_V4::drawToggleButton(g,button,over,down);return;}
    float cx=button.getWidth()*.5f,cy=button.getHeight()*.5f;juce::Path arc;arc.addCentredArc(cx,cy,6,6,0,.55f,juce::MathConstants<float>::twoPi-.55f,true);
    g.setColour(button.getToggleState()?juce::Colour(0xffdfff00):juce::Colour(0xff88818d));g.strokePath(arc,juce::PathStrokeType(1.3f));g.drawLine(cx,cy-8,cx,cy-1,1.3f);
}
void AesterLookAndFeel::drawButtonText(juce::Graphics& g,juce::TextButton& button,bool over,bool down) {
    if(button.getProperties().contains("icon")){auto icon=button.getProperties()["icon"].toString();auto cx=button.getWidth()*.5f,cy=button.getHeight()*.5f;g.setColour(button.isEnabled()?juce::Colour(0xffbab3bf):juce::Colour(0xff625c68));if(icon=="settings"){for(int i=-1;i<=1;++i)g.fillEllipse(cx+float(i*5)-1.5f,cy-1.5f,3,3);}else{bool reverse=icon=="redo";const float direction=reverse?1.0f:-1.0f;juce::Path path;path.startNewSubPath(cx-direction*6,cy+6);path.cubicTo(cx-direction*11,cy-7,cx+direction*4,cy-10,cx+direction*8,cy-3);g.strokePath(path,juce::PathStrokeType(1.8f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));juce::Path arrow;arrow.startNewSubPath(cx+direction*8,cy-3);arrow.lineTo(cx+direction*1,cy-4);arrow.lineTo(cx+direction*7,cy-10);arrow.closeSubPath();g.fillPath(arrow);}return;}
    if(!button.getProperties().contains("shape")){LookAndFeel_V4::drawButtonText(g,button,over,down);return;}
    g.drawImageWithin(curveIcon(int(button.getProperties()["shape"]),juce::Colour(0xffbdb7c5)),4,4,27,20,juce::RectanglePlacement::centred);
    g.setColour(juce::Colour(0xffd5cfd9));g.setFont(juce::FontOptions(10));g.drawText(button.getButtonText(),35,1,button.getWidth()-38,button.getHeight()-2,juce::Justification::left);
}
AesterLookAndFeel::AesterLookAndFeel() {
    setColour(juce::Slider::trackColourId,juce::Colour(0xff425d59));
    setColour(juce::Slider::thumbColourId,juce::Colour(0xffdfff00));
    setColour(juce::Slider::backgroundColourId,juce::Colour(0xff23383a));
    setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff142225));
    setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff23383a));
    setColour(juce::ComboBox::outlineColourId,juce::Colours::transparentBlack);
    setColour(juce::ComboBox::textColourId,juce::Colour(0xffdedde0));
    setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff1b2c2d));
    setColour(juce::TextButton::textColourOffId,juce::Colour(0xffd0cdd3));
    setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffe8e4e9));
    setColour(juce::ToggleButton::textColourId,juce::Colour(0xffc9c4ce));
    setColour(juce::ToggleButton::tickColourId,juce::Colour(0xffdfff00));
}
void AesterLookAndFeel::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool over,bool down) {
    auto r=b.getLocalBounds().toFloat().reduced(1);
    if(bool(b.getProperties()["toolbar"])){if(over||down){g.setColour(juce::Colour(0xff363039));g.fillRoundedRectangle(r,4);}return;}
    g.setColour(juce::Colour(b.getToggleState()?0xffdfff00:down?0xff454149:over?0xff343038:0xff242127));g.fillRoundedRectangle(r,4);
    g.setColour(juce::Colour(0xff49434b));g.drawRoundedRectangle(r,4,0.7f);
}
void AesterLookAndFeel::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider& slider) {
    auto r=juce::Rectangle<float>(float(x),float(y),float(w),float(h)).reduced(7);
    if(bool(slider.getProperties()["rangeRing"])){
        const float radius=std::min(r.getWidth(),r.getHeight())*.5f,cx=r.getCentreX(),cy=r.getCentreY();
        juce::Path rail,arc;rail.addCentredArc(cx,cy,radius,radius,0,start,end,true);arc.addCentredArc(cx,cy,radius,radius,0,start,start+pos*(end-start),true);
        g.setColour(juce::Colour(0xff45404d));g.strokePath(rail,juce::PathStrokeType(6));g.setColour(juce::Colour(0xffa14859));g.strokePath(arc,juce::PathStrokeType(6));
        const auto a=start+pos*(end-start);g.setColour(juce::Colour(0xffe5d05a));g.drawLine(cx+std::sin(a)*(radius-4),cy-std::cos(a)*(radius-4),cx+std::sin(a)*(radius+4),cy-std::cos(a)*(radius+4),2);return;
    }
    float radius=std::min(r.getWidth(),r.getHeight())/2,cx=r.getCentreX(),cy=r.getCentreY();
    g.setColour(juce::Colours::black.withAlpha(0.45f));g.fillEllipse(cx-radius+1,cy-radius+3,2*radius,2*radius);
    juce::ColourGradient gradient(juce::Colour(0xff47424b),cx,cy-radius,juce::Colour(0xff242126),cx,cy+radius,false);
    g.setGradientFill(gradient);g.fillEllipse(cx-radius,cy-radius,2*radius,2*radius);
    g.setColour(juce::Colour(0xff68606c));g.drawEllipse(cx-radius,cy-radius,2*radius,2*radius,0.7f);
    juce::Path arc;arc.addCentredArc(cx,cy,radius+2,radius+2,0,start,start+pos*(end-start),true);
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));g.strokePath(arc,juce::PathStrokeType(2));
    float angle=start+pos*(end-start);g.setColour(juce::Colour(0xfff4edf3));
    g.drawLine(cx+std::sin(angle)*radius*.64f,cy-std::cos(angle)*radius*.64f,cx+std::sin(angle)*radius*.87f,cy-std::cos(angle)*radius*.87f,1.5f);
}


void AesterLookAndFeel::drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float min,float max,juce::Slider::SliderStyle kind,juce::Slider& slider){
    if(!bool(slider.getProperties()["threshold"])) {LookAndFeel_V4::drawLinearSlider(g,x,y,w,h,pos,min,max,kind,slider);return;}
    const float cx=x+w*.5f;juce::ColourGradient colour(juce::Colour(0xffb64c42),cx,float(y),juce::Colour(0xff588e17),cx,float(y+h),false);colour.addColour(.45,juce::Colour(0xffb7a31c));g.setGradientFill(colour);g.fillRoundedRectangle(cx-3,float(y),6,float(h),2);
    const float level=juce::jlimit(0.0f,1.0f,(float(slider.getProperties()["detectorDb"])+80)/80);g.setColour(juce::Colour(0xff101a1c));g.fillRoundedRectangle(cx-3,float(y),6,float(h)*(1-level),2);g.setColour(juce::Colours::white.withAlpha(.7f));g.drawHorizontalLine(int(y+h*(1-level)),cx-4,cx+4);
    g.setColour(juce::Colour(0xff79767c));g.fillRoundedRectangle(cx-10,pos-5,20,10,3);g.setColour(juce::Colour(0xffd4cdd4));g.drawHorizontalLine(int(pos),cx-6,cx+6);
}

class EQEditor::BandCard final : public juce::Component {
public:
    BandCard(EQEditor& editor,int index):owner(editor),band(index){
        setName("bandCard"+juce::String(index+1));
        addAndMakeVisible(title);addAndMakeVisible(enabled);
        title.onClick=[this]{owner.select(band);if(!owner.eqProcessor.settings(band).enabled)owner.eqProcessor.setValue(EQProcessor::bandID(band,"on"),1);};
        enabled.setTooltip("Enable this band");enabled.onClick=[this]{owner.select(band);};
        for(auto* slider:{&frequency,&gain,&quality}){addAndMakeVisible(slider);slider->setSliderStyle(juce::Slider::LinearBar);slider->setTextBoxStyle(juce::Slider::TextBoxLeft,false,110,24);slider->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);}
        frequency.setTooltip("Band frequency (Hz)");gain.setTooltip("Band gain (dB)");quality.setTooltip("Band Q");
        frequencyAttachment=std::make_unique<Attachment>(owner.eqProcessor.parameters,EQProcessor::bandID(band,"frequency"),frequency);
        gainAttachment=std::make_unique<Attachment>(owner.eqProcessor.parameters,EQProcessor::bandID(band,"gain"),gain);
        qualityAttachment=std::make_unique<Attachment>(owner.eqProcessor.parameters,EQProcessor::bandID(band,"q"),quality);
        onAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(owner.eqProcessor.parameters,EQProcessor::bandID(band,"on"),enabled);
        frequency.textFromValueFunction=[](double v){return v>=1000?juce::String(v/1000,2)+" kHz":juce::String(v,1)+" Hz";};
        frequency.valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue()*(t.containsIgnoreCase("k")?1000:1);};
        gain.textFromValueFunction=[](double v){return juce::String(std::abs(v)<.005?0:v,1)+" dB";};quality.textFromValueFunction=[](double v){return juce::String(v,2);};
        frequency.updateText();gain.updateText();quality.updateText();refresh();
    }
    void refresh(){const auto state=owner.eqProcessor.settings(band);const auto text=juce::String(band+1).paddedLeft('0',2)+"  "+(state.enabled?shapeNames[int(state.shape)]:juce::String("Enable"));if(title.getButtonText()!=text)title.setButtonText(text);for(auto* slider:{&frequency,&gain,&quality})slider->setVisible(state.enabled);repaint();}
    void paint(juce::Graphics& g)override{
        const bool selected=owner.selected==band,active=owner.eqProcessor.settings(band).enabled;
        g.setColour(juce::Colour(selected?0xff30403f:0xff252d30));g.fillRoundedRectangle(getLocalBounds().toFloat(),6);
        g.setColour(selected?owner.colour(band):juce::Colour(0xff3b484b));g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.6f),6,selected?1.4f:.7f);
        g.setFont(juce::FontOptions(10));g.setColour(juce::Colour(0xffa6b7b5));
        if(active){for(int row=0;row<3;++row)g.drawText(juce::StringArray{"F","G","Q"}[row],8,37+row*30,14,24,juce::Justification::centred);}
        else g.drawText("Click to enable",8,47,getWidth()-16,40,juce::Justification::centred);
    }
    void resized()override{const int w=getWidth();title.setBounds(5,4,w-37,27);enabled.setBounds(w-30,5,25,25);int y=37;for(auto* slider:{&frequency,&gain,&quality}){slider->setBounds(25,y,w-31,24);slider->setTextBoxStyle(juce::Slider::TextBoxLeft,false,w-31,24);y+=30;}}
    void mouseDown(const juce::MouseEvent& e)override{owner.select(band);if(e.mods.isPopupMenu())owner.showBandMenu();}
private:
    EQEditor& owner;int band;
    juce::TextButton title;juce::ToggleButton enabled;
    juce::Slider frequency,gain,quality;
    using Attachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> frequencyAttachment,gainAttachment,qualityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onAttachment;
};

EQEditor::EQEditor(EQProcessor& p):AudioProcessorEditor(p),eqProcessor(p) {
    setLookAndFeel(&style);
    const int initialWidth=p.editorWidth.load(),initialHeight=p.editorHeight.load();
    setResizable(true,true);setResizeLimits(960,680,1920,1200);setSize(initialWidth,initialHeight);
    for(int i=0;i<24;++i)bandPicker.addItem("Band "+juce::String(i+1),i+1);
    shape.addItemList({"Bell","Low Shelf","Low Cut","High Shelf","High Cut","Notch","Band Pass","Tilt Shelf","Flat Tilt","All Pass"},1);
    mode.addItemList({"Static","Dynamic","Spectral"},1);placement.addItemList({"Stereo","Left","Right","Mid","Side"},1);
    slope.addItemList({"6 dB/oct","12 dB/oct","24 dB/oct","36 dB/oct","48 dB/oct","72 dB/oct","96 dB/oct"},1);
    for(auto* c:std::initializer_list<juce::Component*>{&modeSettings,&undo,&redo,&ab,&copyAB,&presetMenu,&threshold,&dynamicRange,&attack,&release,&thresholdLabel,&rangeLabel,&attackLabel,&releaseLabel,&mode,&placement,&analyzer,&shapeButton,&range,&bandPicker,&shape,&slope,&active,&bypass,&add,&remove,&reset,&save,&load,&frequency,&gain,&q,&output,&frequencyLabel,&gainLabel,&qLabel,&shapeLabel,&slopeLabel,&outputLabel})addAndMakeVisible(c);
    addAndMakeVisible(processingButton);addAndMakeVisible(resolutionButton);processingButton.setTooltip("Processing mode: minimum phase, analog-response FIR or linear-phase FIR. Changing mode changes latency.");resolutionButton.setTooltip("Linear FIR resolution: greater resolution increases latency and pre-ringing.");
    processingButton.onClick=[this]{juce::PopupMenu menu;for(int i=0;i<3;++i)menu.addItem(i+1,juce::StringArray{"Live","Analog FIR","Linear FIR"}[i],true,int(eqProcessor.parameters.getRawParameterValue("processingPhase")->load())==i);auto safe=juce::Component::SafePointer<EQEditor>(this);menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetComponent(&processingButton),[safe](int id){if(safe&&id)safe->eqProcessor.setValue("processingPhase",float(id-1));});};
    resolutionButton.onClick=[this]{juce::PopupMenu menu;for(int i=0;i<5;++i)menu.addItem(i+1,juce::StringArray{"Low","Medium","High","Very High","Maximum"}[i],true,int(eqProcessor.parameters.getRawParameterValue("linearResolution")->load())==i);auto safe=juce::Component::SafePointer<EQEditor>(this);menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetComponent(&resolutionButton),[safe](int id){if(safe&&id)safe->eqProcessor.setValue("linearResolution",float(id-1));});};
    addChildComponent(outputPanel);for(auto* child:std::initializer_list<juce::Component*>{&outputKnob,&outputBalance,&outputBalanceMode,&outputInvert,&outputGainLabel,&outputPanLabel})outputPanel.addAndMakeVisible(child);
    for(auto* knob:{&outputKnob,&outputBalance}){knob->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knob->setTextBoxStyle(juce::Slider::TextBoxBelow,false,100,20);knob->setDoubleClickReturnValue(true,0);}
    outputKnob.setTextValueSuffix(" dB");outputKnob.setTooltip("Output volume. Double-click to reset to 0 dB.");outputBalance.setTooltip("Balance: left favours Left (or Mid), right favours Right (or Side). Centre preserves both channels.");outputBalanceMode.addItemList({"L/R","Mid / Side"},1);outputBalanceMode.setTooltip("Choose Left/Right or Mid/Side output balance.");outputInvert.setTooltip("Invert the polarity of both output channels.");outputGainLabel.setText("OUTPUT",juce::dontSendNotification);outputPanLabel.setText("BALANCE",juce::dontSendNotification);
    outputKnobAttachment=std::make_unique<SliderAttachment>(p.parameters,"output",outputKnob);outputBalanceAttachment=std::make_unique<SliderAttachment>(p.parameters,"balance",outputBalance);outputBalanceModeAttachment=std::make_unique<ComboAttachment>(p.parameters,"balanceMode",outputBalanceMode);outputInvertAttachment=std::make_unique<ButtonAttachment>(p.parameters,"invert",outputInvert);
    output.setTooltip("Hover to open output volume and L/R or M/S balance. Click to pin the panel; click again to release.");output.addMouseListener(this,false);
    frequency.setTooltip("Frequency of the selected EQ band. Drag or double-click a node to edit.");gain.setTooltip("Gain of the selected EQ band in dB.");q.setTooltip("Q: higher values produce a narrower frequency band.");active.setTooltip("Enable or disable the selected EQ band.");placement.setTooltip("Process Stereo, Left, Right, Mid or Side for this band.");attack.setTooltip("Dynamic detector attack time in milliseconds.");release.setTooltip("Dynamic detector release time in milliseconds.");
    addAndMakeVisible(triggerButton);triggerButton.setTooltip("Dynamic detector: Band follows EQ; Free uses independent low/high cuts");triggerButton.onClick=[this]{const int band=selected;const auto p=eqProcessor.settings(band);juce::PopupMenu menu;menu.addItem(1,"Band",true,!p.freeTrigger);menu.addItem(2,"Free",true,p.freeTrigger);const std::array<float,9> cuts{20,50,100,200,500,1000,2000,5000,20000};juce::PopupMenu low,high;for(int i=0;i<int(cuts.size());++i){low.addItem(100+i,juce::String(cuts[size_t(i)],0)+" Hz",true,p.triggerLow==cuts[size_t(i)]);high.addItem(200+i,juce::String(cuts[size_t(i)],0)+" Hz",true,p.triggerHigh==cuts[size_t(i)]);}menu.addSubMenu("Free low cut",low);menu.addSubMenu("Free high cut",high);auto safe=juce::Component::SafePointer<EQEditor>(this);menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetComponent(&triggerButton),[safe,band,cuts](int id){if(!safe||id==0)return;if(id<3)safe->eqProcessor.setValue(EQProcessor::bandID(band,"free"),id==2?1:0);else{safe->eqProcessor.setValue(EQProcessor::bandID(band,id>=200?"triggerHigh":"triggerLow"),cuts[size_t(id%100)]);safe->eqProcessor.setValue(EQProcessor::bandID(band,"free"),1);}});};
    addAndMakeVisible(externalKey);externalKey.setTooltip("External sidechain: choose the source in Logic Side Chain first");
    for(auto* button:{&nodeActions,&help,&resizeEditor,&slopeButton})addAndMakeVisible(button);
    for(auto* s:std::initializer_list<juce::Slider*>{&frequency,&gain,&q,&output,&threshold,&dynamicRange,&attack,&release}) {
        s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,110,24);
        s->setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffdfff00));
        s->setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
    }
    threshold.setTextValueSuffix(" dB");threshold.setTooltip("Band threshold (dBFS)");slope.setTooltip("Band slope: select dB/oct for Low Cut or High Cut");dynamicRange.setTextValueSuffix(" dB");attack.setTextValueSuffix(" ms");release.setTextValueSuffix(" ms");
    frequency.setTextValueSuffix(" Hz");gain.setTextValueSuffix(" dB");output.setTextValueSuffix(" dB");
    frequency.textFromValueFunction=[](double value){return juce::String(value,1);};
    q.textFromValueFunction=[](double value){return juce::String(value,3);};
    for(auto pair:std::initializer_list<std::pair<juce::Label*,juce::String>>{{&thresholdLabel,"THRESHOLD"},{&rangeLabel,"RANGE"},{&attackLabel,"ATTACK"},{&releaseLabel,"RELEASE"},{&frequencyLabel,"FREQ"},{&gainLabel,"GAIN"},{&qLabel,"Q"},{&shapeLabel,"SHAPE"},{&slopeLabel,"CUT SLOPE"},{&outputLabel,"OUTPUT"}}) {
        pair.first->setText(pair.second,juce::dontSendNotification);pair.first->setJustificationType(juce::Justification::centred);
        pair.first->setColour(juce::Label::textColourId,juce::Colour(0xff9eaeb9));
    }
    addAndMakeVisible(freeze);freeze.setTooltip("Freeze the spectrum display while audio and meters continue.");
    freeze.onClick=[this]{analyzerFrozen=!analyzerFrozen;freeze.setButtonText(analyzerFrozen?"Resume":"Freeze");repaint();};
    addAndMakeVisible(wetMix);addAndMakeVisible(wetMixLabel);wetMixLabel.setText("MIX",juce::dontSendNotification);
    wetMix.setSliderStyle(juce::Slider::LinearHorizontal);wetMix.setTextBoxStyle(juce::Slider::TextBoxRight,false,55,22);wetMix.setTooltip("Latency-aligned dry/wet blend. 0% dry, 100% processed. Output settings are blended too.");
    wetMix.textFromValueFunction=[](double v){return juce::String(v*100,0)+" %";};
    wetMix.valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue()/100;};
    mixAttachment=std::make_unique<SliderAttachment>(p.parameters,"mix",wetMix);
    wetMix.textFromValueFunction=[](double v){return juce::String(v*100,0)+" %";};wetMix.valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue()/100;};wetMix.updateText();
    outputAttachment=std::make_unique<SliderAttachment>(p.parameters,"output",output);
    output.textFromValueFunction=[](double value){return juce::String(value,1);};
    output.updateText();
    bypassAttachment=std::make_unique<ButtonAttachment>(p.parameters,"bypass",bypass);
    bandPicker.onChange=[this]{select(bandPicker.getSelectedId()-1);};
    add.onClick=[this]{addBand(1000,0);};
    remove.onClick=[this]{eqProcessor.setValue(EQProcessor::bandID(selected,"on"),0);};
    reset.onClick=[this]{for(int i=0;i<24;++i){eqProcessor.setValue(EQProcessor::bandID(i,"on"),0);eqProcessor.setValue(EQProcessor::bandID(i,"shape"),0);eqProcessor.setValue(EQProcessor::bandID(i,"gain"),0);eqProcessor.setValue(EQProcessor::bandID(i,"frequency"),1000);eqProcessor.setValue(EQProcessor::bandID(i,"q"),1.0f);eqProcessor.setValue(EQProcessor::bandID(i,"slope"),1);eqProcessor.setValue(EQProcessor::bandID(i,"mode"),0);eqProcessor.setValue(EQProcessor::bandID(i,"external"),0);eqProcessor.setValue(EQProcessor::bandID(i,"free"),0);eqProcessor.setValue(EQProcessor::bandID(i,"triggerLow"),20);eqProcessor.setValue(EQProcessor::bandID(i,"triggerHigh"),20000);eqProcessor.setValue(EQProcessor::bandID(i,"placement"),0);eqProcessor.setValue(EQProcessor::bandID(i,"threshold"),-24);eqProcessor.setValue(EQProcessor::bandID(i,"range"),-6);eqProcessor.setValue(EQProcessor::bandID(i,"attack"),10);eqProcessor.setValue(EQProcessor::bandID(i,"release"),120);}eqProcessor.setValue("mix",1);eqProcessor.setValue("output",0);eqProcessor.setValue("balance",0);eqProcessor.setValue("balanceMode",0);eqProcessor.setValue("invert",0);eqProcessor.setValue("midGain",0);eqProcessor.setValue("sideGain",0);eqProcessor.setValue("midInvert",0);eqProcessor.setValue("sideInvert",0);eqProcessor.setValue("processingPhase",0);eqProcessor.setValue("linearResolution",1);eqProcessor.setValue("bypass",0);select(0);};
    save.onClick=[this]{preset(true);};load.onClick=[this]{preset(false);};
    range.onClick=[this]{displayRange=displayRange==12?30:displayRange==30?6:12;range.setButtonText(juce::String(displayRange,0)+" dB");repaint();};
    shape.setVisible(false);mode.setVisible(false);
    modeSettings.setTooltip("Band processing: Static, Dynamic or Spectral");modeSettings.getProperties().set("toolbar",true);modeSettings.onClick=[this]{juce::PopupMenu menu;menu.setLookAndFeel(&menuStyle());for(int i=0;i<3;++i)menu.addItem(i+1,juce::StringArray{"Static","Dynamic","Spectral"}[i],true,eqProcessor.settings(selected).mode==i);auto safe=juce::Component::SafePointer<EQEditor>(this);const int band=selected;menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetComponent(&modeSettings),[safe,band](int result){if(safe&&result)safe->eqProcessor.setValue(EQProcessor::bandID(band,"mode"),float(result-1));});};
    shapeButton.onClick=[this]{showShapeMenu();};
    nodeActions.setTooltip("Band menu");nodeActions.onClick=[this]{showBandMenu();};
    slopeButton.setButtonText("12 dB/oct");slopeButton.setTooltip("Band slope (dB/oct)");
    slopeButton.onClick=[this]{juce::PopupMenu menu;for(int i=6;i>=0;--i)menu.addItem(i+1,juce::StringArray{"6","12","24","36","48","72","96"}[i]+" dB/oct",true,eqProcessor.settings(selected).slope==i);auto safe=juce::Component::SafePointer<EQEditor>(this);menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetComponent(&slopeButton).withMinimumWidth(118).withStandardItemHeight(26),[safe](int id){if(safe&&id>0)safe->eqProcessor.setValue(EQProcessor::bandID(safe->selected,"slope"),float(id-1));});};
    analyzer.onClick=[this]{juce::PopupMenu menu;for(int mode=0;mode<4;++mode)menu.addItem(mode+1,juce::StringArray{"Off","Pre","Post","Pre+Post"}[mode],true,mode==analyzerMode);juce::PopupMenu tilt;for(int i=0;i<4;++i)tilt.addItem(101+i,juce::String(std::array<float,4>{0,3,4.5f,6}[size_t(i)],1)+" dB/oct",true,analyzerTilt==std::array<float,4>{0,3,4.5f,6}[size_t(i)]);menu.addSubMenu("Display tilt",tilt);menu.addItem(202,"Side Chain spectrum",true,showSidechain);auto safe=juce::Component::SafePointer<EQEditor>(this);menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetComponent(&analyzer),[safe](int result){if(!safe||result==0)return;if(result==202){safe->showSidechain=!safe->showSidechain;safe->eqProcessor.sidechainSpectrum.enabled.store(safe->showSidechain);safe->repaint();return;}if(result>=101&&result<=104){safe->analyzerTilt=std::array<float,4>{0,3,4.5f,6}[size_t(result-101)];safe->repaint();return;}safe->analyzerMode=result-1;safe->analyzer.setButtonText("Analyzer: "+juce::StringArray{"Off","Pre","Post","Pre+Post"}[result-1]);safe->eqProcessor.spectrum.enabled.store(result!=1);});};
    save.setVisible(false);load.setVisible(false);reset.setVisible(false);bypass.setVisible(false);
    for(auto* button:{&nodeActions,&help,&resizeEditor,&undo,&redo,&ab,&copyAB,&presetMenu})button->getProperties().set("toolbar",true);
    undo.getProperties().set("icon","undo");redo.getProperties().set("icon","redo");modeSettings.getProperties().set("icon","settings");
    undo.setTooltip("Undo");redo.setTooltip("Redo");ab.setTooltip("Switch between A and B");copyAB.setTooltip("Copy current setting to the other A/B slot");
    undo.onClick=[this]{commitHistory();if(historyPosition>0){restoreState(history[size_t(--historyPosition)]);history[size_t(historyPosition)]=captureState();}};
    redo.onClick=[this]{commitHistory();if(historyPosition+1<int(history.size())){restoreState(history[size_t(++historyPosition)]);history[size_t(historyPosition)]=captureState();}};
    ab.onClick=[this]{commitHistory();(editingB?stateB:stateA)=captureState();editingB=!editingB;restoreState(editingB?stateB:stateA);ab.setButtonText(editingB?"A / B*":"A* / B");commitHistory();};
    copyAB.onClick=[this]{(editingB?stateA:stateB)=captureState();};
    presetMenu.onClick=[this]{showPresetMenu();};
    help.setTooltip("Help: hover a control for its description. Click for the usage guide.");
    help.onClick=[this]{juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,"Aester EQ by Neurora", "Aester EQ is a precision equalizer by Neurora. https://www.neurora.it\n\nDouble-click the graph to add a band. Drag a node to set frequency and gain; scroll to change Q. Right-click a node for band actions. Use the band settings menu for Static, Dynamic or Spectral. Select a node to show its floating frequency, gain and Q controls. SOLO isolates its frequency region; Q sets the audition bandwidth. Click SOLO again to restore the full mix. Solo is not stored in presets and ends when this editor closes. Mix blends latency-aligned dry and wet signals; Freeze holds the analyzer display. Threshold, range, attack and release control dynamics. Live mode keeps Static and Dynamic at zero delay. Analog FIR uses an analog-response FIR with 320 samples of delay. Linear FIR offers five FIR resolutions with increasing delay. Spectral adds 1024 samples. The footer contains Mix, output, balance, polarity and processing phase controls. Undo/Redo and A/B are local to this editor.", "OK",this);};
    resizeEditor.setButtonText("Size");resizeEditor.setTooltip("Choose a window size, or drag the lower-right corner to resize freely.");resizeEditor.onClick=[this]{juce::PopupMenu menu;menu.addItem(1,"Small · 1000 x 700");menu.addItem(2,"Medium · 1280 x 840");menu.addItem(3,"Large · 1440 x 960");auto safe=juce::Component::SafePointer<EQEditor>(this);menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetComponent(&resizeEditor),[safe](int result){if(!safe||result==0)return;if(result==1)safe->setSize(1000,700);else if(result==2)safe->setSize(1280,840);else if(result==3)safe->setSize(1440,960);});};
    addAndMakeVisible(solo);addAndMakeVisible(closePanel);
    solo.setClickingTogglesState(true);solo.setColour(juce::TextButton::textColourOnId,juce::Colour(0xff15191c));
    solo.setTooltip("Solo the selected frequency region. Q sets bandwidth. Monitoring only; click again to exit.");
    solo.onClick=[this]{eqProcessor.setAuditionBand(solo.getToggleState()?selected:-1);resized();repaint();};
    closePanel.setTooltip("Hide band controls");closePanel.onClick=[this]{panelDismissed=true;eqProcessor.setAuditionBand(-1);resized();repaint();};
    for(auto* slider:std::initializer_list<juce::Slider*>{&frequency,&gain,&q,&threshold,&dynamicRange,&attack,&release}){
        slider->onDragStart=[this]{knobDragging=true;};slider->onDragEnd=[this]{knobDragging=false;resized();repaint();};
    }
    addAndMakeVisible(previousPage);addAndMakeVisible(nextPage);addAndMakeVisible(pageLabel);
    previousPage.setTooltip("Previous six bands");nextPage.setTooltip("Next six bands");previousPage.onClick=[this]{select(std::max(0,bandPage-1)*6);};nextPage.onClick=[this]{select(std::min(3,bandPage+1)*6);};
    for(auto* child:std::initializer_list<juce::Component*>{&outputKnob,&outputBalance,&outputBalanceMode,&outputInvert,&outputGainLabel,&outputPanLabel})addAndMakeVisible(child);
    outputGainLabel.setText("OUTPUT",juce::dontSendNotification);outputPanLabel.setText("BALANCE",juce::dontSendNotification);
    for(auto* slider:{&outputKnob,&outputBalance,&wetMix}){slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider->setTextBoxStyle(juce::Slider::TextBoxBelow,false,88,20);slider->setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffdfff00));slider->setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);}
    wetMix.setDoubleClickReturnValue(true,1);
    outputBalance.textFromValueFunction=[this](double value){if(std::abs(value)<.0005)return juce::String("C");const bool ms=eqProcessor.parameters.getRawParameterValue("balanceMode")->load()>.5f;return juce::String(value<0?(ms?"M ":"L "):(ms?"S ":"R "))+juce::String(std::abs(value)*100,0)+"%";};
    outputBalance.valueFromTextFunction=[](const juce::String& text){const auto t=text.trim().toUpperCase();if(t=="C")return 0.0;const double value=t.retainCharacters("0123456789.-").getDoubleValue()/100;return t.startsWith("L")||t.startsWith("M")?-std::abs(value):t.startsWith("R")||t.startsWith("S")?std::abs(value):value;};
    outputBalance.updateText();
    outputBalance.setTooltip("Output balance after Mix. L/R attenuates the opposite channel; M/S favours Mid or Side. Double-click to centre. Unavailable in mono.");
    wetMix.setTooltip("Latency-aligned dry/wet blend. Balance and polarity work on the blended signal, including at 0% Mix.");
    addChildComponent(midSidePanel);
    for(auto* c:std::initializer_list<juce::Component*>{&midOutput,&sideOutput,&midInvert,&sideInvert,&midLabel,&sideLabel,&midSideTitle})midSidePanel.addAndMakeVisible(c);
    midSideTitle.setText("MID / SIDE OUTPUT",juce::dontSendNotification);midLabel.setText("MID",juce::dontSendNotification);sideLabel.setText("SIDE",juce::dontSendNotification);
    for(auto* label:{&midLabel,&sideLabel})label->setJustificationType(juce::Justification::centred);
    for(auto* knob:{&midOutput,&sideOutput}){knob->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knob->setTextBoxStyle(juce::Slider::TextBoxBelow,false,100,22);knob->setTextValueSuffix(" dB");knob->setDoubleClickReturnValue(true,0);knob->setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffdfff00));}
    midOutput.setTooltip("Mid output gain: centre / mono information. Active in Mid / Side mode only.");sideOutput.setTooltip("Side output gain: stereo difference information. Active in Mid / Side mode only.");
    midInvert.setTooltip("Invert only the Mid component before decoding back to L/R.");sideInvert.setTooltip("Invert only the Side component before decoding back to L/R.");
    midOutputAttachment=std::make_unique<SliderAttachment>(p.parameters,"midGain",midOutput);sideOutputAttachment=std::make_unique<SliderAttachment>(p.parameters,"sideGain",sideOutput);
    midInvertAttachment=std::make_unique<ButtonAttachment>(p.parameters,"midInvert",midInvert);sideInvertAttachment=std::make_unique<ButtonAttachment>(p.parameters,"sideInvert",sideInvert);
    outputBalanceMode.onChange=[this]{updateMidSideControls();};
    outputInvert.setButtonText("INV ALL");outputInvert.setTooltip("Invert both output channels. Use Mid / Side controls for independent polarity.");
    updateMidSideControls();resized();
    observedState=captureState();stateA=stateB=observedState;history.push_back(observedState);ab.setButtonText("A* / B");
    eqProcessor.spectrum.enabled.store(true);
    select(0);startTimerHz(60);
}
EQEditor::~EQEditor(){eqProcessor.setAuditionBand(-1);eqProcessor.sidechainSpectrum.enabled.store(false);eqProcessor.spectrum.enabled.store(false);setLookAndFeel(nullptr);stopTimer();if(dragging)for(auto* field:{"frequency","gain"})eqProcessor.parameters.getParameter(EQProcessor::bandID(selected,field))->endChangeGesture();}
void EQEditor::select(int band) {
    selected=juce::jlimit(0,23,band);bandPage=selected/6;panelDismissed=false;if(eqProcessor.auditionBand()>=0)eqProcessor.setAuditionBand(selected);
    externalAttachment.reset();modeAttachment.reset();placementAttachment.reset();thresholdAttachment.reset();rangeAttachment.reset();attackAttachment.reset();releaseAttachment.reset();
    frequencyAttachment.reset();gainAttachment.reset();qAttachment.reset();shapeAttachment.reset();slopeAttachment.reset();activeAttachment.reset();
    auto id=[this](const char* name){return EQProcessor::bandID(selected,name);};
    frequencyAttachment=std::make_unique<SliderAttachment>(eqProcessor.parameters,id("frequency"),frequency);
    gainAttachment=std::make_unique<SliderAttachment>(eqProcessor.parameters,id("gain"),gain);
    qAttachment=std::make_unique<SliderAttachment>(eqProcessor.parameters,id("q"),q);
    shapeAttachment=std::make_unique<ComboAttachment>(eqProcessor.parameters,id("shape"),shape);
    externalAttachment=std::make_unique<ButtonAttachment>(eqProcessor.parameters,id("external"),externalKey);
    slopeAttachment=std::make_unique<ComboAttachment>(eqProcessor.parameters,id("slope"),slope);
    modeAttachment=std::make_unique<ComboAttachment>(eqProcessor.parameters,id("mode"),mode);placementAttachment=std::make_unique<ComboAttachment>(eqProcessor.parameters,id("placement"),placement);
    thresholdAttachment=std::make_unique<SliderAttachment>(eqProcessor.parameters,id("threshold"),threshold);rangeAttachment=std::make_unique<SliderAttachment>(eqProcessor.parameters,id("range"),dynamicRange);attackAttachment=std::make_unique<SliderAttachment>(eqProcessor.parameters,id("attack"),attack);releaseAttachment=std::make_unique<SliderAttachment>(eqProcessor.parameters,id("release"),release);
    for(auto* knob:std::initializer_list<juce::Slider*>{&threshold,&dynamicRange,&attack,&release}){knob->textFromValueFunction=[knob](double value){return juce::String(value,1);};knob->updateText();}
    expanded=eqProcessor.settings(selected).mode!=0;
    activeAttachment=std::make_unique<ButtonAttachment>(eqProcessor.parameters,id("on"),active);
    frequency.textFromValueFunction=[](double value){return juce::String(value,1);};
    q.textFromValueFunction=[](double value){return juce::String(value,3);};
    frequency.updateText();q.updateText();
    shapeButton.getProperties().set("shape",int(eqProcessor.settings(selected).shape));shapeButton.setButtonText(shapeNames[int(eqProcessor.settings(selected).shape)]);
    for(auto* knob:std::initializer_list<juce::Slider*>{&frequency,&gain,&q})knob->setColour(juce::Slider::rotarySliderFillColourId,colour(selected));
    bandPicker.setSelectedId(selected+1,juce::dontSendNotification);resized();repaint();
}
juce::Colour EQEditor::colour(int band) const {if(band==selected)return juce::Colour(0xffdfff00);const juce::uint32 palette[]{0xff75ca69,0xff4ec4de,0xffb75fe1,0xffecc94b,0xffed7765,0xff6e94e5,0xffe782b8,0xffa1d681};return juce::Colour(palette[band%8]);}
float EQEditor::xFor(double hz) const {return plot.getX()+plot.getWidth()*float(std::log(juce::jlimit(10.0,30000.0,hz)/10)/std::log(3000.0));}
float EQEditor::yFor(double db) const {return plot.getY()+plot.getHeight()*.5f-float(db)*plot.getHeight()/(2*displayRange);}
double EQEditor::hzFor(float x) const {return 10*std::pow(3000.0,juce::jlimit(0.0f,1.0f,(x-plot.getX())/plot.getWidth()));}
double EQEditor::dbFor(float y) const {return juce::jlimit(-30.0,30.0,double((plot.getY()+plot.getHeight()*.5f-y)*2*displayRange/plot.getHeight()));}
void EQEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1a2024));
    g.setColour(juce::Colour(0xff232a2d));g.fillRoundedRectangle(plot.expanded(8,8),6);

    g.setColour(juce::Colour(0xffe5e9e7));g.setFont(juce::FontOptions(25,juce::Font::bold));g.drawText("AESTER EQ",24,11,185,38,juce::Justification::left);
    g.setColour(juce::Colour(0xffa6b7b5));g.setFont(juce::FontOptions(12));g.drawText("NEURORA",getWidth()-180,21,105,24,juce::Justification::centred);
    g.setColour(juce::Colour(0xffb4c3c0));g.setFont(juce::FontOptions(10));g.drawText("WET RESPONSE / FREQUENCY",24,61,300,18,juce::Justification::left);
    bool hasBands=false;for(int i=0;i<24;++i)hasBands=hasBands||eqProcessor.settings(i).enabled;
    if(!hasBands){g.setFont(juce::FontOptions(15));g.drawText("Double-click the graph to add a band",int(plot.getCentreX())-200,int(plot.getCentreY())+35,400,30,juce::Justification::centred);}
    g.setFont(juce::FontOptions(11));
    for(double hz:{20.,30.,40.,50.,60.,80.,100.,150.,200.,300.,400.,500.,600.,800.,1000.,1500.,2000.,3000.,4000.,5000.,6000.,8000.,10000.,15000.,20000.}) {
        float x=xFor(hz);g.setColour(juce::Colour(0xff2a3b3e).withAlpha(0.65f));g.drawVerticalLine(int(x),plot.getY(),plot.getBottom());
    }
    for(double hz:{20.,50.,100.,200.,500.,1000.,2000.,5000.,10000.,20000.}) {
        g.setColour(juce::Colour(0xff9c959f));g.drawText(hz>=1000?juce::String(hz/1000,0)+"k":juce::String(hz,0),int(xFor(hz))-20,int(plot.getBottom())-22,40,18,juce::Justification::centred);
    }
    int step=displayRange==30?6:displayRange==12?3:1;
    for(int db=-int(displayRange);db<=int(displayRange);db+=step){float y=yFor(db);g.setColour(juce::Colour(db==0?0xff49605c:0xff2a3b3e));g.drawHorizontalLine(int(y),plot.getX(),plot.getRight());g.setColour(juce::Colour(0xffa1b5b1));if(db<int(displayRange))g.drawText((db>0?"+":"")+juce::String(db),int(plot.getRight())-61,int(y)-9,30,18,juce::Justification::right);}
    if(analyzerMode!=0||showSidechain){juce::Graphics::ScopedSaveState saved(g);g.reduceClipRegion(plot.toNearestInt());
        for(int stage=0;stage<3;++stage){if((stage==2&&!showSidechain)||(stage<2&&analyzerMode==0)||(stage==0&&analyzerMode==2)||(stage==1&&analyzerMode==1))continue;const auto& display=stage==2?sidechainDisplay:spectrum;juce::Path trace;float previousX=0,previousY=0;
            for(int i=0;i<=600;++i){float x=plot.getX()+plot.getWidth()*float(i)/600;double bin=hzFor(x)*SpectrumTap::size/display.rate;const float rawDb=bin>SpectrumDisplay::bins-1?-100:display.displayDb(stage==2?0:stage,bin);float db=SpectrumDisplay::tiltedDb(rawDb,hzFor(x),analyzerTilt);const double edge=juce::jlimit(0.0,1.0,(double(SpectrumDisplay::bins-1)-bin)/4.0);db=float(10*std::log10(1e-10+std::max(0.0,std::pow(10.0,double(db)/10)-1e-10)*edge*edge));float y=plot.getBottom()-juce::jlimit(0.0f,1.0f,(db+100)/100)*plot.getHeight();if(i==0)trace.startNewSubPath(x,y);else trace.quadraticTo(previousX,previousY,(previousX+x)*.5f,(previousY+y)*.5f);previousX=x;previousY=y;if(i==600)trace.lineTo(x,y);}
            auto fill=trace;fill.lineTo(plot.getRight(),plot.getBottom());fill.lineTo(plot.getX(),plot.getBottom());fill.closeSubPath();g.setColour(juce::Colour(stage==0?0xff9b949d:0xffd7d2d9).withAlpha(stage==0?.08f:.12f));g.fillPath(fill);g.setColour(juce::Colour(stage==2?0xff63b9e3:stage==0?0xff8c858f:0xffbcb6c3).withAlpha(.75f));g.strokePath(trace,juce::PathStrokeType(1));
        }
    }
    if(analyzerMode!=0){g.setFont(juce::FontOptions(10));g.setColour(juce::Colour(0xff8c8692));for(int db=-10;db>=-90;db-=10){float y=plot.getBottom()-(db+100)*plot.getHeight()/100;g.drawText(juce::String(db),int(plot.getRight())-27,int(y)-8,24,16,juce::Justification::right);}}
    const double sr=eqProcessor.getSampleRate()>0?eqProcessor.getSampleRate():48000;
    std::array<distillazione::SectionBank,24> banks,dynamicBanks;
    int count=0;
    for(int i=0;i<24;++i){auto p=eqProcessor.settings(i);banks[size_t(i)]=distillazione::designStaticBand(p,sr);dynamicBanks[size_t(i)]=distillazione::designStaticBand({p.enabled&&p.mode==1,distillazione::dynamicShapeFor(p.shape),p.frequency,eqProcessor.dynamicAmount[size_t(i)].load(),p.q,1},sr);if(p.enabled)++count;}
    juce::Path total,individual;std::array<juce::Path,24> bandPaths;
    for(int i=0;i<=320;++i){float x=plot.getX()+plot.getWidth()*float(i)/320;double hz=hzFor(x),theta=2*juce::MathConstants<double>::pi*std::min(hz,sr*0.499)/sr;
        double db=eqProcessor.parameters.getRawParameterValue("output")->load()+eqProcessor.spectralResponseAt(hz),single=0;
        for(int band=0;band<24;++band){double m=std::abs(banks[size_t(band)].response(theta)*dynamicBanks[size_t(band)].response(theta));double value=20*std::log10(std::max(1e-12,m));db+=value;const float bandY=juce::jlimit(plot.getY(),plot.getBottom(),yFor(value));if(i==0)bandPaths[size_t(band)].startNewSubPath(x,bandY);else bandPaths[size_t(band)].lineTo(x,bandY);if(band==selected){auto p=eqProcessor.settings(band);single=value;if(p.mode!=0){auto rangeBank=distillazione::designStaticBand({p.enabled,distillazione::dynamicShapeFor(p.shape),p.frequency,p.range,p.q,1},sr);single=20*std::log10(std::max(1e-12,std::abs(banks[size_t(band)].response(theta)*rangeBank.response(theta))));}}}
        float y=juce::jlimit(plot.getY(),plot.getBottom(),yFor(db)),sy=juce::jlimit(plot.getY(),plot.getBottom(),yFor(single));
        if(i==0){total.startNewSubPath(x,y);individual.startNewSubPath(x,sy);}else{total.lineTo(x,y);individual.lineTo(x,sy);}
    }
    const int soloBand=eqProcessor.auditionBand();
    const bool soloActive=soloBand>=0&&eqProcessor.settings(soloBand).enabled;
    {juce::Graphics::ScopedSaveState saved(g);g.reduceClipRegion(plot.toNearestInt());
        for(int band=0;band<24;++band)if(eqProcessor.settings(band).enabled){
            g.setColour(soloActive?juce::Colour(0xff758185).withAlpha(.13f):colour(band).withAlpha(.45f));
            if(!soloActive||band!=soloBand)g.strokePath(bandPaths[size_t(band)],juce::PathStrokeType(1.1f));
        }
        auto highlighted=soloActive?bandPaths[size_t(soloBand)]:individual;
        auto fill=highlighted;fill.lineTo(plot.getRight(),yFor(0));fill.lineTo(plot.getX(),yFor(0));fill.closeSubPath();
        if(soloActive){const auto p=eqProcessor.settings(soloBand);const double displayedGain=p.gain+(p.mode==1?eqProcessor.dynamicAmount[size_t(soloBand)].load():0);const float apex=juce::jlimit(plot.getY(),plot.getBottom(),yFor(std::abs(displayedGain)>.05?displayedGain:1));
            juce::ColourGradient gradient(juce::Colour(0xffdfff00).withAlpha(.52f),xFor(p.frequency),apex,juce::Colour(0xffdfff00).withAlpha(.015f),xFor(p.frequency),yFor(0),false);g.setGradientFill(gradient);g.fillPath(fill);
            g.setColour(juce::Colour(0xff758185).withAlpha(.28f));g.strokePath(total,juce::PathStrokeType(1.4f));
            g.setColour(juce::Colour(0xffdfff00));g.strokePath(highlighted,juce::PathStrokeType(2.2f));
        }else{g.setColour(colour(selected).withAlpha(.16f));g.fillPath(fill);g.setColour(colour(selected).withAlpha(.6f));g.strokePath(individual,juce::PathStrokeType(1.2f));g.setColour(juce::Colour(0xffdfff00));g.strokePath(total,juce::PathStrokeType(2));}
    }
    for(int i=0;i<24;++i){auto p=eqProcessor.settings(i);if(!p.enabled)continue;const bool muted=soloActive&&i!=soloBand;auto point=juce::Point<float>{xFor(p.frequency),juce::jlimit(plot.getY()+8,plot.getBottom()-26,yFor(p.gain))};g.setColour(colour(i).withAlpha(.12f));if(i==selected&&!muted)g.fillEllipse(point.x-13,point.y-13,26,26);g.setColour(muted?juce::Colour(0xff758185).withAlpha(.2f):colour(i));g.fillEllipse(point.x-5,point.y-5,10,10);g.setColour(juce::Colour(0xffe9e3ed).withAlpha(muted?.15f:1.0f));g.drawEllipse(point.x-5,point.y-5,10,10,1);}
    const auto selectedBand=eqProcessor.settings(selected);
    const float meterTop=98,meterHeight=plot.getHeight()-26;
    for(int channel=0;channel<2;++channel){const float level=meterDisplayDb[size_t(channel)],mx=float(getWidth()-47+channel*17);g.setColour(juce::Colour(0xff30393c));g.fillRoundedRectangle(mx,meterTop,7,meterHeight,3);const float height=meterHeight*juce::jlimit(0.0f,1.0f,(level+72)/72);g.setColour(juce::Colour(level>0?0xffef886f:channel==0?0xffdfff00:0xff64c9ce));g.fillRect(mx,meterTop+meterHeight-height,7.0f,height);g.setFont(juce::FontOptions(10));g.setColour(juce::Colour(0xffa7b3b1));g.drawText(channel==0?"L":"R",int(mx)-4,76,16,16,juce::Justification::centred);}
    g.setColour(juce::Colour(0xff171d21));g.fillRect(0,getHeight()-90,getWidth(),90);
    g.setColour(juce::Colour(0xff859a99));g.setFont(juce::FontOptions(10));g.drawText(juce::String(count)+" / 24 BANDS | "+juce::String(eqProcessor.getLatencySamples())+" samples",24,getHeight()-27,146,20,juce::Justification::left);
    g.setColour(juce::Colour(0xffa3b0aa));for(int i=0;i<3;++i){const float d=5.0f+float(i*5);g.drawLine(float(getWidth())-d-4,float(getHeight())-4,float(getWidth())-4,float(getHeight())-d-4,1.2f);}
    if(analyzerFrozen){g.setColour(juce::Colour(0xffdfff00));g.drawText("SPECTRUM HELD",int(plot.getX())+12,int(plot.getY())+14,160,20,juce::Justification::left);}
    if(eqProcessor.auditionBand()>=0){g.setColour(juce::Colour(0xffdfff00));g.drawText("SOLO · BAND "+juce::String(eqProcessor.auditionBand()+1),int(plot.getX())+12,int(plot.getY())+36,180,22,juce::Justification::left);}
    if(selectedBand.enabled&&!panelDismissed){
        g.setColour(juce::Colours::black.withAlpha(.35f));g.fillRoundedRectangle(panel.translated(0,4).expanded(3),12);
        g.setColour(juce::Colour(0xff202529));g.fillRoundedRectangle(panel,10);g.setColour(juce::Colour(0xff505b5c));g.drawRoundedRectangle(panel,10,1);
        g.setColour(juce::Colour(0xffdfff00));g.fillRoundedRectangle(panel.getX()+16,panel.getY()+16,4,16,2);
        g.setColour(juce::Colour(0xffd7dfdd));g.setFont(juce::FontOptions(12));g.drawText("BAND "+juce::String(selected+1),int(panel.getX())+25,int(panel.getY())+8,90,24,juce::Justification::left);
        const float nx=xFor(selectedBand.frequency),ny=juce::jlimit(plot.getY()+8,plot.getBottom()-26,yFor(selectedBand.gain));
        juce::Rectangle<float> tip(juce::jlimit(plot.getX(),plot.getRight()-126,nx-63),juce::jlimit(plot.getY(),plot.getBottom()-48,ny-58),126,42);
        if(!tip.intersects(panel)){g.setColour(juce::Colour(0xff202529));g.fillRoundedRectangle(tip,6);g.setColour(juce::Colour(0xffdce4df));g.drawText(juce::String(selectedBand.frequency,1)+" Hz",tip.toNearestInt().removeFromTop(21),juce::Justification::centred);g.drawText(juce::String(selectedBand.gain,2)+" dB",tip.toNearestInt().withTrimmedTop(21),juce::Justification::centred);}
    }

}

void EQEditor::resized() {
    const int w=getWidth(),h=getHeight();eqProcessor.editorWidth.store(w);eqProcessor.editorHeight.store(h);plot={24,84,float(w-96),float(h-194)};
    undo.setBounds(218,18,30,28);redo.setBounds(253,18,30,28);presetMenu.setBounds(300,18,std::max(140,w-800),30);ab.setBounds(w-490,18,65,28);copyAB.setBounds(w-420,18,50,28);help.setBounds(w-340,18,60,28);resizeEditor.setBounds(w-265,18,50,28);
    const auto p=eqProcessor.settings(selected);const bool visible=p.enabled&&!panelDismissed;
    const float panelScale=juce::jlimit(.70f,1.0f,std::min(float(w)/1280,float(h)/840));
    const float pw=620*panelScale,ph=(expanded?304:254)*panelScale;
    if(!knobDragging||panel.isEmpty()){
        float px=juce::jlimit(plot.getX()+8,plot.getRight()-pw-8,xFor(p.frequency)-pw*.5f);
        const float nodeY=yFor(p.gain);
        if(panelBelow&&nodeY>plot.getBottom()-ph-8)panelBelow=false;
        else if(!panelBelow&&nodeY<plot.getY()+ph+8)panelBelow=true;
        float desiredY=panelBelow?nodeY+24:nodeY-ph-24;
        if(eqProcessor.auditionBand()==selected)desiredY=p.gain>=0?std::max(desiredY,yFor(0)+24):std::min(desiredY,yFor(0)-ph-24);
        float py=juce::jlimit(plot.getY()+8,plot.getBottom()-ph-8,desiredY);
        panel={px,py,pw,ph};panelFrequency=float(p.frequency);panelBand=selected;
    }
    const int x=int(panel.getX()),y=int(panel.getY());
    solo.setBounds(x+443,y+10,100,28);closePanel.setBounds(x+574,y+10,30,28);
    frequencyLabel.setBounds(x+20,y+46,150,20);gainLabel.setBounds(x+180,y+46,150,20);qLabel.setBounds(x+340,y+46,150,20);
    frequency.setBounds(x+20,y+66,150,125);gain.setBounds(x+180,y+66,150,125);q.setBounds(x+340,y+66,150,125);
    shapeButton.setBounds(x+16,y+210,140,30);slopeButton.setBounds(x+165,y+210,106,30);placement.setBounds(x+280,y+210,125,30);modeSettings.setBounds(x+414,y+210,126,30);remove.setBounds(x+550,y+210,54,30);remove.setButtonText("Del");
    modeSettings.getProperties().remove("icon");modeSettings.setButtonText(juce::StringArray{"Static","Dynamic","Spectral"}[p.mode]);
    threshold.getProperties().set("threshold",true);threshold.setSliderStyle(juce::Slider::LinearVertical);threshold.setTextBoxStyle(juce::Slider::TextBoxBelow,false,100,22);threshold.setBounds(x+503,y+70,100,130);thresholdLabel.setBounds(x+503,y+46,100,20);
    rangeLabel.setBounds(x+16,y+252,135,18);dynamicRange.getProperties().set("rangeRing",false);dynamicRange.setSliderStyle(juce::Slider::LinearHorizontal);dynamicRange.setTextBoxStyle(juce::Slider::TextBoxRight,false,56,20);dynamicRange.setBounds(x+16,y+275,135,24);
    attackLabel.setBounds(x+163,y+252,114,18);attack.setSliderStyle(juce::Slider::LinearHorizontal);attack.setTextBoxStyle(juce::Slider::TextBoxRight,false,56,20);attack.setBounds(x+163,y+275,114,24);
    releaseLabel.setBounds(x+289,y+252,114,18);release.setSliderStyle(juce::Slider::LinearHorizontal);release.setTextBoxStyle(juce::Slider::TextBoxRight,false,56,20);release.setBounds(x+289,y+275,114,24);triggerButton.setBounds(x+417,y+264,85,28);externalKey.setBounds(x+512,y+264,92,28);
    processingButton.setBounds(24,h-73,118,26);resolutionButton.setBounds(150,h-73,92,26);analyzer.setBounds(250,h-73,140,26);freeze.setBounds(398,h-73,60,26);range.setBounds(466,h-73,60,26);
    for(auto* label:{&outputPanLabel,&wetMixLabel,&outputGainLabel})label->setJustificationType(juce::Justification::centred);
    outputPanLabel.setBounds(w-322,h-88,92,18);outputBalance.setBounds(w-322,h-70,92,68);
    wetMixLabel.setBounds(w-220,h-88,92,18);wetMix.setBounds(w-220,h-70,92,68);
    outputGainLabel.setBounds(w-118,h-88,92,18);outputKnob.setBounds(w-118,h-70,92,68);
    outputBalanceMode.setBounds(300,h-26,120,22);outputInvert.setBounds(430,h-26,90,22);
    midSidePanel.setBounds(w-344,h-298,318,200);midSideTitle.setBounds(15,6,288,25);
    midLabel.setBounds(18,35,130,20);sideLabel.setBounds(170,35,130,20);midOutput.setBounds(18,56,130,105);sideOutput.setBounds(170,56,130,105);midInvert.setBounds(23,168,130,26);sideInvert.setBounds(175,168,130,26);
    midSidePanel.toFront(false);
    add.setBounds(180,h-26,105,22);add.setButtonText("+ Add band");
    for(auto& card:bandCards)if(card)card->setVisible(false);
    for(auto* c:std::initializer_list<juce::Component*>{&previousPage,&nextPage,&pageLabel,&bandPicker,&active,&shape,&slope,&mode,&shapeLabel,&slopeLabel,&outputLabel,&save,&load,&reset,&bypass,&nodeActions,&output,&outputPanel})c->setVisible(false);
    for(auto* c:std::initializer_list<juce::Component*>{&solo,&closePanel,&frequency,&gain,&q,&frequencyLabel,&gainLabel,&qLabel,&shapeButton,&slopeButton,&remove,&modeSettings,&placement})c->setVisible(visible);
    for(auto* c:std::initializer_list<juce::Component*>{&threshold,&dynamicRange,&attack,&release,&thresholdLabel,&rangeLabel,&attackLabel,&releaseLabel,&externalKey,&triggerButton})c->setVisible(visible&&expanded);
    const auto transform=juce::AffineTransform::translation(float(-x),float(-y)).scaled(panelScale).translated(float(x),float(y));
    for(auto* c:std::initializer_list<juce::Component*>{&solo,&closePanel,&frequency,&gain,&q,&frequencyLabel,&gainLabel,&qLabel,&shapeButton,&slopeButton,&remove,&modeSettings,&placement,&threshold,&dynamicRange,&attack,&release,&thresholdLabel,&rangeLabel,&attackLabel,&releaseLabel,&externalKey,&triggerButton})c->setTransform(transform);
    resolutionButton.setVisible(int(eqProcessor.parameters.getRawParameterValue("processingPhase")->load())==2);
}
void EQEditor::updateMidSideControls(){
    const bool stereo=eqProcessor.getMainBusNumOutputChannels()>1,ms=eqProcessor.parameters.getRawParameterValue("balanceMode")->load()>.5f;
    outputPanLabel.setText(ms?"MID / SIDE":"BALANCE",juce::dontSendNotification);
    midSidePanel.setVisible(stereo&&ms);if(midSidePanel.isVisible())midSidePanel.toFront(false);
}
void EQEditor::updateNodeAction(){nodeActions.setVisible(false);}
void EQEditor::timerCallback(){const int phaseMode=int(eqProcessor.parameters.getRawParameterValue("processingPhase")->load());processingButton.setButtonText(juce::StringArray{"Live","Analog FIR","Linear FIR"}[phaseMode]);resolutionButton.setVisible(phaseMode==2);resolutionButton.setButtonText(juce::StringArray{"Low","Medium","High","Very High","Maximum"}[int(eqProcessor.parameters.getRawParameterValue("linearResolution")->load())]);const auto mouse=getLocalPoint(nullptr,juce::Desktop::getInstance().getMainMouseSource().getScreenPosition().toInt());const bool outputHover=output.getBounds().expanded(8).contains(mouse)||(outputPanel.isVisible()&&outputPanel.getBounds().expanded(6).contains(mouse));juce::ignoreUnused(outputHover);outputPanel.setVisible(false);outputBalance.setEnabled(eqProcessor.getMainBusNumOutputChannels()>1);outputBalanceMode.setEnabled(eqProcessor.getMainBusNumOutputChannels()>1);const int balanceDisplayMode=int(eqProcessor.parameters.getRawParameterValue("balanceMode")->load());if(int(outputBalance.getProperties().getWithDefault("displayMode",-1))!=balanceDisplayMode){outputBalance.getProperties().set("displayMode",balanceDisplayMode);outputBalance.updateText();}updateMidSideControls();const auto currentBand=eqProcessor.settings(selected);if(shapeButton.isVisible()!=(currentBand.enabled&&!panelDismissed)||(!knobDragging&&(panelBand!=selected||panelFrequency!=float(currentBand.frequency))))resized();solo.setToggleState(eqProcessor.auditionBand()>=0,juce::dontSendNotification);for(auto& card:bandCards)if(card)card->refresh();updateNodeAction();auto current=captureState();if(current!=observedState){observedState=std::move(current);stableTicks=0;}else if(++stableTicks>=10&&!dragging&&!frequency.isMouseButtonDown()&&!gain.isMouseButtonDown()&&!q.isMouseButtonDown()&&!threshold.isMouseButtonDown()&&!dynamicRange.isMouseButtonDown()&&!attack.isMouseButtonDown()&&!release.isMouseButtonDown()&&!output.isMouseButtonDown())commitHistory();undo.setEnabled(historyPosition>0||captureState()!=history[size_t(historyPosition)]);redo.setEnabled(historyPosition+1<int(history.size()));threshold.getProperties().set("detectorDb",eqProcessor.detectorLevel[size_t(selected)].load());thresholdLabel.setText("THRESHOLD",juce::dontSendNotification);if(analyzerMode!=0||showSidechain)updateAnalysis();else updateMeters();const auto p=eqProcessor.settings(selected);if(expanded!=(p.mode!=0)){expanded=p.mode!=0;resized();}modeSettings.setButtonText(juce::StringArray{"Static","Dynamic","Spectral"}[p.mode]);triggerButton.setEnabled(p.mode==1);triggerButton.setButtonText(p.mode==1&&p.freeTrigger?"Free":"Band");triggerButton.setTooltip("Trigger: "+juce::String(std::min(p.triggerLow,p.triggerHigh),0)+"–"+juce::String(std::max(p.triggerLow,p.triggerHigh),0)+" Hz (Free available in Dynamic mode)");const bool cut=p.shape==distillazione::Shape::LowCut||p.shape==distillazione::Shape::HighCut;shapeButton.getProperties().set("shape",int(p.shape));shapeButton.setButtonText(shapeNames[int(p.shape)]);for(auto* knob:std::initializer_list<juce::Slider*>{&frequency,&gain,&q})knob->setColour(juce::Slider::rotarySliderFillColourId,colour(selected));slope.setEnabled(cut);slopeButton.setEnabled(cut);slopeButton.setButtonText(juce::StringArray{"6","12","24","36","48","72","96"}[juce::jlimit(0,6,p.slope)]+" dB/oct");gain.setEnabled(p.shape==distillazione::Shape::Bell||p.shape==distillazione::Shape::LowShelf||p.shape==distillazione::Shape::HighShelf||p.shape==distillazione::Shape::TiltShelf||p.shape==distillazione::Shape::FlatTilt);repaint();}
void EQEditor::mouseDown(const juce::MouseEvent& e) {
    if(e.eventComponent==&output){outputSticky=!outputSticky;outputPanel.setVisible(outputSticky);return;}
    if(!plot.contains(e.position)||(eqProcessor.settings(selected).enabled&&!panelDismissed&&panel.contains(e.position)))return;
    int nearest=-1;float distance=24;
    for(int i=0;i<24;++i){auto p=eqProcessor.settings(i);if(!p.enabled)continue;float d=e.position.getDistanceFrom({xFor(p.frequency),juce::jlimit(plot.getY()+8,plot.getBottom()-26,yFor(p.gain))});if(d<distance){nearest=i;distance=d;}}
    if(nearest<0)return;select(nearest);if(e.mods.isPopupMenu()){showBandMenu();return;}dragging=true;
    for(auto* field:{"frequency","gain"})eqProcessor.parameters.getParameter(EQProcessor::bandID(selected,field))->beginChangeGesture();
}
void EQEditor::mouseDrag(const juce::MouseEvent& e){if(!dragging)return;auto set=[this](const char* field,float value){auto* p=eqProcessor.parameters.getParameter(EQProcessor::bandID(selected,field));p->setValueNotifyingHost(p->convertTo0to1(value));};set("frequency",float(hzFor(e.position.x)));if(gain.isEnabled())set("gain",float(dbFor(e.position.y)));resized();repaint();}
void EQEditor::mouseUp(const juce::MouseEvent&){if(!dragging)return;for(auto* field:{"frequency","gain"})eqProcessor.parameters.getParameter(EQProcessor::bandID(selected,field))->endChangeGesture();dragging=false;resized();repaint();}
void EQEditor::addBand(float hz,float db){for(int i=0;i<24;++i)if(!eqProcessor.settings(i).enabled){distillazione::BandSettings p;p.enabled=true;p.frequency=hz;p.gain=db;eqProcessor.copyBand(i,p);select(i);return;}}
void EQEditor::mouseDoubleClick(const juce::MouseEvent& e){if(plot.contains(e.position)&&!(eqProcessor.settings(selected).enabled&&!panelDismissed&&panel.contains(e.position))){mouseUp(e);addBand(float(hzFor(e.position.x)),float(dbFor(e.position.y)));}}
void EQEditor::mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel){if(plot.contains(e.position)&&!(eqProcessor.settings(selected).enabled&&!panelDismissed&&panel.contains(e.position))){auto p=eqProcessor.settings(selected);eqProcessor.setValue(EQProcessor::bandID(selected,"q"),float(p.q*std::pow(2.0,double(wheel.deltaY)*2)));}}
void EQEditor::preset(bool saving) {
    chooser=std::make_unique<juce::FileChooser>(saving?"Save EQ preset":"Load EQ preset",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),"*.deq");
    auto safe=juce::Component::SafePointer<EQEditor>(this);
    const int flags=saving?(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting):(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles);
    chooser->launchAsync(flags,[safe,saving](const juce::FileChooser& fc){if(!safe)return;auto file=fc.getResult();if(file==juce::File{})return;if(saving){juce::MemoryBlock data;safe->eqProcessor.getStateInformation(data);file.withFileExtension("deq").replaceWithData(data.getData(),data.getSize());}else{juce::MemoryBlock data;if(file.loadFileAsData(data))safe->eqProcessor.setStateInformation(data.getData(),int(data.getSize()));}});
}

juce::Image EQEditor::shapeIcon(int index) const {return curveIcon(index,juce::Colour(0xffd8d1dc));}
juce::PopupMenu EQEditor::shapeMenu(int baseID) const {juce::PopupMenu menu;for(int i=0;i<10;++i)menu.addItem(baseID+i,shapeNames[i],true,int(eqProcessor.settings(selected).shape)==i,shapeIcon(i));return menu;}
void EQEditor::showShapeMenu(){auto menu=shapeMenu(100);menu.setLookAndFeel(&menuStyle());auto safe=juce::Component::SafePointer<EQEditor>(this);const int band=selected;menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetComponent(&shapeButton).withMinimumWidth(220).withStandardItemHeight(34),[safe,band](int result){if(safe&&result>=100&&result<110)safe->eqProcessor.setValue(EQProcessor::bandID(band,"shape"),float(result-100));});}
void EQEditor::showBandMenu(){
    juce::PopupMenu menu;menu.setLookAndFeel(&menuStyle());auto p=eqProcessor.settings(selected);
    menu.addItem(1,p.enabled?"Disable":"Enable");menu.addItem(2,"Invert Gain");menu.addItem(3,p.mode==1?"Make Static":"Make Dynamic",true,p.mode==1);menu.addItem(4,p.mode==2?"Make Static":"Make Spectral",true,p.mode==2);menu.addSeparator();
    menu.addSubMenu("Shape",shapeMenu(100));juce::PopupMenu slopes;for(int i=0;i<7;++i)slopes.addItem(200+i,juce::StringArray{"6","12","24","36","48","72","96"}[i]+" dB/oct",true,p.slope==i);menu.addSubMenu("Slope",slopes,p.shape==distillazione::Shape::LowCut||p.shape==distillazione::Shape::HighCut);
    juce::PopupMenu stereo;for(int route=0;route<5;++route)stereo.addItem(300+route,juce::StringArray{"Stereo","Left","Right","Mid","Side"}[route],true,p.placement==route);menu.addSubMenu("Stereo Placement",stereo);menu.addSeparator();
    menu.addItem(5,"Cut");menu.addItem(6,"Copy");menu.addItem(7,"Paste",copiedBand.has_value());menu.addSeparator();bool freeSlot=false;for(int slot=0;slot<24;++slot)if(!eqProcessor.settings(slot).enabled)freeSlot=true;menu.addItem(8,"Split",freeSlot);menu.addItem(9,"Delete");
    auto safe=juce::Component::SafePointer<EQEditor>(this);const int band=selected;
    menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetComponent(this).withMousePosition().withMinimumWidth(170).withStandardItemHeight(24),[safe,band](int result){if(!safe||result==0)return;auto& processor=safe->eqProcessor;auto settings=processor.settings(band);
        if(result==3||result==4)processor.setValue(EQProcessor::bandID(band,"mode"),settings.mode==result-2?0:float(result-2));
        if(result==8)processor.splitBand(band);
        if(result>=300&&result<=304)processor.setValue(EQProcessor::bandID(band,"placement"),float(result-300));
        if(result==1)processor.setValue(EQProcessor::bandID(band,"on"),settings.enabled?0:1);
        if(result==2){processor.setValue(EQProcessor::bandID(band,"gain"),float(-settings.gain));if(settings.mode!=0)processor.setValue(EQProcessor::bandID(band,"range"),float(-settings.range));}
        if(result==5||result==6)safe->copiedBand=settings;
        if(result==5||result==9)processor.setValue(EQProcessor::bandID(band,"on"),0);
        if(result==7&&safe->copiedBand){auto copy=*safe->copiedBand;copy.enabled=true;processor.copyBand(band,copy);}
        if(result>=100&&result<110)processor.setValue(EQProcessor::bandID(band,"shape"),float(result-100));if(result>=200&&result<207)processor.setValue(EQProcessor::bandID(band,"slope"),float(result-200));
    });
}

std::vector<float> EQEditor::captureState() const {std::vector<float> state;for(auto* parameter:eqProcessor.getParameters())state.push_back(parameter->getValue());return state;}
void EQEditor::restoreState(const std::vector<float>& state){auto& parameters=eqProcessor.getParameters();for(int i=0;i<parameters.size()&&i<int(state.size());++i){parameters[i]->beginChangeGesture();parameters[i]->setValueNotifyingHost(state[size_t(i)]);parameters[i]->endChangeGesture();}observedState=captureState();stableTicks=0;select(selected);}
void EQEditor::commitHistory(){auto state=captureState();if(state==history[size_t(historyPosition)])return;history.resize(size_t(historyPosition+1));history.push_back(std::move(state));if(history.size()>100)history.erase(history.begin());historyPosition=int(history.size())-1;}
void EQEditor::showPresetMenu(){juce::PopupMenu menu;menu.setLookAndFeel(&menuStyle());menu.addItem(1,"Save preset...");menu.addItem(2,"Load preset...");menu.addSeparator();menu.addItem(3,"Default Setting");menu.addItem(4,"Bypass",true,bypass.getToggleState());auto safe=juce::Component::SafePointer<EQEditor>(this);menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetComponent(&presetMenu).withMinimumWidth(220),[safe](int result){if(!safe)return;safe->commitHistory();if(result==1)safe->preset(true);if(result==2)safe->preset(false);if(result==3)safe->reset.onClick();if(result==4)safe->eqProcessor.setValue("bypass",safe->bypass.getToggleState()?0:1);safe->commitHistory();});}

void EQEditor::updateMeters(){const double now=juce::Time::getMillisecondCounterHiRes();const float seconds=float(lastMeterTime>0?juce::jlimit(.001,.1,(now-lastMeterTime)/1000):1.0/60);lastMeterTime=now;for(int ch=0;ch<2;++ch){const float db=juce::Decibels::gainToDecibels(eqProcessor.meterPeak[size_t(ch)].exchange(0),-100.0f);meterDisplayDb[size_t(ch)]=std::max(db,std::max(-100.0f,meterDisplayDb[size_t(ch)]-25*seconds));}}
