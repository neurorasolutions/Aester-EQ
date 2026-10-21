// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "Processor.h"
#include "Editor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <new>
#include <chrono>

static thread_local bool auditCallback=false;
static thread_local size_t callbackAllocations=0;
void* operator new(size_t size){if(auditCallback)++callbackAllocations;if(auto* memory=std::malloc(size?size:1))return memory;throw std::bad_alloc{};}
void* operator new[](size_t size){return ::operator new(size);}
void operator delete(void* memory) noexcept {std::free(memory);}
void operator delete[](void* memory) noexcept {std::free(memory);}
void* operator new(size_t size,std::align_val_t alignment){if(auditCallback)++callbackAllocations;
#if defined(_WIN32)
if(auto* memory=_aligned_malloc(size?size:1,size_t(alignment)))return memory;
#else
void* memory=nullptr;if(posix_memalign(&memory,size_t(alignment),size?size:1)==0)return memory;
#endif
throw std::bad_alloc{};}
void operator delete(void* memory,std::align_val_t) noexcept {
#if defined(_WIN32)
_aligned_free(memory);
#else
std::free(memory);
#endif
}

static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv) {
    juce::ScopedJuceInitialiser_GUI initialise;
    try {
        {
            FollowingPanelSlider fixed,moving;
            for(auto* slider:{&fixed,&moving}){slider->setBounds(100,100,80,80);slider->setRange(0,1);slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider->setValue(.5);}
            auto event=[](juce::Slider& slider,juce::Point<float> position,bool dragged){return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),position,juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier),1,0,0,0,0,&slider,&slider,juce::Time::getCurrentTime(),{40,40},juce::Time::getCurrentTime(),1,dragged);};
            fixed.mouseDown(event(fixed,{40,40},false));moving.mouseDown(event(moving,{40,40},false));
            for(int step=1;step<=8;++step){float dx=float(step*5);fixed.mouseDrag(event(fixed,{40+dx,40},true));moving.setTopLeftPosition(100+step*12,100);moving.mouseDrag(event(moving,{40+dx-step*12,40},true));require(std::abs(fixed.getValue()-moving.getValue())<1e-9,"Moving FREQ control preserves screen-space drag delta");}
            fixed.mouseUp(event(fixed,{80,40},true));moving.mouseUp(event(moving,{-16,40},true));
        }
        {
            juce::MidiBuffer midi;
            for(int phaseMode:{0,1,2})for(int bandMode:{0,2})for(float blendValue:{0.0f,.5f,1.0f}){
                auto mixTest=std::make_unique<EQProcessor>();mixTest->setValue("mix",blendValue);mixTest->setValue("output",6);
                mixTest->setValue("processingPhase",float(phaseMode));mixTest->setValue("linearResolution",0);
                mixTest->setValue(EQProcessor::bandID(0,"on"),1);mixTest->setValue(EQProcessor::bandID(0,"mode"),float(bandMode));mixTest->setValue(EQProcessor::bandID(0,"range"),0);mixTest->prepareToPlay(48000,128);
                const int latency=mixTest->getLatencySamples();double error=0;juce::AudioBuffer<float> impulse(2,128);
                for(int start=0;start<latency+2048;start+=128){impulse.clear();if(start==0)impulse.setSample(0,0,.25f);mixTest->processBlock(impulse,midi);for(int i=0;i<128;++i){const double expected=start+i==latency?.25*(1+blendValue*(juce::Decibels::decibelsToGain(6.0)-1)):0;error=std::max(error,std::abs(impulse.getSample(0,i)-expected));}}
                require(error<.0002,"Dry/wet Mix preserves impulse gain and latency across IIR, FIR and spectral modes");
            }
        }
        {
            auto ui=std::make_unique<EQProcessor>();std::unique_ptr<juce::AudioProcessorEditor> editor(ui->createEditor());
            juce::Slider* freq=nullptr;juce::Button* solo=nullptr;juce::Button* add=nullptr;
            for(auto* c:editor->getChildren()){
                if(auto* slider=dynamic_cast<juce::Slider*>(c))if(slider->getTooltip().startsWith("Frequency of the selected"))freq=slider;
                if(auto* button=dynamic_cast<juce::Button*>(c)){if(button->getButtonText()=="SOLO")solo=button;if(button->getButtonText()=="+ Add band")add=button;}
            }
            require(freq&&solo&&add,"Focus frequency, Solo and add controls exist");
            require(editor->getWidth()==1000&&editor->getHeight()==700,"Compact initial editor size");
            require(editor->isResizable(),"Editor supports host and corner resizing");
            require(!freq->isVisible(),"No floating controls without a band");
            add->onClick();require(freq->isVisible(),"Adding band reveals floating controls");
            const auto initial=freq->getPosition();freq->onDragStart();freq->setValue(3200,juce::sendNotificationSync);editor->resized();
            require(freq->getPosition()==initial,"Panel stays fixed while turning frequency knob");
            freq->onDragEnd();require(freq->getPosition()!=initial,"Panel follows frequency after knob release");
            solo->setToggleState(true,juce::dontSendNotification);solo->onClick();require(ui->auditionBand()==0,"Solo auditions the selected band");
            editor->setSize(960,680);require(editor->getLocalBounds().contains(freq->getBoundsInParent()),"Frequency knob fits compact editor");
            editor->setSize(1440,960);editor.reset();require(ui->auditionBand()==-1,"Closing editor releases Solo");
            editor.reset(ui->createEditor());require(editor->getWidth()==1440&&editor->getHeight()==960,"Reopened editor retains instance window size");
        }
        {
            // Measured auditory region, state/latency invariance, stereo routing and realtime safety.
            for(double sr:{44100.,48000.,96000.}){
                auto p=std::make_unique<EQProcessor>();p->setValue(EQProcessor::bandID(0,"on"),1);p->setValue(EQProcessor::bandID(0,"frequency"),1000);p->setValue(EQProcessor::bandID(0,"q"),4);p->prepareToPlay(sr,128);const int latency=p->getLatencySamples();
                juce::MidiBuffer midi;juce::AudioBuffer<float> b(2,128);
                auto rms=[&](double hz){double sum=0;int n=0;for(int block=0;block<160;++block){for(int i=0;i<128;++i){const float v=float(.1*std::sin(juce::MathConstants<double>::twoPi*hz*(block*128+i)/sr));b.setSample(0,i,v);b.setSample(1,i,v);}auditCallback=true;p->processBlock(b,midi);auditCallback=false;if(block>80)for(int i=0;i<128;++i){sum+=b.getSample(0,i)*b.getSample(0,i);++n;}}return std::sqrt(sum/n);};
                const size_t allocations=callbackAllocations;p->setAuditionBand(0);const double centre=rms(1000),outside=rms(100);
                require(centre>.065&&centre<.076,"Solo has unity centre gain");require(outside<centre*.04,"Solo attenuates frequencies outside selected Q region");require(p->getLatencySamples()==latency,"Solo does not alter processing latency");require(callbackAllocations==allocations,"Solo does not allocate in audio callback");
                p->setAuditionBand(-1);require(rms(100)>.065,"Exiting Solo restores full bandwidth");
                p->setAuditionBand(0);p->setValue(EQProcessor::bandID(0,"placement"),4);require(rms(1000)<1e-6,"Side Solo rejects correlated stereo input");
                juce::MemoryBlock state;p->getStateInformation(state);auto loaded=std::make_unique<EQProcessor>();loaded->setStateInformation(state.getData(),int(state.getSize()));require(loaded->auditionBand()==-1,"Solo is never recalled from presets");
                p->setValue(EQProcessor::bandID(0,"on"),0);rms(1000);require(p->auditionBand()==-1,"Disabling auditioned band exits Solo");
            }
        }
        {
            auto p=std::make_unique<EQProcessor>();p->setValue("mix",0);p->prepareToPlay(48000,128);auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p->createEditor());
            juce::Slider *pan=nullptr,*mix=nullptr,*out=nullptr;
            for(auto* child:editor->getChildren())if(auto* s=dynamic_cast<juce::Slider*>(child)){if(s->getTooltip().startsWith("Output balance after Mix"))pan=s;if(s->getTooltip().startsWith("Latency-aligned dry/wet"))mix=s;if(s->getTooltip().startsWith("Output volume."))out=s;}
            require(pan&&mix&&out,"Global output knobs are present");for(auto* s:{pan,mix,out})require(s->getSliderStyle()==juce::Slider::RotaryHorizontalVerticalDrag,"Global controls use rotary knobs");
            pan->setValue(1,juce::sendNotificationSync);require(p->parameters.getRawParameterValue("balance")->load()>.999f,"Balance knob reaches the processor parameter");
            juce::MidiBuffer midi;juce::AudioBuffer<float> b(2,128);
            auto render=[&]{for(int block=0;block<24;++block){for(int i=0;i<128;++i){b.setSample(0,i,.2f);b.setSample(1,i,.1f);}p->processBlock(b,midi);}};
            render();require(std::abs(b.getSample(0,127))<1e-6&&std::abs(b.getSample(1,127)-.1f)<1e-6,"Global balance works with a fully dry Mix");
            p->setValue("bypass",1);render();require(std::abs(b.getSample(0,127)-.2f)<1e-6&&std::abs(b.getSample(1,127)-.1f)<1e-6,"Bypass releases global balance");
        }
        {
            for(float mix:{0.f,1.f})for(int invM:{0,1})for(int invS:{0,1}){
                auto p=std::make_unique<EQProcessor>();p->setValue("balanceMode",1);p->setValue("mix",mix);p->setValue("midInvert",float(invM));p->setValue("sideInvert",float(invS));p->prepareToPlay(48000,128);
                juce::MidiBuffer midi;juce::AudioBuffer<float> b(2,128);for(int i=0;i<128;++i){b.setSample(0,i,.2f);b.setSample(1,i,.1f);}p->processBlock(b,midi);
                const double mid=.15*(invM?-1:1),side=.05*(invS?-1:1);require(std::abs(b.getSample(0,127)-mid-side)<1e-6&&std::abs(b.getSample(1,127)-mid+side)<1e-6,"Independent Mid/Side polarity decodes correctly at all Mix settings");
                p->setValue("bypass",1);for(int block=0;block<24;++block){for(int i=0;i<128;++i){b.setSample(0,i,.2f);b.setSample(1,i,.1f);}p->processBlock(b,midi);}require(std::abs(b.getSample(0,127)-.2f)<1e-6&&std::abs(b.getSample(1,127)-.1f)<1e-6,"Bypass releases Mid/Side polarity");
            }
            for(bool mid:{false,true}){auto p=std::make_unique<EQProcessor>();p->setValue("balanceMode",1);p->setValue(mid?"midGain":"sideGain",6.0206f);p->prepareToPlay(48000,128);juce::MidiBuffer midi;juce::AudioBuffer<float> b(2,128);for(int i=0;i<128;++i){b.setSample(0,i,.2f);b.setSample(1,i,.1f);}p->processBlock(b,midi);require(std::abs(b.getSample(0,127)-(mid?.35:.25))<.0001&&std::abs(b.getSample(1,127)-(mid?.25:.05))<.0001,"Independent Mid/Side output gain is correct");juce::MemoryBlock state;p->getStateInformation(state);auto loaded=std::make_unique<EQProcessor>();loaded->setStateInformation(state.getData(),int(state.getSize()));require(loaded->parameters.getRawParameterValue(mid?"midGain":"sideGain")->load()>6,"Mid/Side gain is recalled from state");}
        }
        {
            auto p=std::make_unique<EQProcessor>();auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p->createEditor());juce::ComboBox* mode=nullptr;juce::Slider* mid=nullptr;juce::ToggleButton* inv=nullptr;
            for(auto* c:editor->getChildren()){if(auto* box=dynamic_cast<juce::ComboBox*>(c))if(box->getTooltip()=="Choose Left/Right or Mid/Side output balance.")mode=box;for(auto* child:c->getChildren()){if(auto* s=dynamic_cast<juce::Slider*>(child))if(s->getTooltip().startsWith("Mid output gain:"))mid=s;if(auto* t=dynamic_cast<juce::ToggleButton*>(child))if(t->getTooltip().startsWith("Invert only the Mid"))inv=t;}}
            require(mode&&mid&&inv,"Mid/Side selector and controls are connected");mode->setSelectedId(2,juce::sendNotificationSync);require(p->parameters.getRawParameterValue("balanceMode")->load()>.5f,"UI selector actually enables Mid/Side processing");require(mid->isVisible()&&mid->getParentComponent()->isVisible(),"Selecting Mid/Side automatically reveals controls");mid->setValue(3,juce::sendNotificationSync);inv->setToggleState(true,juce::sendNotificationSync);require(std::abs(p->parameters.getRawParameterValue("midGain")->load()-3)<.01&&p->parameters.getRawParameterValue("midInvert")->load()>.5f,"Mid level and polarity controls reach their audio parameters");mode->setSelectedId(1,juce::sendNotificationSync);require(!mid->getParentComponent()->isVisible(),"Selecting L/R automatically hides Mid/Side controls");
        }
        EQProcessor processor;for(int i=0;i<24;++i)require(!processor.settings(i).enabled,"New instance starts without points");processor.setValue(EQProcessor::bandID(0,"on"),1);
        require(processor.getParameters().size()==396,"Stable parameter count");
        juce::MidiBuffer midi;
        for(double sr:{44100.,48000.,96000.}) {
            processor.prepareToPlay(sr,128);
            juce::AudioBuffer<float> audio(2,128);
            for(int i=0;i<128;++i){audio.setSample(0,i,float(std::sin(i*0.1)*0.1));audio.setSample(1,i,-audio.getSample(0,i));}
            auto original=juce::AudioBuffer<float>(audio);
            for(int block=0;block<9;++block){audio.makeCopyOf(original);processor.processBlock(audio,midi);}
            for(int i=0;i<128;++i)require(std::abs(audio.getSample(0,i)-original.getSample(0,i))<1e-6f,"Default transparent response");
        }
        {
            EQProcessor irregular;juce::AudioProcessor::BusesLayout layout;layout.inputBuses.add(juce::AudioChannelSet::mono());layout.inputBuses.add(juce::AudioChannelSet::disabled());layout.outputBuses.add(juce::AudioChannelSet::mono());require(irregular.setBusesLayout(layout),"Mono layout accepted");irregular.setValue(EQProcessor::bandID(0,"mode"),2);irregular.setValue(EQProcessor::bandID(0,"threshold"),0);irregular.prepareToPlay(44100,512);
            int offset=0;for(int block=0;block<32;++block){int count=juce::Array<int>{64,137,512}[block%3];juce::AudioBuffer<float> signal(1,count);for(int i=0;i<count;++i)signal.setSample(0,i,float(.1*std::sin((offset+i)*.073+.03)));irregular.processBlock(signal,midi);for(int i=0;i<count;++i){float expected=offset+i<1024?0:float(.1*std::sin((offset+i-1024)*.073+.03));require(std::abs(signal.getSample(0,i)-expected)<1e-6f,"Irregular blocks preserve mono reconstruction and exact sample delay");}offset+=count;}
        }
        {
            EQProcessor instant;juce::AudioBuffer<float> impulse(2,137);
            for(int mode:{0,1,2,0}){
                instant.setValue(EQProcessor::bandID(0,"mode"),float(mode));instant.setValue(EQProcessor::bandID(0,"threshold"),0);instant.prepareToPlay(48000,137);
                require(instant.getLatencySamples()==(mode==2?1024:0),"Static/dynamic zero latency and spectral latency notification");
                int peakIndex=-1;float peak=0;int absolute=0;
                for(int block=0;block<12;++block){impulse.clear();if(block==0)impulse.setSample(0,0,.25f);instant.processBlock(impulse,midi);for(int i=0;i<137;++i)if(std::abs(impulse.getSample(0,i))>peak){peak=std::abs(impulse.getSample(0,i));peakIndex=absolute+i;}absolute+=137;}
                require(peakIndex==instant.getLatencySamples()&&std::abs(peak-.25f)<1e-6f,"Impulse arrival agrees with host latency");
            }
            instant.setValue(EQProcessor::bandID(0,"mode"),2);require(instant.getLatencySamples()==1024,"Live spectral mode updates host latency");
            instant.setValue(EQProcessor::bandID(0,"mode"),1);require(instant.getLatencySamples()==0,"Live dynamic mode restores zero latency");
        }
        processor.setValue(EQProcessor::bandID(0,"frequency"),2300);
        processor.setValue(EQProcessor::bandID(0,"gain"),8);
        processor.setValue(EQProcessor::bandID(0,"shape"),3);
        processor.setValue(EQProcessor::bandID(23,"on"),1);
        processor.setValue(EQProcessor::bandID(23,"gain"),-3);
        processor.setValue("output",-6);
        processor.setValue(EQProcessor::bandID(0,"mode"),2);processor.setValue(EQProcessor::bandID(0,"threshold"),-42);processor.setValue(EQProcessor::bandID(0,"range"),-9);processor.setValue(EQProcessor::bandID(0,"placement"),4);
        juce::MemoryBlock state;processor.getStateInformation(state);
        EQProcessor restored;restored.setStateInformation(state.getData(),int(state.getSize()));
        require(std::abs(restored.settings(0).frequency-2300)<1,"Frequency state round trip");
        require(std::abs(restored.settings(0).gain-8)<0.01,"Gain state round trip");
        require(restored.settings(23).enabled,"Band 24 state round trip");
        require(restored.settings(0).shape==distillazione::Shape::HighShelf,"Shape state round trip");
        require(restored.settings(0).mode==2&&restored.settings(0).placement==4,"Advanced mode and placement state round trip");require(std::abs(restored.settings(0).threshold+42)<.01&&std::abs(restored.settings(0).range+9)<.01,"Dynamics settings state round trip");
        {
            auto legacy=restored.parameters.copyState();for(int child=legacy.getNumChildren()-1;child>=0;--child){auto id=legacy.getChild(child).getProperty("id").toString();if(id.endsWith("_mode")||id.endsWith("_placement")||id.endsWith("_threshold")||id.endsWith("_range")||id.endsWith("_attack")||id.endsWith("_release")||id.endsWith("_external")||id.endsWith("_free")||id.endsWith("_triggerLow")||id.endsWith("_triggerHigh"))legacy.removeChild(child,nullptr);}juce::MemoryBlock legacyBytes;juce::AudioProcessor::copyXmlToBinary(*legacy.createXml(),legacyBytes);EQProcessor legacyRestored;legacyRestored.setValue(EQProcessor::bandID(0,"mode"),2);legacyRestored.setStateInformation(legacyBytes.getData(),int(legacyBytes.getSize()));require(legacyRestored.settings(0).mode==0&&legacyRestored.settings(0).placement==0&&!legacyRestored.settings(0).externalSidechain,"Old presets restore new parameters to neutral defaults");require(std::abs(legacyRestored.settings(0).gain-8)<.01,"Old preset preserves original EQ settings");
        }
        processor.setValue(EQProcessor::bandID(0,"mode"),0);processor.setValue(EQProcessor::bandID(0,"placement"),0);
        processor.setValue("bypass",1);processor.prepareToPlay(48000,128);
        juce::AudioBuffer<float> audio(2,128);
        for(int i=0;i<128;++i){audio.setSample(0,i,float(std::sin(i)*0.1));audio.setSample(1,i,0.25f);}
        auto original=juce::AudioBuffer<float>(audio);for(int block=0;block<9;++block){audio.makeCopyOf(original);processor.processBlock(audio,midi);}
        for(int ch=0;ch<2;++ch)for(int i=0;i<128;++i)require(audio.getSample(ch,i)==original.getSample(ch,i),"Bypass passes exact dry samples");
        require(processor.channelOutputPeak[0].load()<.11f&&std::abs(processor.channelOutputPeak[1].load()-.25f)<1e-6f,"Stereo output meters measure their own channels");
        processor.setValue("bypass",0);processor.prepareToPlay(48000,128);
        for(int block=0;block<300;++block) {
            processor.setValue(EQProcessor::bandID(0,"frequency"),float(30+block*60));
            processor.setValue(EQProcessor::bandID(0,"gain"),float(12*std::sin(block*0.03)));
            if(block%30==0)processor.setValue(EQProcessor::bandID(0,"shape"),float((block/30)%10));
            for(int i=0;i<128;++i)for(int ch=0;ch<2;++ch)audio.setSample(ch,i,float(0.02*std::sin((block*128+i)*0.08)));
            processor.processBlock(audio,midi);
            for(int ch=0;ch<2;++ch)for(int i=0;i<128;++i)require(std::isfinite(audio.getSample(ch,i)),"Finite automation output");
        }
        {
            EQProcessor dynamics;require(dynamics.getLatencySamples()==0,"Static host latency is zero");
            auto configure=[&](int mode,int route){dynamics.setValue(EQProcessor::bandID(0,"on"),1);dynamics.setValue(EQProcessor::bandID(0,"mode"),float(mode));dynamics.setValue(EQProcessor::bandID(0,"placement"),float(route));dynamics.setValue(EQProcessor::bandID(0,"frequency"),1875);dynamics.setValue(EQProcessor::bandID(0,"q"),8);dynamics.setValue(EQProcessor::bandID(0,"threshold"),-60);dynamics.setValue(EQProcessor::bandID(0,"range"),-12);dynamics.setValue(EQProcessor::bandID(0,"attack"),1);dynamics.setValue(EQProcessor::bandID(0,"release"),50);dynamics.prepareToPlay(48000,128);};
            auto measure=[&](bool antiPhase){std::array<double,4> dots{};juce::AudioBuffer<float> block(2,128);int measured=0;
                for(int offset=0;offset<48000;offset+=128){for(int i=0;i<128;++i){double t=double(offset+i)/48000;float value=float(.1*std::sin(juce::MathConstants<double>::twoPi*1875*t)+.06*std::sin(juce::MathConstants<double>::twoPi*6000*t));block.setSample(0,i,value);block.setSample(1,i,antiPhase?-value:value);}dynamics.processBlock(block,midi);if(offset>=24000){for(int i=0;i<128;++i){double t=double(offset+i-dynamics.getLatencySamples())/48000;for(int ch=0;ch<2;++ch){dots[size_t(ch*2)]+=block.getSample(ch,i)*std::sin(juce::MathConstants<double>::twoPi*1875*t);dots[size_t(ch*2+1)]+=block.getSample(ch,i)*std::sin(juce::MathConstants<double>::twoPi*6000*t);}++measured;}}}
                for(auto& value:dots)value=2*std::abs(value)/measured;return dots;};
            configure(0,0);auto neutral=measure(false);require(std::abs(neutral[0]-.1)<.0003&&std::abs(neutral[1]-.06)<.0003,"STFT reconstruction preserves both tones");
            configure(1,0);auto dynamic=measure(false);require(dynamic[0]<.035&&dynamic[1]>.055,"Dynamic EQ attenuates detected band, preserves distant tone");require(dynamics.dynamicAmount[0].load()<-11,"Dynamic gain meter reacts");
            configure(2,0);auto spectral=measure(false);require(spectral[0]<.037&&spectral[1]>.055,"Spectral EQ attenuates selected bins, preserves distant tone");
            configure(2,0);measure(false);dynamics.setValue(EQProcessor::bandID(0,"mode"),0);auto released=measure(false);require(std::abs(released[0]-.1)<.0003,"Spectral processing releases after returning to Static");
            configure(1,0);dynamics.setValue(EQProcessor::bandID(0,"threshold"),0);auto belowThreshold=measure(false);require(std::abs(belowThreshold[0]-.1)<.0003,"Below-threshold dynamic EQ stays neutral");
            configure(2,0);dynamics.setValue(EQProcessor::bandID(0,"threshold"),0);belowThreshold=measure(false);require(std::abs(belowThreshold[0]-.1)<.0003,"Below-threshold spectral EQ stays neutral");
            configure(1,0);dynamics.setValue(EQProcessor::bandID(0,"range"),6);auto boosted=measure(false);require(boosted[0]>.18,"Positive dynamic range produces expansion");
            configure(2,0);dynamics.setValue(EQProcessor::bandID(0,"range"),6);boosted=measure(false);require(boosted[0]>.17,"Positive spectral range boosts selected bins");
            configure(1,1);auto leftOnly=measure(false);require(leftOnly[0]<.035&&std::abs(leftOnly[2]-.1)<.0003,"Left placement preserves right channel");
            configure(2,2);auto rightOnly=measure(false);require(std::abs(rightOnly[0]-.1)<.0003&&rightOnly[2]<.037,"Spectral right placement preserves left channel");
            configure(1,3);auto midOnly=measure(true);require(std::abs(midOnly[0]-.1)<.0003,"Mid detector ignores pure side signal");
            configure(2,4);auto sideOnly=measure(true);require(sideOnly[0]<.037&&sideOnly[2]<.037,"Spectral side EQ processes anti-phase signal");
            dynamics.setValue(EQProcessor::bandID(0,"mode"),1);dynamics.prepareToPlay(48000,128);measure(false);juce::AudioBuffer<float> silence(2,128);silence.clear();for(int i=0;i<1000;++i)dynamics.processBlock(silence,midi);require(std::abs(dynamics.dynamicAmount[0].load())<.05,"Dynamic detector releases on silence");
            require(dynamics.splitBand(0),"Split uses free slot");require(std::abs(dynamics.settings(0).range+6)<.01&&dynamics.settings(1).mode==1,"Split copies advanced settings and divides range");
            for(int i=0;i<24;++i){dynamics.setValue(EQProcessor::bandID(i,"on"),1);dynamics.setValue(EQProcessor::bandID(i,"mode"),float(i%3));dynamics.setValue(EQProcessor::bandID(i,"placement"),float(i%5));dynamics.setValue(EQProcessor::bandID(i,"frequency"),float(80*std::pow(1.22,i)));dynamics.setValue(EQProcessor::bandID(i,"range"),-3);}require(!dynamics.splitBand(0),"Split refuses when all slots are occupied");dynamics.prepareToPlay(48000,128);
            juce::AudioBuffer<float> stress(2,128);auto began=std::chrono::steady_clock::now();callbackAllocations=0;for(int block=0;block<400;++block){if(block%17==0){int band=(block/17)%24;dynamics.setValue(EQProcessor::bandID(band,"mode"),float((block/17)%3));dynamics.setValue(EQProcessor::bandID(band,"placement"),float((block/17)%5));dynamics.setValue(EQProcessor::bandID(band,"threshold"),float(-50+(block/17)%20));}for(int i=0;i<128;++i){float value=float(.02*std::sin((block*128+i)*.07));stress.setSample(0,i,value);stress.setSample(1,i,-value);}auditCallback=true;dynamics.processBlock(stress,midi);auditCallback=false;for(int ch=0;ch<2;++ch)for(int i=0;i<128;++i)require(std::isfinite(stress.getSample(ch,i)),"24 mixed-mode bands produce finite output");}require(callbackAllocations==0,"No C++ heap allocations in callback");auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-began).count();std::cout<<"24-band mixed-mode render: "<<elapsed<<" seconds for "<<400.*128/48000<<" seconds of stereo audio; C++ callback allocations: "<<callbackAllocations<<'\n';

        }
        {
            for(int mode:{1,2}) {
                auto measureKey=[&](bool external,float keyAmplitude,float mainAmplitude,int keyChannels=2,bool free=false,float keyHz=1875){
                    EQProcessor keyed;auto layout=keyed.getBusesLayout();layout.inputBuses.set(1,keyChannels==1?juce::AudioChannelSet::mono():juce::AudioChannelSet::stereo());require(keyed.setBusesLayout(layout),"Stereo sidechain layout accepted");
                    auto p=keyed.settings(0);p.enabled=true;p.mode=mode;p.frequency=1875;p.threshold=-48;p.range=-12;p.attack=.1;p.release=10;p.externalSidechain=external;p.freeTrigger=free;p.triggerLow=80;p.triggerHigh=400;keyed.copyBand(0,p);keyed.prepareToPlay(48000,128);
                    juce::AudioBuffer<float> block(2+keyChannels,128);double energy=0;int count=0;
                    for(int n=0;n<800;++n){for(int i=0;i<128;++i){float tone=float(std::sin(juce::MathConstants<double>::twoPi*1875*(n*128+i)/48000));for(int ch=0;ch<2;++ch){block.setSample(ch,i,mainAmplitude*tone);if(ch<keyChannels)block.setSample(ch+2,i,keyAmplitude*float(std::sin(juce::MathConstants<double>::twoPi*keyHz*(n*128+i)/48000)));}}auditCallback=true;keyed.processBlock(block,midi);auditCallback=false;if(n>600)for(int i=0;i<128;++i){energy+=std::pow(block.getSample(0,i),2);++count;}}
                    juce::MemoryBlock state;keyed.getStateInformation(state);EQProcessor saved;saved.setStateInformation(state.getData(),int(state.getSize()));require(saved.settings(0).externalSidechain==external,"External sidechain survives state roundtrip");
                    require(saved.settings(0).freeTrigger==free&&std::abs(saved.settings(0).triggerLow-80)<.01&&std::abs(saved.settings(0).triggerHigh-400)<.01,"Free trigger survives state roundtrip");return std::sqrt(energy/count);
                };
                const double quiet=measureKey(true,0,.1f),triggered=measureKey(true,.25f,.1f),internal=measureKey(false,0,.1f);
                require(quiet>.070&&quiet<.071,"Silent external key does not use main input");require(triggered<quiet*.4,"External key controls attenuation");require(internal<quiet*.4,"Internal detector remains available");require(measureKey(true,.25f,0)<1e-10,"Sidechain never leaks into main output");require(measureKey(true,.25f,.1f,1)<quiet*.4,"Mono sidechain triggers stereo output");if(mode==1){const double passed=measureKey(true,.25f,.1f,2,true,187.5f),rejected=measureKey(true,.25f,.1f,2,true,12000);require(passed<quiet*.4,"Free trigger responds to key outside EQ band");require(rejected>quiet*.9,"Free trigger rejects key outside detector range");}
                require(callbackAllocations==0,"External sidechain callback has no C++ allocations");
            }
        }
        {
            require(std::abs(SpectrumDisplay::tiltedDb(-100,20000,4.5f)+100)<.001,"Visual tilt preserves silent floor");require(SpectrumDisplay::tiltedDb(-99.99f,20000,4.5f)<-99,"Near-floor energy does not become a false high-frequency ramp");
            SpectrumTap tap;SpectrumDisplay display;tap.enabled.store(true);tap.prepare(48000);
            constexpr int bin=43;
            for(int frame=0;frame<24;++frame){for(int i=0;i<SpectrumTap::size;++i){float sample=float(.25*std::sin(juce::MathConstants<double>::twoPi*bin*i/SpectrumTap::size));tap.push(sample,-sample,sample*.5f,-sample*.5f);}require(display.update(tap),"Spectrum frame arrives");}
            auto peak=std::max_element(display.pre.begin()+1,display.pre.end());
            require(std::distance(display.pre.begin(),peak)==bin,"FFT locates known tone");
            require(std::abs(*peak-juce::Decibels::gainToDecibels(.25f))<.2f,"Stereo anti-phase energy is preserved");
            require(std::abs(display.post[size_t(bin)]-display.pre[size_t(bin)]+6.0206f)<.2f,"Pre/post FFT measures gain difference");
            const float held=display.pre[size_t(bin)];require(!display.update(tap),"No fresh frame");require(display.pre[size_t(bin)]==held,"Hold analysis between overlapping frames");
            for(int i=0;i<512;++i)tap.push(0,0,0,0);require(display.update(tap),"Quarter-window analyzer hop");
            require(std::isfinite(display.displayDb(0,43.5)),"Log-frequency display interpolation finite");
            for(int i=0;i<8*SpectrumTap::size;++i)tap.push(0,0,0,0);
            require(display.update(tap),"Full analyzer queue drops without blocking");
            while(display.update(tap)){};
            for(int frame=0;frame<40;++frame){for(int i=0;i<SpectrumTap::size;++i)tap.push(0,0,0,0);display.update(tap);}
            require(display.pre[size_t(bin)]<-99,"Spectrum decays after silence");
        }
        {
            for(int mode:{0,1})for(float pan:{-1.f,0.f,1.f}){
                EQProcessor p;p.setValue("balanceMode",float(mode));p.setValue("balance",pan);p.prepareToPlay(48000,128);juce::AudioBuffer<float> b(2,128);for(int i=0;i<128;++i){b.setSample(0,i,.2f);b.setSample(1,i,.1f);}p.processBlock(b,midi);const float left=b.getSample(0,127),right=b.getSample(1,127);if(mode==0){require(std::abs(left-(pan==1?0:.2f))<1e-6&&std::abs(right-(pan==-1?0:.1f))<1e-6,"Output L/R balance endpoints");}else{if(pan==-1)require(std::abs(left-.15f)<1e-6&&std::abs(right-.15f)<1e-6,"M/S balance can isolate Mid");if(pan==1)require(std::abs(left-.05f)<1e-6&&std::abs(right+.05f)<1e-6,"M/S balance can isolate Side");}
                p.setValue("invert",1);p.prepareToPlay(48000,128);for(int i=0;i<128;++i){b.setSample(0,i,.2f);b.setSample(1,i,.1f);}p.processBlock(b,midi);require(std::abs(b.getSample(0,127)+left)<1e-6,"Output polarity inversion");
            }
        }
        {
            for(int mode:{1,2})for(int resolution:{0,1,2,3,4}){if(mode==1&&resolution!=0)continue;
                EQProcessor p;p.setValue("processingPhase",float(mode));p.setValue("linearResolution",float(resolution));p.prepareToPlay(48000,128);juce::AudioBuffer<float> b(2,128);callbackAllocations=0;
                for(int n=0;n<2000&&!p.phaseReady();++n){b.clear();auditCallback=true;p.processBlock(b,midi);auditCallback=false;juce::Thread::sleep(1);}require(p.phaseReady(),"Phase FIR kernel becomes ready");require(callbackAllocations==0,"FIR handoff and warm-up have no C++ callback allocation");
                const int latency=PhaseEngine::delay(mode,resolution,48000);require(p.getLatencySamples()==latency,"Phase latency matches selected resolution");
                int peak=-1;float amplitude=0;callbackAllocations=0;
                for(int n=0;n<latency+256;n+=128){b.clear();if(n==0){b.setSample(0,0,.25f);b.setSample(1,0,.125f);}auditCallback=true;p.processBlock(b,midi);auditCallback=false;for(int i=0;i<128;++i)if(std::abs(b.getSample(0,i))>amplitude){amplitude=std::abs(b.getSample(0,i));peak=n+i;}}
                std::cout<<"Measured phase "<<mode<<" res="<<resolution<<" peak="<<peak<<" expected="<<latency<<" amp="<<amplitude<<std::endl;require(peak==latency&&std::abs(amplitude-.25f)<1e-5,"Neutral FIR impulse arrival and amplitude");require(callbackAllocations==0,"Steady phase rendering has no C++ allocation");std::cout<<"Phase "<<mode<<" resolution "<<resolution<<" delay "<<peak<<" allocation "<<callbackAllocations<<'\n';
            }
            for(int mode:{1,2}){EQProcessor p;auto setting=p.settings(0);setting.enabled=true;setting.frequency=1000;setting.gain=6;p.copyBand(0,setting);p.setValue("processingPhase",float(mode));p.setValue("linearResolution",0);p.prepareToPlay(48000,128);juce::AudioBuffer<float> b(2,128);for(int n=0;n<2000&&!p.phaseReady();++n){b.clear();p.processBlock(b,midi);juce::Thread::sleep(1);}require(p.phaseReady(),"Non-neutral phase kernel ready");std::complex<double> transfer{};int count=0;for(int block=0;block<800;++block){for(int i=0;i<128;++i){auto t=block*128+i;b.setSample(0,i,float(.1*std::sin(juce::MathConstants<double>::twoPi*1000*t/48000)));b.setSample(1,i,0);}p.processBlock(b,midi);if(block>=500)for(int i=0;i<128;++i){transfer+=double(b.getSample(0,i))*std::polar(1.0,-juce::MathConstants<double>::twoPi*1000*(block*128+i-p.getLatencySamples())/48000);++count;}}transfer*=2.0/count;require(std::abs(20*std::log10(std::abs(transfer)/.1)-6)<.15,"Phase FIR realizes requested Bell gain");require(std::abs(std::remainder(std::arg(transfer)+juce::MathConstants<double>::halfPi,juce::MathConstants<double>::twoPi))<.02,"Phase FIR centre phase after delay compensation");}
        }
        {
            for(int mode:{1,2})for(double sampleRate:{44100.,96000.}){
                EQProcessor p;p.setValue("processingPhase",float(mode));p.setValue("linearResolution",0);p.setValue("bypass",1);auto band=p.settings(0);band.enabled=true;band.gain=12;band.mode=2;band.threshold=0;p.copyBand(0,band);p.prepareToPlay(sampleRate,137);juce::AudioBuffer<float> b(2,137);
                for(int n=0;n<2000&&!p.phaseReady();++n){b.clear();p.processBlock(b,midi);juce::Thread::sleep(1);}require(p.phaseReady(),"Bypass phase kernel ready");const int delay=PhaseEngine::delay(mode,0,sampleRate)+1024;require(p.getLatencySamples()==delay,"Phase plus spectral latency is additive");
                for(int n=0;n<delay+512;n+=137){b.clear();if(n==0)b.setSample(0,0,.2f);p.processBlock(b,midi);for(int i=0;i<137;++i)require(std::abs(b.getSample(0,i)-(n+i==delay?.2f:0))<1e-6,"Phase/spectral bypass returns delayed dry impulse");}
                juce::MemoryBlock saved;p.getStateInformation(saved);EQProcessor restored;restored.setStateInformation(saved.getData(),int(saved.getSize()));require(int(restored.parameters.getRawParameterValue("processingPhase")->load())==mode,"Processing phase survives preset roundtrip");
            }
        }
        {
            for(int mode:{1,2}){EQProcessor p;auto setting=p.settings(0);setting.enabled=true;setting.frequency=1000;setting.gain=6;p.copyBand(0,setting);p.setValue("processingPhase",float(mode));p.setValue("linearResolution",0);p.prepareToPlay(48000,128);juce::AudioBuffer<float> b(2,128);std::complex<double> response{};for(int n=0;n<35000;n+=128){b.clear();if(n==0)b.setSample(0,0,.1f);p.processBlock(b,midi);for(int i=0;i<128;++i)response+=double(b.getSample(0,i))*std::polar(1.0,-juce::MathConstants<double>::twoPi*1000*(n+i-p.getLatencySamples())/48000);}require(std::abs(20*std::log10(std::abs(response)/.1)-6)<.15,"Initial FIR is active from the first impulse, without realtime pacing");}
        }
        {
            for(int shape:{0,2,4}){EQProcessor p;auto setting=p.settings(0);setting.enabled=true;setting.shape=distillazione::Shape(shape);setting.q=1;setting.gain=12;p.copyBand(0,setting);p.prepareToPlay(48000,128);juce::AudioBuffer<float> b(2,128);double peak=0;for(int block=0;block<800;++block){p.setValue(EQProcessor::bandID(0,"frequency"),float(20*std::pow(1000.,double(block%40)/39)));p.setValue(EQProcessor::bandID(0,"gain"),float(12*std::sin(block*.11)));for(int i=0;i<128;++i){const int t=block*128+i;float x=float(.02*(std::sin(t*.013)+std::sin(t*.13)+std::sin(t*1.3)));b.setSample(0,i,x);b.setSample(1,i,x);}p.processBlock(b,midi);for(int i=0;i<128;++i)peak=std::max(peak,std::abs(double(b.getSample(0,i))));}std::cout<<"Rapid drag shape "<<shape<<" peak="<<peak<<std::endl;require(peak<.5,"Rapid node motion avoids large artificial audio peaks");}
        }
        {
            EQProcessor p;p.prepareToPlay(48000,128);juce::AudioBuffer<float> b(2,128);b.clear();b.setSample(0,0,.8f);p.processBlock(b,midi);b.clear();p.processBlock(b,midi);require(p.channelOutputPeak[0].load()==0,"Raw latest block is silent");require(std::abs(p.meterPeak[0].exchange(0)-.8f)<1e-6,"UI meter retains a peak between refreshes");require(p.meterPeak[0].load()==0,"UI peak latch is consumed without changing audio");
        }
        {
            EQProcessor ui;std::unique_ptr<juce::AudioProcessorEditor> editor(ui.createEditor());
            auto button=[&](const char* tooltip)->juce::Button* {for(auto* child:editor->getChildren())if(auto* b=dynamic_cast<juce::Button*>(child))if(b->getTooltip()==tooltip)return b;return nullptr;};
            auto* freeze=button("Freeze the spectrum display while audio and meters continue.");require(freeze!=nullptr,"Analyzer Freeze control exists");freeze->onClick();require(freeze->getButtonText()=="Resume","Freeze exposes Resume action");freeze->onClick();require(freeze->getButtonText()=="Freeze","Resume returns to live spectrum");
            auto* undo=button("Undo");auto* redo=button("Redo");auto* ab=button("Switch between A and B");auto* copy=button("Copy current setting to the other A/B slot");require(undo&&redo&&ab&&copy,"Operational comparison toolbar exists");
            require(button("Choose a window size, or drag the lower-right corner to resize freely."),"Band menu and resize controls are attached to the editor");
            ui.setValue(EQProcessor::bandID(0,"gain"),6);undo->onClick();require(std::abs(ui.settings(0).gain)<.01,"Editor Undo restores previous setting");redo->onClick();require(std::abs(ui.settings(0).gain-6)<.01,"Editor Redo restores edited setting");
            ab->onClick();require(std::abs(ui.settings(0).gain)<.01,"A/B recalls independent B setting");ui.setValue(EQProcessor::bandID(0,"gain"),-3);copy->onClick();ab->onClick();require(std::abs(ui.settings(0).gain+3)<.01,"Comparison Copy copies current slot to other slot");
        }
        if(argc>1) {
            if(argc>2&&juce::String(argv[2]).startsWith("--snapshot-")){
                for(int band=0;band<24;++band){distillazione::BandSettings p;p.enabled=band==0&&juce::String(argv[2])!="--snapshot-empty";p.frequency=400;p.q=1;if(juce::String(argv[2])=="--snapshot-lowcut")p.shape=distillazione::Shape::LowCut;if(juce::String(argv[2])=="--snapshot-highcut")p.shape=distillazione::Shape::HighCut;p.mode=band==0?(juce::String(argv[2])=="--snapshot-dynamic"?1:juce::String(argv[2])=="--snapshot-spectral"?2:0):0;restored.copyBand(band,p);}restored.setValue("output",0);
            }
            if(argc>2&&juce::String(argv[2])=="--snapshot-linear"){restored.setValue("processingPhase",2);restored.setValue("linearResolution",0);}
            if(argc>2&&juce::String(argv[2])=="--snapshot-multiband"){
                const float frequencies[]{45,180,800,3400,7000,14000},gains[]{0,2.5f,-1.5f,-4,2,0};
                for(int band=0;band<6;++band){distillazione::BandSettings setting;setting.enabled=true;setting.frequency=frequencies[band];setting.gain=gains[band];setting.q=.8;setting.shape=band==0?distillazione::Shape::LowCut:band==5?distillazione::Shape::HighCut:distillazione::Shape::Bell;setting.mode=band==3?1:0;restored.copyBand(band,setting);}restored.setValue("mix",.85f);
            }
            if(argc>2&&juce::String(argv[2])=="--snapshot-solo"){restored.setValue(EQProcessor::bandID(0,"gain"),4);distillazione::BandSettings other;other.enabled=true;other.frequency=6000;other.gain=-6;restored.copyBand(1,other);}
            std::unique_ptr<juce::AudioProcessorEditor> editor(restored.createEditor());
            restored.prepareToPlay(48000,128);
            juce::AudioBuffer<float> signal(2,128);
            for(int block=0;block<128;++block){for(int i=0;i<128;++i){double t=double(block*128+i)/48000;float sample=0;for(int harmonic=1;harmonic<=((argc>2&&juce::String(argv[2])=="--snapshot-empty")?0:28);++harmonic)sample+=float(.009/std::sqrt(double(harmonic))*std::sin(juce::MathConstants<double>::twoPi*(37*harmonic*harmonic)*t));signal.setSample(0,i,sample);signal.setSample(1,i,-sample);}restored.processBlock(signal,midi);if(block%16==15)static_cast<EQEditor*>(editor.get())->updateAnalysis();}
            if(argc>2&&juce::String(argv[2])=="--snapshot-ms"){for(auto* c:editor->getChildren())if(auto* box=dynamic_cast<juce::ComboBox*>(c))if(box->getTooltip()=="Choose Left/Right or Mid/Side output balance.")box->setSelectedId(2,juce::sendNotificationSync);}
            if(argc>2&&juce::String(argv[2])=="--snapshot-solo"){restored.setAuditionBand(0);editor->resized();for(auto* c:editor->getChildren())if(auto* button=dynamic_cast<juce::Button*>(c))if(button->getButtonText()=="SOLO")button->setToggleState(true,juce::dontSendNotification);}
            if(argc>2&&juce::String(argv[2])=="--snapshot-output")for(auto* child:editor->getChildren())for(auto* nested:child->getChildren())if(auto* box=dynamic_cast<juce::ComboBox*>(nested))if(box->getText()=="L/R")child->setVisible(true);
            if(argc>2&&juce::String(argv[2])=="--snapshot-small")editor->setSize(1120,780);
            if(argc>2&&juce::String(argv[2])=="--snapshot-frozen"){for(auto* child:editor->getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()=="Freeze")button->onClick();}
            auto image=editor->createComponentSnapshot(editor->getLocalBounds(),true,1);
            juce::FileOutputStream output{juce::File{juce::String{argv[1]}}};
            require(output.openedOk(),"Snapshot output");output.setPosition(0);require(output.truncate().wasOk(),"Snapshot truncate");juce::PNGImageFormat png;require(png.writeImageToStream(image,output),"Editor render");
            if(argc>2&&juce::String(argv[2])=="--ui-test"){
                juce::DocumentWindow window("Aester EQ UI verification",juce::Colour(0xff17151b),juce::DocumentWindow::allButtons);
                window.setUsingNativeTitleBar(true);window.setContentOwned(editor.release(),true);window.setBounds(80,180,1000,650);window.setVisible(true);
                juce::Timer::callAfterDelay(60000,[]{juce::MessageManager::getInstance()->stopDispatchLoop();});
                juce::MessageManager::getInstance()->runDispatchLoop();
            }

        }
        std::cout<<"Processor checks passed: STFT reconstruction/latency, mono, legacy/new state, bypass, automation, dynamic/spectral selectivity, stereo routing, expansion, release, split, allocation audit, analyzer FFT, editor render\n";
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
