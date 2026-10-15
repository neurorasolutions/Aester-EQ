// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#pragma once
#include "StaticEQ.h"
#include <juce_dsp/juce_dsp.h>
#include <functional>

// Independent frequency-sampled FIR implementation. Kernel design and disposal
// are off the render thread; JUCE partitioned convolution performs rendering.
class PhaseEngine final:private juce::Thread {
public:
 bool isReady()const{return readyState.load();}
 struct Configuration {std::array<distillazione::BandSettings,24> bands;std::array<float,24> dynamics{};int mode=0,resolution=1,channels=2;double rate=48000;};
 std::function<Configuration()> configuration;
 PhaseEngine():Thread("PRO N phase design"){for(auto& c:convolvers)c=std::make_unique<juce::dsp::Convolution>(juce::dsp::Convolution::NonUniform{256},messageQueue);}
 void shutdown(){stopThread(10000);}
 ~PhaseEngine(){stopThread(10000);delete pending.exchange(nullptr);delete retired.exchange(nullptr);}
 static int delay(int mode,int resolution,double rate){if(mode==0)return 0;if(mode==1)return 320;constexpr int samples[]{3072,5120,9216,17408,66560};return samples[std::clamp(resolution,0,4)]*(rate>64000?2:1);}
 void prepare(double sampleRate,int maxBlock){stopThread(10000);delete pending.exchange(nullptr);delete retired.exchange(nullptr);rate=sampleRate;scratch.setSize(4,maxBlock);source.setSize(2,maxBlock);delayedKey.setSize(2,maxBlock);keyHistory.setSize(2,133121);keyHistory.clear();dry.setSize(2,maxBlock);history.setSize(2,133121);history.clear();position=0;preparedConfig=configuration?configuration():Configuration{};lastMode=preparedConfig.mode;lastDelay=delay(preparedConfig.mode,preparedConfig.resolution,rate);expectedSize=0;settledSamples=0;
  if(preparedConfig.mode!=0){auto initial=design(preparedConfig);expectedSize=initial->length;for(int i=0;i<4;++i)convolvers[size_t(i)]->loadImpulseResponse(std::move(initial->data[size_t(i)]),rate,juce::dsp::Convolution::Stereo::no,juce::dsp::Convolution::Trim::no,juce::dsp::Convolution::Normalise::no);settledSamples=int(rate*.1);}
  for(auto& c:convolvers)c->prepare({rate,juce::uint32(maxBlock),1});readyState.store(preparedConfig.mode!=0);startThread();}
 void process(juce::AudioBuffer<float>& buffer,int channels,int mode,int resolution,int keyChannels=0){
  const int count=buffer.getNumSamples(),latency=delay(mode,resolution,rate);
  if(mode!=lastMode||latency!=lastDelay){history.clear();keyHistory.clear();position=0;for(auto& c:convolvers)c->reset();lastMode=mode;lastDelay=latency;expectedSize=0;settledSamples=0;readyState.store(false);}
  for(int i=0;i<count;++i){for(int ch=0;ch<2;++ch){float x=ch<channels?buffer.getSample(ch,i):0;const float key=keyChannels>0?buffer.getSample(channels+(keyChannels>1?ch:0),i):0;keyHistory.setSample(ch,position,key);delayedKey.setSample(ch,i,keyHistory.getSample(ch,(position-latency+keyHistory.getNumSamples())%keyHistory.getNumSamples()));source.setSample(ch,i,x);history.setSample(ch,position,x);dry.setSample(ch,i,history.getSample(ch,(position-latency+history.getNumSamples())%history.getNumSamples()));scratch.setSample(ch*2,i,x);scratch.setSample(ch*2+1,i,x);}position=(position+1)%history.getNumSamples();}
  if(mode==0)return;
  if(retired.load()==nullptr)if(auto* kernels=pending.exchange(nullptr)){if(kernels->mode==mode&&kernels->latency==latency){expectedSize=kernels->length;for(int i=0;i<4;++i)convolvers[size_t(i)]->loadImpulseResponse(std::move(kernels->data[size_t(i)]),rate,juce::dsp::Convolution::Stereo::no,juce::dsp::Convolution::Trim::no,juce::dsp::Convolution::Normalise::no);}retired.store(kernels);}
  bool ready=expectedSize>0;for(int i=0;i<4;++i){auto block=juce::dsp::AudioBlock<float>(scratch).getSingleChannelBlock(size_t(i)).getSubBlock(0,size_t(count));juce::dsp::ProcessContextReplacing<float> context(block);convolvers[size_t(i)]->process(context);ready=ready&&convolvers[size_t(i)]->getCurrentIRSize()==expectedSize;}
  if(ready)settledSamples+=count;else settledSamples=0;ready=ready&&settledSamples>=int(rate*.1);
  readyState.store(ready);
  for(int i=0;i<count;++i){buffer.setSample(0,i,ready?scratch.getSample(0,i)+scratch.getSample(2,i):dry.getSample(0,i));if(channels>1)buffer.setSample(1,i,ready?scratch.getSample(1,i)+scratch.getSample(3,i):dry.getSample(1,i));}
 }
 std::array<double,2> keyAt(int sample)const{return {delayedKey.getSample(0,sample),delayedKey.getSample(1,sample)};}
 const juce::AudioBuffer<float>& sourceBuffer()const{return source;}
 const juce::AudioBuffer<float>& dryBuffer()const{return dry;}
private:
 struct Kernels{std::array<juce::AudioBuffer<float>,4> data;int mode=0,latency=0,length=0;};
 juce::dsp::ConvolutionMessageQueue messageQueue{32};
 std::array<std::unique_ptr<juce::dsp::Convolution>,4> convolvers;
 juce::AudioBuffer<float> scratch,source,dry,history,delayedKey,keyHistory;
 std::atomic<bool> readyState{false};
 std::atomic<Kernels*> pending{nullptr},retired{nullptr};
 Configuration preparedConfig;
 double rate=48000;int position=0,lastMode=-1,lastDelay=-1,expectedSize=0,settledSamples=0;
 static bool same(const Configuration& a,const Configuration& b){if(a.mode!=b.mode||a.resolution!=b.resolution||a.rate!=b.rate)return false;for(int i=0;i<24;++i){const auto& x=a.bands[size_t(i)];const auto& y=b.bands[size_t(i)];if(x.enabled!=y.enabled||x.shape!=y.shape||x.frequency!=y.frequency||x.gain!=y.gain||x.q!=y.q||x.slope!=y.slope||x.placement!=y.placement||std::abs(a.dynamics[size_t(i)]-b.dynamics[size_t(i)])>.02f)return false;}return true;}
 void run()override{Configuration previous=preparedConfig;while(!threadShouldExit()){delete retired.exchange(nullptr);if(configuration){auto current=configuration();if(current.mode!=0&&!same(current,previous)){auto result=design(current);if(threadShouldExit())break;delete pending.exchange(result.release());previous=current;}}wait(25);}}
 std::unique_ptr<Kernels> design(const Configuration& config){
  auto result=std::make_unique<Kernels>();result->mode=config.mode;result->latency=delay(config.mode,config.resolution,config.rate);result->length=config.mode==1?16385:result->latency*2+1;
  int order=1;while((1<<order)<result->length)++order;const int size=1<<order;juce::dsp::FFT fft(order);using Complex=juce::dsp::Complex<float>;std::array<std::vector<Complex>,4> frequency;for(auto& v:frequency)v.resize(size_t(size));
  std::array<distillazione::SectionBank,24> banks,dynamicBanks;const double designRate=config.rate*32;for(int i=0;i<24;++i){const auto& p=config.bands[size_t(i)];banks[size_t(i)]=distillazione::designStaticBand(p,designRate);dynamicBanks[size_t(i)]=distillazione::designStaticBand({p.enabled,distillazione::dynamicShapeFor(p.shape),p.frequency,config.dynamics[size_t(i)],p.q,1},designRate);}
  for(int bin=0;bin<=size/2;++bin){const double hz=config.rate*bin/size,theta=2*juce::MathConstants<double>::pi*hz/designRate;std::complex<double> ll=1,lr=0,rl=0,rr=1;
   for(int i=0;i<24;++i){const auto& p=config.bands[size_t(i)];if(!p.enabled||(config.channels==1&&(p.placement==2||p.placement==4)))continue;auto h=banks[size_t(i)].response(theta)*dynamicBanks[size_t(i)].response(theta);if(config.mode==2)h=std::abs(h);std::complex<double> a=1,b=0,c=0,d=1;switch(config.channels==1&&p.placement==3?0:p.placement){case 1:a=h;break;case 2:d=h;break;case 3:a=d=(h+1.)*.5;b=c=(h-1.)*.5;break;case 4:a=d=(h+1.)*.5;b=c=(1.-h)*.5;break;default:a=d=h;break;}auto newLL=a*ll+b*rl,newLR=a*lr+b*rr,newRL=c*ll+d*rl,newRR=c*lr+d*rr;ll=newLL;lr=newLR;rl=newRL;rr=newRR;}
   const std::array<std::complex<double>,4> matrix{ll,rl,lr,rr};for(int m=0;m<4;++m){auto transfer=matrix[size_t(m)];const auto magnitude=std::abs(transfer);if(!std::isfinite(magnitude))transfer=0;else if(magnitude>1e20)transfer*=1e20/magnitude;auto value=Complex(float(transfer.real()),bin==0||bin==size/2?0:float(transfer.imag()));frequency[size_t(m)][size_t(bin)]=value;if(bin>0&&bin<size/2)frequency[size_t(m)][size_t(size-bin)]=std::conj(value);}
  }
  for(int m=0;m<4;++m){std::vector<Complex> impulse(size_t(size),Complex{});fft.perform(frequency[size_t(m)].data(),impulse.data(),true);auto& kernel=result->data[size_t(m)];kernel.setSize(1,result->length);for(int n=0;n<result->length;++n){const int lag=n-result->latency,index=(lag+size)%size;double taper=1;if(config.mode==2){const int span=result->latency-1024*(config.rate>64000?2:1);taper=std::abs(lag)>span?0:.5+.5*std::cos(juce::MathConstants<double>::pi*lag/(span+1));}else if(n>result->length-1024)taper=.5-.5*std::cos(juce::MathConstants<double>::pi*(result->length-n)/1024.);kernel.setSample(0,n,float(impulse[size_t(index)].real()*taper));}}
  return result;
 }
};
