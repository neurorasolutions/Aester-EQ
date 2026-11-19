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
    if(button.getButtonText().isNotEmpty()){
        auto r=button.getLocalBounds().toFloat().reduced(1);
        const bool on=button.getToggleState();
        if(on){
            g.setColour(juce::Colour(0x4422e4ff));g.fillRoundedRectangle(r.expanded(1.5f),6.5f);
            juce::ColourGradient activeGrad(juce::Colour(0xff3cebff),r.getX(),r.getY(),juce::Colour(0xff14b8d4),r.getX(),r.getBottom(),false);
            g.setGradientFill(activeGrad);g.fillRoundedRectangle(r,5);
            g.setColour(juce::Colour(0xffb8f7ff));g.drawRoundedRectangle(r,5,0.9f);
            const float dotX=r.getX()+10,dotY=r.getCentreY();
            g.setColour(juce::Colour(0xff06121a));g.fillEllipse(dotX-3,dotY-3,6,6);
            g.setColour(juce::Colour(0xff06121a));
            g.setFont(juce::FontOptions(11,juce::Font::bold));
            g.drawText(button.getButtonText(),r.withTrimmedLeft(18).toNearestInt(),juce::Justification::centredLeft);
            return;
        }
        g.setColour(down?juce::Colour(0xff1d2b38):over?juce::Colour(0xff17232e):juce::Colour(0xff111922));
        g.fillRoundedRectangle(r,5);
        g.setColour(juce::Colour(over?0xff365066:0xff243544));
        g.drawRoundedRectangle(r,5,0.8f);
        const float dotX=r.getX()+10,dotY=r.getCentreY();
        g.setColour(juce::Colour(0xff425768));
        g.fillEllipse(dotX-3,dotY-3,6,6);
        g.setColour(juce::Colour(over?0xffcfe3f2:0xff9bb0c0));
        g.setFont(juce::FontOptions(11,juce::Font::plain));
        g.drawText(button.getButtonText(),r.withTrimmedLeft(18).toNearestInt(),juce::Justification::centredLeft);
        return;
    }
    float cx=button.getWidth()*.5f,cy=button.getHeight()*.5f;juce::Path arc;arc.addCentredArc(cx,cy,6,6,0,.55f,juce::MathConstants<float>::twoPi-.55f,true);
    g.setColour(button.getToggleState()?juce::Colour(0xff22e4ff):juce::Colour(0xff5a6d7e));g.strokePath(arc,juce::PathStrokeType(1.4f));g.drawLine(cx,cy-8,cx,cy-1,1.4f);
}
void AesterLookAndFeel::drawButtonText(juce::Graphics& g,juce::TextButton& button,bool over,bool down) {
    if(bool(button.getProperties()["rangeButton"])){
        g.setColour(juce::Colour(over||down?0xffffd966:0xfff5c242));
        g.setFont(juce::FontOptions(12.0f,juce::Font::bold));
        g.drawText(button.getButtonText(),button.getLocalBounds(),juce::Justification::centred);
        return;
    }
    if(button.getProperties().contains("icon")){auto icon=button.getProperties()["icon"].toString();auto cx=button.getWidth()*.5f,cy=button.getHeight()*.5f;g.setColour(button.isEnabled()?juce::Colour(over?0xffe8f6ff:0xff9eb4c4):juce::Colour(0xff41505d));if(icon=="settings"){for(int i=-1;i<=1;++i)g.fillEllipse(cx+float(i*5)-1.5f,cy-1.5f,3,3);}else{bool reverse=icon=="redo";const float direction=reverse?1.0f:-1.0f;juce::Path path;path.startNewSubPath(cx-direction*6,cy+6);path.cubicTo(cx-direction*11,cy-7,cx+direction*4,cy-10,cx+direction*8,cy-3);g.strokePath(path,juce::PathStrokeType(1.7f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));juce::Path arrow;arrow.startNewSubPath(cx+direction*8,cy-3);arrow.lineTo(cx+direction*1,cy-4);arrow.lineTo(cx+direction*7,cy-10);arrow.closeSubPath();g.fillPath(arrow);}return;}
    if(!button.getProperties().contains("shape")){
        const bool toggled=button.getToggleState();
        g.setColour(toggled?juce::Colour(0xff0b1016):button.isEnabled()?juce::Colour(over?0xfff0f8ff:0xffcfe0ec):juce::Colour(0xff51616e));
        g.setFont(juce::FontOptions(11.5f,toggled?juce::Font::bold:juce::Font::plain));
        g.drawText(button.getButtonText(),button.getLocalBounds().reduced(4,1),juce::Justification::centred);
        return;
    }
    g.drawImageWithin(curveIcon(int(button.getProperties()["shape"]),juce::Colour(0xff22e4ff)),5,4,25,20,juce::RectanglePlacement::centred);
    g.setColour(juce::Colour(0xffd8eaf6));g.setFont(juce::FontOptions(11));g.drawText(button.getButtonText(),33,1,button.getWidth()-36,button.getHeight()-2,juce::Justification::left);
}
AesterLookAndFeel::AesterLookAndFeel() {
    setColour(juce::Slider::trackColourId,juce::Colour(0xff1e3545));
    setColour(juce::Slider::thumbColourId,juce::Colour(0xff22e4ff));
    setColour(juce::Slider::backgroundColourId,juce::Colour(0xff0e1720));
    setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff0a1118));
    setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff1e3040));
    setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff111b24));
    setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff263a4b));
    setColour(juce::ComboBox::textColourId,juce::Colour(0xffd8eaf6));
    setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xf2111821));
    setColour(juce::PopupMenu::textColourId,juce::Colour(0xffdceaf5));
    setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0x3322e4ff));
    setColour(juce::PopupMenu::highlightedTextColourId,juce::Colour(0xffffffff));
    setColour(juce::TextButton::textColourOffId,juce::Colour(0xffcfe0ec));
    setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffe6f4ff));
    setColour(juce::ToggleButton::textColourId,juce::Colour(0xffb8ccd9));
    setColour(juce::ToggleButton::tickColourId,juce::Colour(0xff22e4ff));
}
void AesterLookAndFeel::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool over,bool down) {
    auto r=b.getLocalBounds().toFloat().reduced(1);
    if(bool(b.getProperties()["rangeButton"])){
        juce::ColourGradient pill(juce::Colour(over?0xff242c36:0xff1a222c),r.getX(),r.getY(),juce::Colour(0xff10161e),r.getX(),r.getBottom(),false);
        g.setGradientFill(pill);g.fillRoundedRectangle(r,6);
        g.setColour(juce::Colour(over||down?0xaaf5c242:0x55485b6e));
        g.drawRoundedRectangle(r,6,1.0f);
        return;
    }
    if(bool(b.getProperties()["toolbar"])){
        if(over||down){
            g.setColour(juce::Colour(down?0xff1e2d3b:0xff15212c));g.fillRoundedRectangle(r,6);
            g.setColour(juce::Colour(0x6622e4ff));g.drawRoundedRectangle(r,6,0.8f);
        }
        return;
    }
    const bool on=b.getToggleState();
    if(on){
        g.setColour(juce::Colour(0x44f5c242));g.fillRoundedRectangle(r.expanded(1.5f),7);
        juce::ColourGradient activeGrad(juce::Colour(0xfff5d142),r.getX(),r.getY(),juce::Colour(0xffd9a826),r.getX(),r.getBottom(),false);
        g.setGradientFill(activeGrad);g.fillRoundedRectangle(r,6);
        g.setColour(juce::Colour(0xffffebb3));g.drawRoundedRectangle(r,6,0.9f);
        return;
    }
    juce::ColourGradient grad(juce::Colour(down?0xff1f2f3d:over?0xff192633:0xff121b24),r.getX(),r.getY(),juce::Colour(down?0xff14202b:0xff0d141c),r.getX(),r.getBottom(),false);
    g.setGradientFill(grad);g.fillRoundedRectangle(r,6);
    g.setColour(juce::Colour(over?0xff38566e:0xff233443));g.drawRoundedRectangle(r,6,0.85f);
}
void AesterLookAndFeel::drawComboBox(juce::Graphics& g,int w,int h,bool down,int,int,int,int,juce::ComboBox& box) {
    auto r=juce::Rectangle<float>(0,0,float(w),float(h)).reduced(1);
    juce::ColourGradient grad(juce::Colour(down?0xff1d2d3b:0xff131d27),r.getX(),r.getY(),juce::Colour(0xff0d141c),r.getX(),r.getBottom(),false);
    g.setGradientFill(grad);g.fillRoundedRectangle(r,6);
    g.setColour(juce::Colour(box.isMouseOver()?0xff38566e:0xff243646));g.drawRoundedRectangle(r,6,0.85f);
    juce::Path arrow;const float ax=float(w)-14,ay=float(h)*.5f;
    arrow.startNewSubPath(ax-4,ay-2);arrow.lineTo(ax,ay+2.5f);arrow.lineTo(ax+4,ay-2);
    g.setColour(juce::Colour(0xff7ea8c4));g.strokePath(arrow,juce::PathStrokeType(1.4f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
}
void AesterLookAndFeel::drawPopupMenuBackground(juce::Graphics& g,int w,int h) {
    auto r=juce::Rectangle<float>(0,0,float(w),float(h)).reduced(0.5f);
    juce::ColourGradient glass(juce::Colour(0xd815202d),r.getX(),r.getY(),juce::Colour(0xe20a1018),r.getX(),r.getBottom(),false);
    g.setGradientFill(glass);g.fillRoundedRectangle(r,11);
    {juce::Graphics::ScopedSaveState s(g);juce::Path clip;clip.addRoundedRectangle(r,11);g.reduceClipRegion(clip);
        juce::ColourGradient sheen(juce::Colour(0x1effffff),r.getX(),r.getY(),juce::Colour(0x00ffffff),r.getX(),r.getY()+r.getHeight()*0.42f,false);
        g.setGradientFill(sheen);g.fillRect(r.withHeight(r.getHeight()*0.42f));
    }
    juce::ColourGradient rim(juce::Colour(0x55ffffff),r.getX(),r.getY(),juce::Colour(0x2a22e4ff),r.getRight(),r.getBottom(),false);
    g.setGradientFill(rim);g.drawRoundedRectangle(r,11,1.0f);
    g.setColour(juce::Colour(0x16ffffff));g.drawRoundedRectangle(r.reduced(1),10,0.6f);
}
void AesterLookAndFeel::drawPopupMenuItem(juce::Graphics& g,const juce::Rectangle<int>& area,bool isSeparator,bool isActive,bool isHighlighted,bool isTicked,bool hasSubMenu,const juce::String& text,const juce::String&,const juce::Drawable* icon,const juce::Colour*) {
    if(isSeparator){g.setColour(juce::Colour(0x3340586d));g.fillRect(area.reduced(10,0).withHeight(1).withCentre(area.getCentre()));return;}
    auto r=area.reduced(4,1).toFloat();
    if(isHighlighted&&isActive){
        g.setColour(juce::Colour(0x3022e4ff));g.fillRoundedRectangle(r,6);
        g.setColour(juce::Colour(0x6622e4ff));g.drawRoundedRectangle(r,6,0.7f);
    }
    if(isTicked){
        juce::Path tick;const float tx=r.getX()+12,ty=r.getCentreY();
        tick.startNewSubPath(tx-4,ty);tick.lineTo(tx-1,ty+3.5f);tick.lineTo(tx+5,ty-4);
        g.setColour(juce::Colour(0xfff5c242));g.strokePath(tick,juce::PathStrokeType(2.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    }
    int textLeft=int(r.getX())+26;
    if(icon!=nullptr){icon->drawWithin(g,juce::Rectangle<float>(r.getX()+24,r.getY()+3,28,r.getHeight()-6),juce::RectanglePlacement::centred,1.0f);textLeft+=32;}
    g.setColour(isActive?(isHighlighted?juce::Colours::white:juce::Colour(0xffdce9f5)):juce::Colour(0xff556674));
    g.setFont(juce::FontOptions(13.0f,isTicked?juce::Font::bold:juce::Font::plain));
    g.drawText(text,textLeft,area.getY(),area.getRight()-textLeft-16,area.getHeight(),juce::Justification::centredLeft);
    if(hasSubMenu){
        juce::Path sub;const float sx=r.getRight()-10,sy=r.getCentreY();
        sub.startNewSubPath(sx-3,sy-4);sub.lineTo(sx+2,sy);sub.lineTo(sx-3,sy+4);
        g.setColour(juce::Colour(0xff8aa7bd));g.strokePath(sub,juce::PathStrokeType(1.3f));
    }
}
void AesterLookAndFeel::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider& slider) {
    auto r=juce::Rectangle<float>(float(x),float(y),float(w),float(h)).reduced(7);
    if(bool(slider.getProperties()["rangeRing"])){
        const float radius=std::min(r.getWidth(),r.getHeight())*.5f,cx=r.getCentreX(),cy=r.getCentreY();
        juce::Path rail,arc;rail.addCentredArc(cx,cy,radius,radius,0,start,end,true);arc.addCentredArc(cx,cy,radius,radius,0,start,start+pos*(end-start),true);
        g.setColour(juce::Colour(0xff1e2d3b));g.strokePath(rail,juce::PathStrokeType(5));g.setColour(juce::Colour(0xffff5c7a));g.strokePath(arc,juce::PathStrokeType(5));
        const auto a=start+pos*(end-start);g.setColour(juce::Colour(0xfff5c242));g.drawLine(cx+std::sin(a)*(radius-4),cy-std::cos(a)*(radius-4),cx+std::sin(a)*(radius+4),cy-std::cos(a)*(radius+4),2);return;
    }
    const float radius=std::min(r.getWidth(),r.getHeight())*0.5f,cx=r.getCentreX(),cy=r.getCentreY();
    const float angle=start+pos*(end-start);
    const auto accent=slider.findColour(juce::Slider::rotarySliderFillColourId);
    // Outer track rail
    juce::Path rail,arc;
    rail.addCentredArc(cx,cy,radius+2.5f,radius+2.5f,0,start,end,true);
    arc.addCentredArc(cx,cy,radius+2.5f,radius+2.5f,0,start,angle,true);
    g.setColour(juce::Colour(0xff182532));
    g.strokePath(rail,juce::PathStrokeType(2.8f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    // Soft luminous glow + crisp neon arc
    g.setColour(accent.withAlpha(0.22f));
    g.strokePath(arc,juce::PathStrokeType(6.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    g.setColour(accent);
    g.strokePath(arc,juce::PathStrokeType(2.4f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    // Subtle precision drop shadow & machined titanium body
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.fillEllipse(cx-radius+1,cy-radius+3,2*radius,2*radius);
    juce::ColourGradient body(juce::Colour(0xff263340),cx-radius*0.3f,cy-radius,juce::Colour(0xff0e151d),cx+radius*0.3f,cy+radius,false);
    g.setGradientFill(body);g.fillEllipse(cx-radius,cy-radius,2*radius,2*radius);
    // Inner recessed bevel ring
    g.setColour(juce::Colour(0xff384d60).withAlpha(0.75f));
    g.drawEllipse(cx-radius,cy-radius,2*radius,2*radius,0.9f);
    g.setColour(juce::Colour(0xff0b1118));
    g.drawEllipse(cx-radius*0.76f,cy-radius*0.76f,radius*1.52f,radius*1.52f,0.8f);
    // Glowing pointer needle
    const float ix1=cx+std::sin(angle)*radius*0.42f,iy1=cy-std::cos(angle)*radius*0.42f;
    const float ix2=cx+std::sin(angle)*radius*0.86f,iy2=cy-std::cos(angle)*radius*0.86f;
    g.setColour(accent.withAlpha(0.45f));g.drawLine(ix1,iy1,ix2,iy2,3.4f);
    g.setColour(juce::Colour(0xfff4fbff));g.drawLine(ix1,iy1,ix2,iy2,1.6f);
}


void AesterLookAndFeel::drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float min,float max,juce::Slider::SliderStyle kind,juce::Slider& slider){
    if(!bool(slider.getProperties()["threshold"])) {LookAndFeel_V4::drawLinearSlider(g,x,y,w,h,pos,min,max,kind,slider);return;}
    const float cx=x+w*.5f;
    juce::ColourGradient colour(juce::Colour(0xffff4d6a),cx,float(y),juce::Colour(0xff1fd6a8),cx,float(y+h),false);
    colour.addColour(.45,juce::Colour(0xfff5c242));
    g.setGradientFill(colour);g.fillRoundedRectangle(cx-3,float(y),6,float(h),3);
    const float level=juce::jlimit(0.0f,1.0f,(float(slider.getProperties()["detectorDb"])+80)/80);
    g.setColour(juce::Colour(0xff0d151d));g.fillRoundedRectangle(cx-3,float(y),6,float(h)*(1-level),3);
    g.setColour(juce::Colour(0xff22e4ff).withAlpha(.85f));g.drawHorizontalLine(int(y+h*(1-level)),cx-5,cx+5);
    g.setColour(juce::Colour(0xff20303f));g.fillRoundedRectangle(cx-11,pos-5,22,10,4);
    g.setColour(juce::Colour(0xff22e4ff));g.drawRoundedRectangle(cx-11,pos-5,22,10,4,0.9f);
    g.setColour(juce::Colours::white);g.drawHorizontalLine(int(pos),cx-6,cx+6);
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
    range.getProperties().set("rangeButton",true);
    range.setTooltip("Display dB range: 3, 6, 12 or 30 dB");
    range.onClick=[this]{
        juce::PopupMenu menu;menu.setLookAndFeel(&menuStyle());
        const std::array<float,4> ranges{3.f,6.f,12.f,30.f};
        for(int i=0;i<4;++i)menu.addItem(i+1,juce::String(ranges[size_t(i)],0)+" dB",true,std::abs(displayRange-ranges[size_t(i)])<0.1f);
        auto safe=juce::Component::SafePointer<EQEditor>(this);
        menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this).withTargetComponent(&range).withMinimumWidth(98).withStandardItemHeight(30),[safe,ranges](int id){
            if(!safe||id<=0)return;
            safe->displayRange=ranges[size_t(id-1)];
            safe->range.setButtonText(juce::String(safe->displayRange,0)+" dB");
            safe->resized();safe->repaint();
        });
    };
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
    help.onClick=[this]{juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,"Aester EQ by DropDownAcademy", "Aester EQ is a precision equalizer by DropDownAcademy. https://www.dropdownacademy.com\n\nDouble-click the graph to add a band. Drag a node to set frequency and gain; scroll to change Q. Right-click a node for band actions. Use the band settings menu for Static, Dynamic or Spectral. Select a node to show its floating frequency, gain and Q controls. SOLO isolates its frequency region; Q sets the audition bandwidth. Click SOLO again to restore the full mix. Solo is not stored in presets and ends when this editor closes. Mix blends latency-aligned dry and wet signals; Freeze holds the analyzer display. Threshold, range, attack and release control dynamics. Live mode keeps Static and Dynamic at zero delay. Analog FIR uses an analog-response FIR with 320 samples of delay. Linear FIR offers five FIR resolutions with increasing delay. Spectral adds 1024 samples. The footer contains Mix, output, balance, polarity and processing phase controls. Undo/Redo and A/B are local to this editor.", "OK",this);};
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
    for(auto* slider:{&outputKnob,&outputBalance,&wetMix}){slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider->setTextBoxStyle(juce::Slider::TextBoxBelow,false,88,20);slider->setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xff22e4ff));slider->setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff1e3040));}
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
    outputInvert.setButtonText("INVERT L/R");outputInvert.setTooltip("Invert polarity of both Left and Right output channels. Use Mid / Side controls for independent polarity.");
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
    for(auto* knob:std::initializer_list<juce::Slider*>{&threshold,&dynamicRange}){knob->textFromValueFunction=[](double value){return juce::String(value,1);};knob->updateText();}
    for(auto* knob:std::initializer_list<juce::Slider*>{&attack,&release}){knob->textFromValueFunction=[](double value){return value>=10.0?juce::String(value,0):juce::String(value,1);};knob->updateText();}
    expanded=eqProcessor.settings(selected).mode!=0;
    activeAttachment=std::make_unique<ButtonAttachment>(eqProcessor.parameters,id("on"),active);
    frequency.textFromValueFunction=[](double value){return juce::String(value,1);};
    q.textFromValueFunction=[](double value){return juce::String(value,3);};
    frequency.updateText();q.updateText();
    shapeButton.getProperties().set("shape",int(eqProcessor.settings(selected).shape));shapeButton.setButtonText(shapeNames[int(eqProcessor.settings(selected).shape)]);
    for(auto* knob:std::initializer_list<juce::Slider*>{&frequency,&gain,&q})knob->setColour(juce::Slider::rotarySliderFillColourId,colour(selected));
    bandPicker.setSelectedId(selected+1,juce::dontSendNotification);resized();repaint();
}
juce::Colour EQEditor::colour(int band) const {if(band==selected)return juce::Colour(0xfff5c842);const juce::uint32 palette[]{0xff22e4ff,0xff2af598,0xffb86bff,0xffffaa33,0xffff5a79,0xff4d9eff,0xff56f0c8,0xffe879f9};return juce::Colour(palette[band%8]);}
float EQEditor::xFor(double hz) const {return plot.getX()+plot.getWidth()*float(std::log(juce::jlimit(10.0,30000.0,hz)/10)/std::log(3000.0));}
float EQEditor::yFor(double db) const {return plot.getY()+plot.getHeight()*.5f-float(db)*plot.getHeight()/(2*displayRange);}
double EQEditor::hzFor(float x) const {return 10*std::pow(3000.0,juce::jlimit(0.0f,1.0f,(x-plot.getX())/plot.getWidth()));}
double EQEditor::dbFor(float y) const {return juce::jlimit(-30.0,30.0,double((plot.getY()+plot.getHeight()*.5f-y)*2*displayRange/plot.getHeight()));}
void EQEditor::paint(juce::Graphics& g) {
    // Deep obsidian futuristic backdrop
    juce::ColourGradient bg(juce::Colour(0xff0b1017),0,0,juce::Colour(0xff070a0f),0,float(getHeight()),false);
    g.setGradientFill(bg);g.fillAll();

    // Top header bar
    g.setColour(juce::Colour(0xff0e151f));g.fillRect(0,0,getWidth(),56);
    g.setColour(juce::Colour(0x3322e4ff));g.drawHorizontalLine(56,0,float(getWidth()));

    // Futuristic brand mark
    g.setColour(juce::Colour(0x4422e4ff));g.fillRoundedRectangle(20,17,6,24,3);
    g.setColour(juce::Colour(0xff22e4ff));g.fillRoundedRectangle(21,18,4,22,2);
    g.setColour(juce::Colour(0xffeef7ff));g.setFont(juce::FontOptions(22,juce::Font::bold));
    g.drawText("AESTER EQ",33,12,170,34,juce::Justification::left);
    g.setColour(juce::Colour(0xff6b8294));g.setFont(juce::FontOptions(10.5f,juce::Font::bold));
    g.drawText("DROPDOWNACADEMY",getWidth()-190,18,168,22,juce::Justification::right);

    // Cockpit plot surface
    auto frame=plot.expanded(4,4);
    juce::ColourGradient plotBg(juce::Colour(0xff0f1620),frame.getCentreX(),frame.getY(),juce::Colour(0xff090e14),frame.getCentreX(),frame.getBottom(),false);
    g.setGradientFill(plotBg);g.fillRoundedRectangle(frame,8);
    g.setColour(juce::Colour(0xff1c2d3d));g.drawRoundedRectangle(frame,8,1.0f);
    // Subtle futuristic HUD corner ticks
    g.setColour(juce::Colour(0x8822e4ff));
    const float cl=10.0f;
    g.drawLine(frame.getX()+1,frame.getY()+cl,frame.getX()+1,frame.getY()+1,1.3f);g.drawLine(frame.getX()+1,frame.getY()+1,frame.getX()+cl,frame.getY()+1,1.3f);
    g.drawLine(frame.getRight()-cl,frame.getY()+1,frame.getRight()-1,frame.getY()+1,1.3f);g.drawLine(frame.getRight()-1,frame.getY()+1,frame.getRight()-1,frame.getY()+cl,1.3f);
    g.drawLine(frame.getX()+1,frame.getBottom()-cl,frame.getX()+1,frame.getBottom()-1,1.3f);g.drawLine(frame.getX()+1,frame.getBottom()-1,frame.getX()+cl,frame.getBottom()-1,1.3f);
    g.drawLine(frame.getRight()-cl,frame.getBottom()-1,frame.getRight()-1,frame.getBottom()-1,1.3f);g.drawLine(frame.getRight()-1,frame.getBottom()-cl,frame.getRight()-1,frame.getBottom()-1,1.3f);

    bool hasBands=false;for(int i=0;i<24;++i)hasBands=hasBands||eqProcessor.settings(i).enabled;
    if(!hasBands){
        g.setColour(juce::Colour(0xff5e7588));g.setFont(juce::FontOptions(14.5f));
        g.drawText("Double-click the graph to add a band",int(plot.getCentreX())-200,int(plot.getCentreY())+28,400,28,juce::Justification::centred);
    }

    // Frequency grid lines
    g.setFont(juce::FontOptions(10.5f));
    for(double hz:{20.,30.,40.,50.,60.,80.,100.,150.,200.,300.,400.,500.,600.,800.,1000.,1500.,2000.,3000.,4000.,5000.,6000.,8000.,10000.,15000.,20000.}) {
        float x=xFor(hz);
        const bool major=(hz==100.||hz==1000.||hz==10000.);
        g.setColour(juce::Colour(major?0xff213647:0xff15232f).withAlpha(major?0.85f:0.65f));
        g.drawVerticalLine(int(x),plot.getY(),plot.getBottom());
    }
    for(double hz:{20.,50.,100.,200.,500.,1000.,2000.,5000.,10000.,20000.}) {
        g.setColour(juce::Colour(0xff6c8091));
        g.drawText(hz>=1000?juce::String(hz/1000,0)+"k":juce::String(hz,0),int(xFor(hz))-20,int(plot.getBottom())-19,40,16,juce::Justification::centred);
    }

    // Horizontal dB grid + Right-hand EQ dB Scale (amber/gold)
    const juce::String minusSign=juce::String::fromUTF8("\xe2\x88\x92");
    const int step=displayRange==30?6:displayRange==12?3:displayRange==6?2:1;
    g.setFont(juce::FontOptions(11.5f,juce::Font::bold));
    for(int db=-int(displayRange);db<=int(displayRange);db+=step){
        float y=yFor(db);
        g.setColour(juce::Colour(db==0?0xff2f4d63:0xff162430));
        g.drawHorizontalLine(int(y),plot.getX(),plot.getRight());
        if(db<int(displayRange)){
            const juce::String label=db>0?("+"+juce::String(db)):db<0?(minusSign+juce::String(-db)):juce::String("0");
            g.setColour(juce::Colour(db==0?0xffffdf7a:0xfff5c242));
            g.drawText(label,int(plot.getRight())+6,int(y)-9,34,18,juce::Justification::right);
        }
    }

    // Right-hand Analyzer / Meter dBFS scale (0 .. -90)
    g.setFont(juce::FontOptions(10.5f));
    for(int db=0;db>=-90;db-=10){
        float y=plot.getBottom()-float(db+100)*plot.getHeight()/100.0f;
        const juce::String label=db==0?juce::String("0"):(minusSign+juce::String(-db));
        g.setColour(juce::Colour(db==0?0xff9db0c0:0xff6d7f8f));
        g.drawText(label,int(plot.getRight())+44,int(y)-8,30,16,juce::Justification::right);
    }

    // Holographic spectrum analyzer
    if(analyzerMode!=0||showSidechain){juce::Graphics::ScopedSaveState saved(g);g.reduceClipRegion(plot.toNearestInt());
        for(int stage=0;stage<3;++stage){if((stage==2&&!showSidechain)||(stage<2&&analyzerMode==0)||(stage==0&&analyzerMode==2)||(stage==1&&analyzerMode==1))continue;const auto& display=stage==2?sidechainDisplay:spectrum;juce::Path trace;float previousX=0,previousY=0;
            for(int i=0;i<=600;++i){float x=plot.getX()+plot.getWidth()*float(i)/600;double bin=hzFor(x)*SpectrumTap::size/display.rate;const float rawDb=bin>SpectrumDisplay::bins-1?-100:display.displayDb(stage==2?0:stage,bin);float db=SpectrumDisplay::tiltedDb(rawDb,hzFor(x),analyzerTilt);const double edge=juce::jlimit(0.0,1.0,(double(SpectrumDisplay::bins-1)-bin)/4.0);db=float(10*std::log10(1e-10+std::max(0.0,std::pow(10.0,double(db)/10)-1e-10)*edge*edge));float y=plot.getBottom()-juce::jlimit(0.0f,1.0f,(db+100)/100)*plot.getHeight();if(i==0)trace.startNewSubPath(x,y);else trace.quadraticTo(previousX,previousY,(previousX+x)*.5f,(previousY+y)*.5f);previousX=x;previousY=y;if(i==600)trace.lineTo(x,y);}
            auto fill=trace;fill.lineTo(plot.getRight(),plot.getBottom());fill.lineTo(plot.getX(),plot.getBottom());fill.closeSubPath();
            juce::ColourGradient specGrad(juce::Colour(stage==2?0xff38bdf8:stage==0?0xff3b82f6:0xff22e4ff).withAlpha(stage==0?0.10f:0.16f),0,plot.getY(),juce::Colour(0x000a1420),0,plot.getBottom(),false);
            g.setGradientFill(specGrad);g.fillPath(fill);
            g.setColour(juce::Colour(stage==2?0xff60a5fa:stage==0?0xff5b7d99:0xff4be7ff).withAlpha(stage==0?0.48f:0.75f));
            g.strokePath(trace,juce::PathStrokeType(stage==1?1.25f:1.0f));
        }
    }

    // EQ Response Curves
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
            g.setColour(soloActive?juce::Colour(0xff526575).withAlpha(.16f):colour(band).withAlpha(.42f));
            if(!soloActive||band!=soloBand)g.strokePath(bandPaths[size_t(band)],juce::PathStrokeType(1.1f));
        }
        auto highlighted=soloActive?bandPaths[size_t(soloBand)]:individual;
        auto fill=highlighted;fill.lineTo(plot.getRight(),yFor(0));fill.lineTo(plot.getX(),yFor(0));fill.closeSubPath();
        if(soloActive){
            const auto p=eqProcessor.settings(soloBand);const double displayedGain=p.gain+(p.mode==1?eqProcessor.dynamicAmount[size_t(soloBand)].load():0);const float apex=juce::jlimit(plot.getY(),plot.getBottom(),yFor(std::abs(displayedGain)>.05?displayedGain:1));
            juce::ColourGradient gradient(juce::Colour(0xfff5c842).withAlpha(.45f),xFor(p.frequency),apex,juce::Colour(0xfff5c842).withAlpha(.015f),xFor(p.frequency),yFor(0),false);g.setGradientFill(gradient);g.fillPath(fill);
            g.setColour(juce::Colour(0xff6b7f90).withAlpha(.28f));g.strokePath(total,juce::PathStrokeType(1.4f));
            g.setColour(juce::Colour(0x44f5c842));g.strokePath(highlighted,juce::PathStrokeType(6.0f));
            g.setColour(juce::Colour(0xffffdf6e));g.strokePath(highlighted,juce::PathStrokeType(2.3f));
        }else{
            const auto selCol=colour(selected);
            juce::ColourGradient selFill(selCol.withAlpha(.26f),plot.getCentreX(),plot.getY(),selCol.withAlpha(.04f),plot.getCentreX(),yFor(0),false);
            g.setGradientFill(selFill);g.fillPath(fill);
            g.setColour(selCol.withAlpha(.65f));g.strokePath(individual,juce::PathStrokeType(1.3f));
            // Multi-pass luminous glow on total wet EQ curve
            g.setColour(juce::Colour(0x28f5c842));g.strokePath(total,juce::PathStrokeType(6.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
            g.setColour(juce::Colour(0x55ffe066));g.strokePath(total,juce::PathStrokeType(3.4f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
            g.setColour(juce::Colour(0xffffdf6e));g.strokePath(total,juce::PathStrokeType(2.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        }
    }

    // Band nodes with holographic glow + subtle crosshair while dragging
    for(int i=0;i<24;++i){
        auto p=eqProcessor.settings(i);if(!p.enabled)continue;
        const bool muted=soloActive&&i!=soloBand;
        auto point=juce::Point<float>{xFor(p.frequency),juce::jlimit(plot.getY()+8,plot.getBottom()-24,yFor(p.gain))};
        const auto c=colour(i);
        if(i==selected&&!muted){
            if(dragging){
                g.setColour(c.withAlpha(0.22f));
                g.drawVerticalLine(int(point.x),plot.getY(),plot.getBottom());
                g.drawHorizontalLine(int(point.y),plot.getX(),plot.getRight());
            }
            g.setColour(c.withAlpha(.16f));g.fillEllipse(point.x-15,point.y-15,30,30);
            g.setColour(c.withAlpha(.42f));g.drawEllipse(point.x-10,point.y-10,20,20,1.1f);
        }
        g.setColour(muted?juce::Colour(0xff5a6d7e).withAlpha(.22f):c);g.fillEllipse(point.x-5.5f,point.y-5.5f,11,11);
        g.setColour(juce::Colour(0xffffffff).withAlpha(muted?.15f:0.95f));g.drawEllipse(point.x-5.5f,point.y-5.5f,11,11,1.2f);
    }

    // Futuristic L/R Output Meters aligned with 0 .. -100 dBFS scale
    const float meterTop=plot.getY(),meterHeight=plot.getHeight();
    for(int channel=0;channel<2;++channel){
        const float level=meterDisplayDb[size_t(channel)],peakHold=meterPeakHoldDb[size_t(channel)];
        const float mx=float(getWidth()-28+channel*10);
        g.setColour(juce::Colour(0xff111a24));g.fillRoundedRectangle(mx,meterTop,7,meterHeight,3.5f);
        g.setColour(juce::Colour(0xff1e2f3f));g.drawRoundedRectangle(mx,meterTop,7,meterHeight,3.5f,0.7f);
        const float norm=juce::jlimit(0.0f,1.0f,(level+100.0f)/100.0f);
        const float height=meterHeight*norm;
        if(height>0.5f){
            juce::ColourGradient mg(juce::Colour(0xffff4d5a),mx,meterTop,juce::Colour(0xff18b88c),mx,meterTop+meterHeight,false);
            mg.addColour(0.10,juce::Colour(0xfff5c242));
            mg.addColour(0.35,juce::Colour(0xff7ce832));
            g.setGradientFill(mg);
            g.fillRoundedRectangle(mx+0.5f,meterTop+meterHeight-height,6.0f,height,2.5f);
        }
        if(peakHold>-99.0f){
            const float phNorm=juce::jlimit(0.0f,1.0f,(peakHold+100.0f)/100.0f);
            const float py=meterTop+meterHeight*(1.0f-phNorm);
            g.setColour(juce::Colour(peakHold>0.0f?0xffff5c70:0xff9cf542));
            g.fillRect(mx+0.5f,py-1.0f,6.0f,2.0f);
        }
        g.setFont(juce::FontOptions(9.5f,juce::Font::bold));g.setColour(juce::Colour(0xff7d93a5));
        g.drawText(channel==0?"L":"R",int(mx)-2,int(meterTop)-18,11,14,juce::Justification::centred);
    }

    // Bottom futuristic console dock
    g.setColour(juce::Colour(0xff0b1118));g.fillRect(0,getHeight()-88,getWidth(),88);
    g.setColour(juce::Colour(0x3322e4ff));g.drawHorizontalLine(getHeight()-88,0,float(getWidth()));
    g.setColour(juce::Colour(0xff6b8294));g.setFont(juce::FontOptions(10.0f));
    g.drawText(juce::String(count)+" / 24 BANDS  ·  "+juce::String(eqProcessor.getLatencySamples())+" samples",24,getHeight()-26,180,18,juce::Justification::left);
    g.setColour(juce::Colour(0xff4c6578));
    for(int i=0;i<3;++i){const float d=5.0f+float(i*5);g.drawLine(float(getWidth())-d-4,float(getHeight())-4,float(getWidth())-4,float(getHeight())-d-4,1.2f);}

    if(analyzerFrozen){
        g.setColour(juce::Colour(0xcc0d1722));g.fillRoundedRectangle(plot.getX()+10,plot.getY()+10,126,22,5);
        g.setColour(juce::Colour(0xff22e4ff));g.drawRoundedRectangle(plot.getX()+10,plot.getY()+10,126,22,5,0.8f);
        g.setFont(juce::FontOptions(10.0f,juce::Font::bold));g.drawText("SPECTRUM HELD",int(plot.getX())+10,int(plot.getY())+10,126,22,juce::Justification::centred);
    }
    if(eqProcessor.auditionBand()>=0){
        const float sy=plot.getY()+(analyzerFrozen?38.0f:10.0f);
        g.setColour(juce::Colour(0xcc1f1a0d));g.fillRoundedRectangle(plot.getX()+10,sy,126,22,5);
        g.setColour(juce::Colour(0xfff5c242));g.drawRoundedRectangle(plot.getX()+10,sy,126,22,5,0.9f);
        g.setFont(juce::FontOptions(10.0f,juce::Font::bold));g.drawText("SOLO · BAND "+juce::String(eqProcessor.auditionBand()+1),int(plot.getX())+10,int(sy),126,22,juce::Justification::centred);
    }

    // Floating futuristic Liquid Glass Band Inspector
    const auto selectedBand=eqProcessor.settings(selected);
    if(selectedBand.enabled&&!panelDismissed){
        g.setColour(juce::Colours::black.withAlpha(.42f));g.fillRoundedRectangle(panel.translated(0,6).expanded(3.5f),14.5f);
        // Translucent smoked glass body so grid & spectrum subtly shine through
        juce::ColourGradient cardBg(juce::Colour(0xc6152230),panel.getX(),panel.getY(),juce::Colour(0xd2091018),panel.getX(),panel.getBottom(),false);
        g.setGradientFill(cardBg);g.fillRoundedRectangle(panel,13);
        // Liquid specular top sheen + subtle diagonal refraction
        {juce::Graphics::ScopedSaveState saved(g);juce::Path clip;clip.addRoundedRectangle(panel,13);g.reduceClipRegion(clip);
            juce::ColourGradient topSheen(juce::Colour(0x24ffffff),panel.getX(),panel.getY(),juce::Colour(0x00ffffff),panel.getX(),panel.getY()+panel.getHeight()*0.44f,false);
            g.setGradientFill(topSheen);g.fillRect(panel.withHeight(panel.getHeight()*0.44f));
            juce::ColourGradient diagGlow(colour(selected).withAlpha(0.11f),panel.getX(),panel.getY(),juce::Colour(0x0022e4ff),panel.getX()+panel.getWidth()*0.55f,panel.getBottom(),false);
            g.setGradientFill(diagGlow);g.fillRect(panel);
            // Soft bottom internal bounce light
            juce::ColourGradient bottomBounce(juce::Colour(0x0022e4ff),panel.getX(),panel.getBottom()-24,juce::Colour(0x1422e4ff),panel.getX(),panel.getBottom(),false);
            g.setGradientFill(bottomBounce);g.fillRect(panel.withTrimmedTop(panel.getHeight()-24));
        }
        // Prismatic specular rim border (bright top-left catchlight fading to accent tint)
        juce::ColourGradient rimGrad(juce::Colour(0x66ffffff),panel.getX(),panel.getY(),colour(selected).withAlpha(0.38f),panel.getRight(),panel.getBottom(),false);
        g.setGradientFill(rimGrad);g.drawRoundedRectangle(panel,13,1.15f);
        g.setColour(juce::Colour(0x1affffff));g.drawRoundedRectangle(panel.reduced(1.2f),11.8f,0.65f);
        // Header neon bar & title
        g.setColour(colour(selected).withAlpha(0.38f));g.fillRoundedRectangle(panel.getX()+11,panel.getY()+11,6,18,3);
        g.setColour(colour(selected));g.fillRoundedRectangle(panel.getX()+12,panel.getY()+12,4,16,2);
        g.setColour(juce::Colour(0xffeaf6ff));g.setFont(juce::FontOptions(12.0f,juce::Font::bold));
        g.drawText("BAND "+juce::String(selected+1),int(panel.getX())+22,int(panel.getY())+8,90,24,juce::Justification::left);
        // Subtle Liquid Glass HUD readout chip near active dot
        const float nx=xFor(selectedBand.frequency),ny=juce::jlimit(plot.getY()+8,plot.getBottom()-24,yFor(selectedBand.gain));
        juce::Rectangle<float> tip(juce::jlimit(plot.getX()+4,plot.getRight()-118,nx-57),juce::jlimit(plot.getY()+4,plot.getBottom()-46,ny-52),114,38);
        if(!tip.intersects(panel)){
            juce::ColourGradient tipBg(juce::Colour(0xc8162331),tip.getX(),tip.getY(),juce::Colour(0xd40a1119),tip.getX(),tip.getBottom(),false);
            g.setGradientFill(tipBg);g.fillRoundedRectangle(tip,8);
            {juce::Graphics::ScopedSaveState s(g);juce::Path tc;tc.addRoundedRectangle(tip,8);g.reduceClipRegion(tc);
                juce::ColourGradient ts(juce::Colour(0x20ffffff),tip.getX(),tip.getY(),juce::Colour(0x00ffffff),tip.getX(),tip.getCentreY(),false);
                g.setGradientFill(ts);g.fillRect(tip.withHeight(tip.getHeight()*0.5f));
            }
            juce::ColourGradient tipRim(juce::Colour(0x55ffffff),tip.getX(),tip.getY(),colour(selected).withAlpha(0.55f),tip.getRight(),tip.getBottom(),false);
            g.setGradientFill(tipRim);g.drawRoundedRectangle(tip,8,0.95f);
            g.setFont(juce::FontOptions(11.0f,juce::Font::bold));g.setColour(juce::Colour(0xffeaf6ff));
            const juce::String freqTxt=selectedBand.frequency>=1000.0?juce::String(selectedBand.frequency/1000.0,2)+" kHz":juce::String(selectedBand.frequency,1)+" Hz";
            g.drawText(freqTxt,tip.toNearestInt().removeFromTop(19),juce::Justification::centred);
            g.setFont(juce::FontOptions(10.5f));g.setColour(colour(selected));
            const juce::String gainTxt=(selectedBand.gain>=0?"+":"")+juce::String(selectedBand.gain,2)+" dB";
            g.drawText(gainTxt,tip.toNearestInt().withTrimmedTop(19),juce::Justification::centred);
        }
    }
}

void EQEditor::resized() {
    const int w=getWidth(),h=getHeight();eqProcessor.editorWidth.store(w);eqProcessor.editorHeight.store(h);
    // Reserve clean right margin for EQ dB scale + Analyzer dBFS scale + L/R output meters
    plot={24,82,float(w-134),float(h-184)};

    // Top bar controls
    undo.setBounds(214,14,30,28);redo.setBounds(248,14,30,28);
    presetMenu.setBounds(290,14,std::max(140,w-780),28);
    ab.setBounds(w-470,14,62,28);copyAB.setBounds(w-402,14,50,28);
    help.setBounds(w-340,14,56,28);resizeEditor.setBounds(w-278,14,54,28);

    // Right-hand dB range selector pill (aligned directly over amber EQ dB scale)
    range.setBounds(int(plot.getRight())-4,int(plot.getY())-28,52,22);

    // Smart Left / Right opposite-side docking for Band Inspector (never covers the band node)
    const auto p=eqProcessor.settings(selected);const bool visible=p.enabled&&!panelDismissed;
    const float panelScale=juce::jlimit(0.68f,0.92f,std::min(float(w)/1280.0f,float(h)/840.0f));
    const float baseW=460.0f,baseH=expanded?280.0f:224.0f;
    const float pw=baseW*panelScale,ph=baseH*panelScale;
    if(!knobDragging||panel.isEmpty()){
        const float nodeX=xFor(p.frequency);
        const float splitX=xFor(1000.0)+1.0f;
        const bool dockRight=(nodeX<=splitX);
        float px=dockRight?(plot.getRight()-pw-10.0f):(plot.getX()+10.0f);
        px=juce::jlimit(plot.getX()+6.0f,plot.getRight()-pw-6.0f,px);
        float py=juce::jlimit(plot.getY()+8.0f,plot.getBottom()-ph-24.0f,plot.getBottom()-ph-24.0f);
        panel={px,py,pw,ph};panelFrequency=float(p.frequency);panelBand=selected;
    }
    const int x=int(panel.getX()),y=int(panel.getY());
    solo.setBounds(x+326,y+8,86,26);closePanel.setBounds(x+420,y+8,28,26);
    if(expanded){
        frequencyLabel.setBounds(x+12,y+40,106,18);gainLabel.setBounds(x+124,y+40,106,18);qLabel.setBounds(x+236,y+40,106,18);
        frequency.setBounds(x+12,y+58,106,114);gain.setBounds(x+124,y+58,106,114);q.setBounds(x+236,y+58,106,114);
        thresholdLabel.setBounds(x+350,y+40,98,18);
        threshold.getProperties().set("threshold",true);threshold.setSliderStyle(juce::Slider::LinearVertical);threshold.setTextBoxStyle(juce::Slider::TextBoxBelow,false,94,20);threshold.setBounds(x+350,y+58,98,114);
    }else{
        frequencyLabel.setBounds(x+14,y+40,136,18);gainLabel.setBounds(x+162,y+40,136,18);qLabel.setBounds(x+310,y+40,136,18);
        frequency.setBounds(x+14,y+58,136,116);gain.setBounds(x+162,y+58,136,116);q.setBounds(x+310,y+58,136,116);
    }
    shapeButton.setBounds(x+12,y+184,112,28);slopeButton.setBounds(x+130,y+184,88,28);placement.setBounds(x+224,y+184,92,28);modeSettings.setBounds(x+322,y+184,78,28);remove.setBounds(x+406,y+184,42,28);remove.setButtonText("Del");
    modeSettings.getProperties().remove("icon");modeSettings.setButtonText(juce::StringArray{"Static","Dynamic","Spectral"}[p.mode]);
    rangeLabel.setBounds(x+12,y+222,102,16);dynamicRange.getProperties().set("rangeRing",false);dynamicRange.setSliderStyle(juce::Slider::LinearHorizontal);dynamicRange.setTextBoxStyle(juce::Slider::TextBoxRight,false,52,20);dynamicRange.setBounds(x+12,y+242,102,24);
    attackLabel.setBounds(x+118,y+222,100,16);attack.setSliderStyle(juce::Slider::LinearHorizontal);attack.setTextBoxStyle(juce::Slider::TextBoxRight,false,54,20);attack.setBounds(x+118,y+242,100,24);
    releaseLabel.setBounds(x+222,y+222,100,16);release.setSliderStyle(juce::Slider::LinearHorizontal);release.setTextBoxStyle(juce::Slider::TextBoxRight,false,54,20);release.setBounds(x+222,y+242,100,24);
    triggerButton.setBounds(x+328,y+238,58,28);externalKey.setBounds(x+392,y+238,56,28);

    // Bottom console dock controls
    processingButton.setBounds(24,h-72,112,26);resolutionButton.setBounds(142,h-72,90,26);
    analyzer.setBounds(238,h-72,144,26);freeze.setBounds(388,h-72,66,26);
    for(auto* label:{&outputPanLabel,&wetMixLabel,&outputGainLabel})label->setJustificationType(juce::Justification::centred);
    outputPanLabel.setBounds(w-322,h-84,92,16);outputBalance.setBounds(w-322,h-68,92,64);
    wetMixLabel.setBounds(w-220,h-84,92,16);wetMix.setBounds(w-220,h-68,92,64);
    outputGainLabel.setBounds(w-118,h-84,92,16);outputKnob.setBounds(w-118,h-68,92,64);
    add.setBounds(214,h-30,104,24);add.setButtonText("+ Add band");
    outputBalanceMode.setBounds(326,h-30,116,24);outputInvert.setBounds(450,h-30,104,24);
    midSidePanel.setBounds(w-344,h-296,318,200);midSideTitle.setBounds(15,6,288,25);
    midLabel.setBounds(18,35,130,20);sideLabel.setBounds(170,35,130,20);midOutput.setBounds(18,56,130,105);sideOutput.setBounds(170,56,130,105);midInvert.setBounds(23,168,130,26);sideInvert.setBounds(175,168,130,26);
    midSidePanel.toFront(false);
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

void EQEditor::updateMeters(){
    const double now=juce::Time::getMillisecondCounterHiRes();
    const float seconds=float(lastMeterTime>0?juce::jlimit(.001,.1,(now-lastMeterTime)/1000):1.0/60);
    lastMeterTime=now;
    for(int ch=0;ch<2;++ch){
        const float db=juce::Decibels::gainToDecibels(eqProcessor.meterPeak[size_t(ch)].exchange(0),-100.0f);
        meterDisplayDb[size_t(ch)]=std::max(db,std::max(-100.0f,meterDisplayDb[size_t(ch)]-25*seconds));
        if(db>=meterPeakHoldDb[size_t(ch)]){
            meterPeakHoldDb[size_t(ch)]=db;
            meterPeakHoldTimer[size_t(ch)]=1.2f;
        }else if(meterPeakHoldTimer[size_t(ch)]>0.0f){
            meterPeakHoldTimer[size_t(ch)]=std::max(0.0f,meterPeakHoldTimer[size_t(ch)]-seconds);
        }else{
            meterPeakHoldDb[size_t(ch)]=std::max(-100.0f,meterPeakHoldDb[size_t(ch)]-18.0f*seconds);
        }
    }
}
