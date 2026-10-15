// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "Processor.h"
#include "Editor.h"

juce::String EQProcessor::bandID(int band,const char* field) { return "band"+juce::String(band+1)+"_"+field; }
juce::AudioProcessorValueTreeState::ParameterLayout EQProcessor::layout() {
    juce::AudioProcessorValueTreeState::ParameterLayout result;
    const juce::StringArray shapes{"Bell","Low Shelf","Low Cut","High Shelf","High Cut","Notch","Band Pass","Tilt Shelf","Flat Tilt","All Pass"};
    for (int i=0;i<24;++i) {
        const auto prefix="Band "+juce::String(i+1)+" ";
        auto id=[i](const char* name){return juce::ParameterID{bandID(i,name),1};};
        result.add(std::make_unique<juce::AudioParameterBool>(id("on"),prefix+"Enabled",false));
        result.add(std::make_unique<juce::AudioParameterChoice>(id("shape"),prefix+"Shape",shapes,0));
        juce::NormalisableRange<float> freq(10,30000); freq.setSkewForCentre(1000);
        juce::NormalisableRange<float> q(0.1f,20); q.setSkewForCentre(0.70710678f);
        result.add(std::make_unique<juce::AudioParameterFloat>(id("frequency"),prefix+"Frequency",freq,1000));
        result.add(std::make_unique<juce::AudioParameterFloat>(id("gain"),prefix+"Gain",juce::NormalisableRange<float>(-30,30,0.01f),0));
        result.add(std::make_unique<juce::AudioParameterFloat>(id("q"),prefix+"Q",q,1.0f));
        result.add(std::make_unique<juce::AudioParameterChoice>(id("slope"),prefix+"Slope",juce::StringArray{"6","12","24","36","48","72","96"},1));
    }
    result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"output",1},"Output",juce::NormalisableRange<float>(-24,12,0.01f),0));
    result.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"Bypass",false));
    for(int i=0;i<24;++i){auto id=[i](const char* field){return juce::ParameterID{bandID(i,field),1};};auto prefix="Band "+juce::String(i+1)+" ";
        result.add(std::make_unique<juce::AudioParameterChoice>(id("mode"),prefix+"Mode",juce::StringArray{"Static","Dynamic","Spectral"},0));
        result.add(std::make_unique<juce::AudioParameterChoice>(id("placement"),prefix+"Placement",juce::StringArray{"Stereo","Left","Right","Mid","Side"},0));
        result.add(std::make_unique<juce::AudioParameterFloat>(id("threshold"),prefix+"Threshold",juce::NormalisableRange<float>(-80,0,.1f),-24));
        result.add(std::make_unique<juce::AudioParameterFloat>(id("range"),prefix+"Dynamic range",juce::NormalisableRange<float>(-24,24,.1f),-6));
        result.add(std::make_unique<juce::AudioParameterFloat>(id("attack"),prefix+"Attack",juce::NormalisableRange<float>(.1f,200,.1f),10));
        result.add(std::make_unique<juce::AudioParameterFloat>(id("release"),prefix+"Release",juce::NormalisableRange<float>(10,2000,1),120));
    }
    for(int i=0;i<24;++i)result.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{bandID(i,"external"),2},"Band "+juce::String(i+1)+" External sidechain",false));
    for(int i=0;i<24;++i){auto id=[i](const char* field){return juce::ParameterID{bandID(i,field),3};};auto prefix="Band "+juce::String(i+1)+" Trigger ";result.add(std::make_unique<juce::AudioParameterBool>(id("free"),prefix+"Free",false));juce::NormalisableRange<float> hz(10,30000);hz.setSkewForCentre(1000);result.add(std::make_unique<juce::AudioParameterFloat>(id("triggerLow"),prefix+"Low cut",hz,20));result.add(std::make_unique<juce::AudioParameterFloat>(id("triggerHigh"),prefix+"High cut",hz,20000));}
    result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"balance",4},"Output balance",juce::NormalisableRange<float>(-1,1,.001f),0));
    result.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"balanceMode",4},"Output balance mode",juce::StringArray{"L/R","M/S"},0));
    result.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"invert",4},"Output polarity invert",false));
    result.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"processingPhase",5},"Processing Mode",juce::StringArray{"Live","Analog FIR","Linear FIR"},0));
    result.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"linearResolution",5},"Linear Resolution",juce::StringArray{"Low","Medium","High","Very High","Maximum"},1));
    result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"mix",6},"Mix",juce::NormalisableRange<float>(0,1,.001f),1));
    return result;
}
EQProcessor::EQProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withInput("Sidechain",juce::AudioChannelSet::stereo(),false).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"DistillazioneState",layout()) {
    for(int i=0;i<24;++i) raw[size_t(i)]={parameters.getRawParameterValue(bandID(i,"on")),parameters.getRawParameterValue(bandID(i,"shape")),parameters.getRawParameterValue(bandID(i,"frequency")),parameters.getRawParameterValue(bandID(i,"gain")),parameters.getRawParameterValue(bandID(i,"q")),parameters.getRawParameterValue(bandID(i,"slope")),parameters.getRawParameterValue(bandID(i,"mode")),parameters.getRawParameterValue(bandID(i,"placement")),parameters.getRawParameterValue(bandID(i,"threshold")),parameters.getRawParameterValue(bandID(i,"range")),parameters.getRawParameterValue(bandID(i,"attack")),parameters.getRawParameterValue(bandID(i,"release")),parameters.getRawParameterValue(bandID(i,"external")),parameters.getRawParameterValue(bandID(i,"free")),parameters.getRawParameterValue(bandID(i,"triggerLow")),parameters.getRawParameterValue(bandID(i,"triggerHigh"))};
    mixRaw=parameters.getRawParameterValue("mix");
    outputRaw=parameters.getRawParameterValue("output"); bypassRaw=parameters.getRawParameterValue("bypass");balanceRaw=parameters.getRawParameterValue("balance");balanceModeRaw=parameters.getRawParameterValue("balanceMode");invertRaw=parameters.getRawParameterValue("invert");phaseRaw=parameters.getRawParameterValue("processingPhase");resolutionRaw=parameters.getRawParameterValue("linearResolution");
    phase.configuration=[this]{PhaseEngine::Configuration config;config.mode=int(phaseRaw->load());config.resolution=int(resolutionRaw->load());config.rate=rate;config.channels=getMainBusNumOutputChannels();for(int i=0;i<24;++i){config.bands[size_t(i)]=settings(i);if(config.bands[size_t(i)].mode==1)config.dynamics[size_t(i)]=dynamicAmount[size_t(i)].load();}return config;};
    parameters.addParameterListener("processingPhase",this);parameters.addParameterListener("linearResolution",this);
    for(int i=0;i<24;++i)parameters.addParameterListener(bandID(i,"mode"),this);
}
EQProcessor::~EQProcessor(){phase.shutdown();parameters.removeParameterListener("processingPhase",this);parameters.removeParameterListener("linearResolution",this);cancelPendingUpdate();for(int i=0;i<24;++i)parameters.removeParameterListener(bandID(i,"mode"),this);}
void EQProcessor::parameterChanged(const juce::String& id,float value){
    for(int i=0;i<24;++i)if(id==bandID(i,"mode")){const unsigned bit=1u<<unsigned(i);if(int(value)==2)spectralModes.fetch_or(bit);else spectralModes.fetch_and(~bit);break;}
    // Host latency notifications belong on the message thread, not in render.
    if(juce::MessageManager::getInstance()->isThisTheMessageThread())handleAsyncUpdate();else triggerAsyncUpdate();
}
void EQProcessor::handleAsyncUpdate(){const int latency=(spectralModes.load()!=0?SpectralEngine::size:0)+PhaseEngine::delay(int(phaseRaw->load()),int(resolutionRaw->load()),rate);setLatencySamples(latency);processingLatency.store(latency);}
void EQProcessor::refreshProcessingMode(){unsigned mask=0;for(int i=0;i<24;++i)if(settings(i).mode==2)mask|=1u<<unsigned(i);spectralModes.store(mask);handleAsyncUpdate();}
bool EQProcessor::isBusesLayoutSupported(const BusesLayout& l) const { if(l.inputBuses.size()!=2||l.outputBuses.size()!=1)return false;const auto side=l.inputBuses[1];if(!side.isDisabled()&&side!=juce::AudioChannelSet::mono()&&side!=juce::AudioChannelSet::stereo())return false;return l.getMainInputChannelSet()==l.getMainOutputChannelSet()&&(l.getMainOutputChannelSet()==juce::AudioChannelSet::mono()||l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo()); }
distillazione::BandSettings EQProcessor::settings(int i) const {
    const auto& r=raw[size_t(i)];
    return {r.on->load()>0.5f,static_cast<distillazione::Shape>(int(r.shape->load())),r.frequency->load(),r.gain->load(),r.q->load(),int(r.slope->load()),int(r.mode->load()),int(r.placement->load()),r.threshold->load(),r.range->load(),r.attack->load(),r.release->load(),r.external->load()>0.5f,r.freeTrigger->load()>0.5f,r.triggerLow->load(),r.triggerHigh->load()};
}
void EQProcessor::prepareToPlay(double sr,int maxBlock) {
    phase.shutdown();for(auto& peak:meterPeak)peak.store(0);for(auto& amount:dynamicAmount)amount.store(0);rate=sr;phase.prepare(sr,std::max(1,maxBlock));lastPhaseMode=int(phaseRaw->load()); for(auto& level:detectorLevel)level.store(-100);coefficientTick=0;spectrum.prepare(sr);sidechainSpectrum.prepare(sr);spectral.prepare(sr);
    refreshProcessingMode();spectralWasActive=spectralModes.load()!=0;
    for(int i=0;i<24;++i) {
        auto& b=bands[size_t(i)]; auto p=settings(i);
        for(auto& channel:b.states) for(auto& state:channel) state.reset();
        b.frequency.reset(sr,0.025);b.q.reset(sr,0.025);b.gain.reset(sr,0.025);b.wet.reset(sr,0.02);b.shapeMix.reset(sr,0.015);b.shapeMix.setCurrentAndTargetValue(1);b.wasActive=p.enabled;
        b.frequency.setCurrentAndTargetValue(p.frequency);b.q.setCurrentAndTargetValue(p.q);b.gain.setCurrentAndTargetValue(p.gain);b.wet.setCurrentAndTargetValue(p.enabled?1:0);
        b.envelope=0;b.change=0;for(auto& channel:b.freeStates)for(auto& state:channel)state.reset();for(auto& state:b.detectorStates)state.reset();for(auto& channel:b.dynamicStates)for(auto& state:channel)state.reset();
        b.mode=p.mode;b.placement=p.placement;
        b.shape=int(p.shape);b.slope=p.slope; p.enabled=true; b.coefficients=distillazione::designStaticBand(p,rate);
    }
    blend.reset(sr,.02);blend.setCurrentAndTargetValue(mixRaw->load());
    output.reset(sr,0.02);output.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(double(outputRaw->load())));
    balance.reset(sr,.02);balance.setCurrentAndTargetValue(balanceRaw->load());polarity.reset(sr,.02);polarity.setCurrentAndTargetValue(invertRaw->load()>.5f?-1:1);
    bypass.reset(sr,0.02);bypass.setCurrentAndTargetValue(bypassRaw->load()>0.5f?0:1);
}
void EQProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    const int channels=getMainBusNumOutputChannels();
    const int phaseMode=int(phaseRaw->load());phase.process(buffer,channels,phaseMode,int(resolutionRaw->load()),getChannelCountOfBus(true,1));
    if(phaseMode!=lastPhaseMode){for(auto& b:bands){for(auto& channel:b.states)for(auto& state:channel)state.reset();for(auto& channel:b.dynamicStates)for(auto& state:channel)state.reset();b.shapeMix.setCurrentAndTargetValue(1);}coefficientTick=0;lastPhaseMode=phaseMode;}
    auto sidechain=getBusBuffer(buffer,true,1);
    const int sideChannels=getChannelCountOfBus(true,1);
    float inPeak=0,outPeak=0;
    std::array<float,2> channelPeaks{};
    std::array<distillazione::BandSettings,24> configured;
    for(int i=0;i<24;++i) {
        auto p=settings(i);configured[size_t(i)]=p;auto& b=bands[size_t(i)];
        if(b.external!=p.externalSidechain){for(auto& state:b.detectorStates)state.reset();b.envelope=0;}
        if(b.placement!=p.placement){for(auto& channel:b.states)for(auto& state:channel)state.reset();for(auto& channel:b.previousStates)for(auto& state:channel)state.reset();for(auto& channel:b.dynamicStates)for(auto& state:channel)state.reset();for(auto& state:b.detectorStates)state.reset();b.envelope=0;b.change=0;}
        if(b.freeTrigger!=p.freeTrigger||b.external!=p.externalSidechain||b.placement!=p.placement){for(auto& channel:b.freeStates)for(auto& state:channel)state.reset();b.envelope=0;}
        b.freeTrigger=p.freeTrigger;b.triggerLow=std::min(p.triggerLow,p.triggerHigh);b.triggerHigh=std::max(p.triggerLow,p.triggerHigh);
        b.triggerLowFilter=distillazione::designStaticBand({true,distillazione::Shape::LowCut,b.triggerLow,0,1,1},rate).sections[0];b.triggerHighFilter=distillazione::designStaticBand({true,distillazione::Shape::HighCut,b.triggerHigh,0,1,1},rate).sections[0];
        b.external=p.externalSidechain;b.mode=p.mode;b.placement=p.placement;b.threshold=p.threshold;b.range=p.range;
        b.attackCoefficient=std::exp(-1/(rate*.001*p.attack));b.releaseCoefficient=std::exp(-1/(rate*.001*p.release));
        b.frequency.setTargetValue(p.frequency);b.q.setTargetValue(p.q);b.gain.setTargetValue(p.gain);b.wet.setTargetValue(p.enabled?1:0);
        if(b.shape!=int(p.shape)||b.slope!=p.slope) {
            b.previousStates=b.states;b.previousCoefficients=b.coefficients;
            b.shapeMix.setCurrentAndTargetValue(0);b.shapeMix.setTargetValue(1);
            b.shape=int(p.shape);b.slope=p.slope;
            for(auto& channel:b.states) for(auto& state:channel) state.reset();
            coefficientTick=0;
        }
    }
    const bool spectralActive=spectralModes.load()!=0;
    if(spectralActive!=spectralWasActive){spectral.clearHistory();spectralWasActive=spectralActive;}
    if(spectralActive)spectral.configure(configured);
    blend.setTargetValue(mixRaw->load());
    output.setTargetValue(juce::Decibels::decibelsToGain(double(outputRaw->load())));bypass.setTargetValue(bypassRaw->load()>0.5f?0:1);
    balance.setTargetValue(balanceRaw->load());polarity.setTargetValue(invertRaw->load()>.5f?-1:1);
    for(int sample=0;sample<buffer.getNumSamples();++sample) {
        std::array<double,2> original{},values{},key{};
        if(sideChannels>0){key[0]=sidechain.getSample(0,sample);key[1]=sidechain.getSample(sideChannels>1?1:0,sample);}
        for(int ch=0;ch<channels;++ch) { values[size_t(ch)]=buffer.getSample(ch,sample);original[size_t(ch)]=phaseMode?phase.dryBuffer().getSample(ch,sample):values[size_t(ch)];inPeak=std::max(inPeak,float(std::abs(values[size_t(ch)]))); }
        for(auto& b:bands) {
            const double hz=b.frequency.getNextValue(),q=b.q.getNextValue(),gain=b.gain.getNextValue(),wet=b.wet.getNextValue();
            const double shapeMix=b.shapeMix.getNextValue();
            if(wet<=0){if(b.wasActive){for(auto& channel:b.states)for(auto& state:channel)state.reset();for(auto& channel:b.dynamicStates)for(auto& state:channel)state.reset();for(auto& state:b.detectorStates)state.reset();b.envelope=0;b.change=0;}b.wasActive=false;continue;}
            b.wasActive=true;
            if(coefficientTick==0){b.coefficients=distillazione::designStaticBand({true,static_cast<distillazione::Shape>(b.shape),hz,gain,q,b.slope},rate);b.detector=distillazione::designStaticBand({true,distillazione::Shape::BandPass,hz,0,q,1},rate).sections[0];}
            auto routed=values;int routeChannels=channels;
            if(b.placement==1)routeChannels=1;
            else if(b.placement==2){if(channels<2)continue;routed[0]=values[1];routeChannels=1;}
            else if(b.placement>=3){if(channels<2){if(b.placement==4)continue;}else{routed[0]=(values[0]+(b.placement==3?values[1]:-values[1]))*.7071067811865476;routeChannels=1;}}
            auto detectorInput=b.external?key:routed;
            if(phaseMode&&!b.external){detectorInput[0]=phase.sourceBuffer().getSample(0,sample);detectorInput[1]=channels>1?phase.sourceBuffer().getSample(1,sample):0;if(b.placement==2)detectorInput[0]=detectorInput[1];else if(b.placement>=3)detectorInput[0]=channels>1?(detectorInput[0]+(b.placement==3?detectorInput[1]:-detectorInput[1]))*.7071067811865476:(b.placement==3?detectorInput[0]:0);}

            if(b.external){if(b.placement==2)detectorInput[0]=key[1];else if(b.placement>=3)detectorInput[0]=channels>1?(key[0]+(b.placement==3?key[1]:-key[1]))*.7071067811865476:(b.placement==3?key[0]:0);}
            double power=0;for(int ch=0;ch<routeChannels;++ch){double detected=b.freeTrigger&&b.mode==1?b.freeStates[size_t(ch)][1].process(b.freeStates[size_t(ch)][0].process(detectorInput[size_t(ch)],b.triggerLowFilter),b.triggerHighFilter):b.detectorStates[size_t(ch)].process(detectorInput[size_t(ch)],b.detector);power+=detected*detected;}power/=std::max(1,routeChannels);
            double ec=power>b.envelope?b.attackCoefficient:b.releaseCoefficient;b.envelope=power+(b.envelope-power)*ec;
            double db=10*std::log10(std::max(1e-20,b.envelope));double target=b.mode==1?b.range*std::clamp((db-b.threshold)/12.0,0.0,1.0):0;
            double gc=std::abs(target)>std::abs(b.change)?b.attackCoefficient:b.releaseCoefficient;b.change=target+(b.change-target)*gc;
            if(coefficientTick==0){auto dynamicShape=distillazione::dynamicShapeFor(static_cast<distillazione::Shape>(b.shape));b.dynamicFilter=distillazione::designStaticBand({true,dynamicShape,hz,b.change,q,1},rate);}
            if(phaseMode)continue;
            for(int ch=0;ch<routeChannels;++ch){double dry=routed[size_t(ch)],v=dry;for(int stage=0;stage<b.coefficients.count;++stage)v=b.states[size_t(ch)][size_t(stage)].process(v,b.coefficients.sections[size_t(stage)]);
                if(shapeMix<1){double previous=dry;for(int stage=0;stage<b.previousCoefficients.count;++stage)previous=b.previousStates[size_t(ch)][size_t(stage)].process(previous,b.previousCoefficients.sections[size_t(stage)]);v=previous+(v-previous)*shapeMix;}
                for(int stage=0;stage<b.dynamicFilter.count;++stage)v=b.dynamicStates[size_t(ch)][size_t(stage)].process(v,b.dynamicFilter.sections[size_t(stage)]);routed[size_t(ch)]=dry+(v-dry)*wet;
            }
            if(b.placement==0)values=routed;
            else if(b.placement==1||channels==1)values[0]=routed[0];
            else if(b.placement==2)values[1]=routed[0];
            else{double difference=(routed[0]-(values[0]+(b.placement==3?values[1]:-values[1]))*.7071067811865476)*.7071067811865476;values[0]+=difference;values[1]+=(b.placement==3?difference:-difference);}
        }
        coefficientTick=(coefficientTick+1)%16;
        const double level=output.getNextValue(),mix=bypass.getNextValue()*blend.getNextValue();
        auto processed=values;
        if(spectralActive)processed=spectral.process(values,original,channels,mix,level,phaseMode?phase.keyAt(sample):key);
        else for(int ch=0;ch<channels;++ch)processed[size_t(ch)]=original[size_t(ch)]+(values[size_t(ch)]*level-original[size_t(ch)])*mix;
        const double panValue=balance.getNextValue()*mix,pan=std::abs(panValue)<1e-6?0:panValue,sign=1+(polarity.getNextValue()-1)*mix;
        if(channels==2){const double a=pan>0?1-pan:1,b=pan<0?1+pan:1;if(balanceModeRaw->load()>.5f){double mid=(processed[0]+processed[1])*.7071067811865476*a,side=(processed[0]-processed[1])*.7071067811865476*b;processed[0]=(mid+side)*.7071067811865476;processed[1]=(mid-side)*.7071067811865476;}else{processed[0]*=a;processed[1]*=b;}}
        for(auto& value:processed)value*=sign;
        for(int ch=0;ch<channels;++ch){double value=processed[size_t(ch)];if(!std::isfinite(value)||std::abs(value)>std::numeric_limits<float>::max())value=0;buffer.setSample(ch,sample,float(value));outPeak=std::max(outPeak,float(std::abs(value)));channelPeaks[size_t(ch)]=std::max(channelPeaks[size_t(ch)],float(std::abs(value)));}
        sidechainSpectrum.push(float(key[0]),float(key[1]),float(key[0]),float(key[1]));
        if(channels>0)spectrum.push(float(original[0]),float(original[size_t(channels>1?1:0)]),buffer.getSample(0,sample),buffer.getSample(channels>1?1:0,sample));
    }
    for(int i=0;i<24;++i)detectorLevel[size_t(i)].store(float(10*std::log10(std::max(1e-10,bands[size_t(i)].envelope))));
    for(int i=0;i<24;++i)dynamicAmount[size_t(i)].store(bands[size_t(i)].mode==2?(spectralActive?spectral.amounts[size_t(i)]:0):float(bands[size_t(i)].change));
    inputPeak.store(inPeak);outputPeak.store(outPeak);
    for(int ch=0;ch<2;++ch){const float value=channelPeaks[size_t(channels>1?ch:0)];auto& peak=meterPeak[size_t(ch)];float previous=peak.load(std::memory_order_relaxed);while(value>previous&&!peak.compare_exchange_weak(previous,value,std::memory_order_relaxed)){} }
    channelOutputPeak[0].store(channelPeaks[0]);channelOutputPeak[1].store(channels>1?channelPeaks[1]:channelPeaks[0]);
}
void EQProcessor::setValue(const juce::String& id,float value) { if(auto* p=parameters.getParameter(id)) {p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(value));p->endChangeGesture();} }
void EQProcessor::getStateInformation(juce::MemoryBlock& data) {auto state=parameters.copyState();state.setProperty("schema",7,nullptr);if(auto xml=state.createXml())copyXmlToBinary(*xml,data);}
void EQProcessor::setStateInformation(const void* data,int bytes) {if(auto xml=getXmlFromBinary(data,bytes))if(xml->hasTagName(parameters.state.getType())){auto incoming=juce::ValueTree::fromXml(*xml);for(auto* parameter:getParameters())if(auto* ranged=dynamic_cast<juce::RangedAudioParameter*>(parameter))if(!incoming.getChildWithProperty("id",ranged->paramID).isValid()){juce::ValueTree child("PARAM");child.setProperty("id",ranged->paramID,nullptr);child.setProperty("value",ranged->convertFrom0to1(ranged->getDefaultValue()),nullptr);incoming.appendChild(child,nullptr);}parameters.replaceState(incoming);}}
juce::AudioProcessorEditor* EQProcessor::createEditor(){return new EQEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new EQProcessor();}

void EQProcessor::copyBand(int band,const distillazione::BandSettings& p){for(auto pair:std::initializer_list<std::pair<const char*,float>>{{"on",p.enabled?1.0f:0.0f},{"shape",float(p.shape)},{"frequency",float(p.frequency)},{"gain",float(p.gain)},{"q",float(p.q)},{"slope",float(p.slope)},{"mode",float(p.mode)},{"placement",float(p.placement)},{"threshold",float(p.threshold)},{"range",float(p.range)},{"attack",float(p.attack)},{"release",float(p.release)},{"external",p.externalSidechain?1.0f:0.0f},{"free",p.freeTrigger?1.0f:0.0f},{"triggerLow",float(p.triggerLow)},{"triggerHigh",float(p.triggerHigh)}})setValue(bandID(band,pair.first),pair.second);}
bool EQProcessor::splitBand(int band){auto p=settings(band);for(int target=0;target<24;++target)if(target!=band&&!settings(target).enabled){p.gain*=.5;p.range*=.5;copyBand(band,p);copyBand(target,p);return true;}return false;}
