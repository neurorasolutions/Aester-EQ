// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#pragma once
#include "StaticEQ.h"
#include <array>
#include <atomic>
#include <complex>
#include <cmath>
#include <algorithm>

// Fixed-storage STFT. Both processed and bypass paths have the same 1024-sample
// delay. Periodic sqrt-Hann windows, 75% overlap and 1/2 synthesis scaling give
// unity reconstruction. No allocation, locks or third-party FFT in render.
class SpectralEngine {
public:
    static constexpr int size=1024,hop=256,bins=size/2+1;
    void clearHistory() noexcept {position=0;clock=0;input={};keyInput={};dry={};output={};delta={};amounts={};for(auto& value:curveDb)value.store(0);}
    float responseAt(double hz) const noexcept {const double bin=std::clamp(hz*size/rate,0.0,double(bins-1));const int first=int(bin),next=std::min(first+1,bins-1);return curveDb[size_t(first)].load()+float(bin-first)*(curveDb[size_t(next)].load()-curveDb[size_t(first)].load());}
    void prepare(double sampleRate) {
        rate=sampleRate;clearHistory();valid.fill(false);
        for(int i=0;i<size;++i){window[size_t(i)]=float(std::sqrt(.5-.5*std::cos(2*pi*i/size)));roots[size_t(i)]=std::polar(1.0f,float(-2*pi*i/size));unsigned value=unsigned(i),reversed=0;for(int bit=0;bit<10;++bit){reversed=(reversed<<1)|(value&1);value>>=1;}reverse[size_t(i)]=int(reversed);}
    }
    void configure(const std::array<distillazione::BandSettings,24>& settings) {
        bands=settings;
        for(int band=0;band<24;++band){const auto& p=bands[size_t(band)];auto& previous=cached[size_t(band)];
            if(valid[size_t(band)]&&(p.placement!=previous.placement||p.externalSidechain!=previous.externalSidechain))delta[size_t(band)].fill(0);
            if(!valid[size_t(band)]||p.frequency!=previous.frequency||p.q!=previous.q||p.shape!=previous.shape){
                for(int bin=0;bin<bins;++bin){double hz=std::max(1.0,rate*bin/size),oct=std::log2(hz/std::clamp(p.frequency,10.0,rate*.475)),width=std::clamp(1.0/p.q,.08,4.0),x=oct/width;double weight=std::exp(-.5*x*x);
                    switch(p.shape){case distillazione::Shape::LowShelf:case distillazione::Shape::LowCut:weight=1/(1+std::exp(std::clamp(4*x,-40.0,40.0)));break;case distillazione::Shape::HighShelf:case distillazione::Shape::HighCut:weight=1/(1+std::exp(std::clamp(-4*x,-40.0,40.0)));break;case distillazione::Shape::TiltShelf:case distillazione::Shape::FlatTilt:weight=std::tanh(x);break;case distillazione::Shape::Bell:case distillazione::Shape::Notch:case distillazione::Shape::BandPass:case distillazione::Shape::AllPass:break;}
                    weights[size_t(band)][size_t(bin)]=float(weight);
                }previous=p;valid[size_t(band)]=true;
            }
            previous=p;
            attack[size_t(band)]=float(std::exp(-hop/(rate*.001*std::max(.1,p.attack))));release[size_t(band)]=float(std::exp(-hop/(rate*.001*std::max(1.0,p.release))));
        }
    }
    std::array<double,2> process(std::array<double,2> wet,std::array<double,2> original,int channels,double bypassMix,double outputGain=1,std::array<double,2> key={}) noexcept {
        std::array<double,2> result{};
        for(int ch=0;ch<2;++ch){auto c=size_t(ch);double delayedDry=dry[c][size_t(position)];dry[c][size_t(position)]=original[c];input[c][size_t(position)]=float(wet[c]);keyInput[c][size_t(position)]=float(key[c]);float filtered=output[c][size_t(clock)];output[c][size_t(clock)]=0;result[c]=delayedDry+(double(filtered)*outputGain-delayedDry)*bypassMix;}
        position=(position+1)%size;
        if(position%hop==0)render(channels);
        clock=(clock+1)%(2*size);return result;
    }
    std::array<float,24> amounts{};
private:
    static constexpr double pi=3.14159265358979323846;
    using Complex=std::complex<float>;
    std::array<distillazione::BandSettings,24> bands{},cached{};
    std::array<bool,24> valid{};
    std::array<std::array<float,bins>,24> weights{},delta{};
    std::array<float,24> attack{},release{};
    std::array<std::atomic<float>,bins> curveDb{};
    std::array<std::array<float,size>,2> input{},keyInput{};
    std::array<std::array<double,size>,2> dry{};
    std::array<std::array<float,2*size>,2> output{};
    std::array<std::array<Complex,size>,2> spectrum{},keySpectrum{};
    std::array<Complex,size> roots{};
    std::array<int,size> reverse{};
    std::array<float,size> window{};
    int position=0,clock=0;
    double rate=48000;
    void fft(std::array<Complex,size>& data,bool inverse) noexcept {
        for(int i=0;i<size;++i)if(i<reverse[size_t(i)])std::swap(data[size_t(i)],data[size_t(reverse[size_t(i)])]);
        for(int length=2;length<=size;length*=2)for(int start=0;start<size;start+=length)for(int j=0;j<length/2;++j){auto root=roots[size_t(j*size/length)];if(inverse)root=std::conj(root);auto a=data[size_t(start+j)],b=data[size_t(start+j+length/2)]*root;data[size_t(start+j)]=a+b;data[size_t(start+j+length/2)]=a-b;}
        if(inverse)for(auto& value:data)value/=float(size);
    }
    void render(int channels) noexcept {
        for(int ch=0;ch<2;++ch){for(int i=0;i<size;++i)spectrum[size_t(ch)][size_t(i)]={input[size_t(ch)][size_t((position+i)%size)]*window[size_t(i)],0};fft(spectrum[size_t(ch)],false);}
        const bool external=std::any_of(bands.begin(),bands.end(),[](const auto& p){return p.enabled&&p.mode==2&&p.externalSidechain;});
        if(external)for(int ch=0;ch<2;++ch){for(int i=0;i<size;++i)keySpectrum[size_t(ch)][size_t(i)]={keyInput[size_t(ch)][size_t((position+i)%size)]*window[size_t(i)],0};fft(keySpectrum[size_t(ch)],false);}
        std::array<bool,24> active{};for(int band=0;band<24;++band)active[size_t(band)]=(bands[size_t(band)].enabled&&bands[size_t(band)].mode==2)||std::abs(amounts[size_t(band)])>1e-5f;
        amounts.fill(0);
        for(int bin=0;bin<bins;++bin){auto originalL=spectrum[0][size_t(bin)],originalR=spectrum[1][size_t(bin)],left=originalL,right=originalR;
            for(int band=0;band<24;++band){if(!active[size_t(band)])continue;const auto& p=bands[size_t(band)];float magnitude=0;
                const auto detectorL=p.externalSidechain?keySpectrum[0][size_t(bin)]:originalL,detectorR=p.externalSidechain?keySpectrum[1][size_t(bin)]:originalR;
                switch(p.placement){case 1:magnitude=std::abs(detectorL);break;case 2:magnitude=channels>1?std::abs(detectorR):0;break;case 3:magnitude=channels>1?std::abs((detectorL+detectorR)*.70710678f):std::abs(detectorL);break;case 4:magnitude=channels>1?std::abs((detectorL-detectorR)*.70710678f):0;break;default:magnitude=channels>1?std::sqrt((std::norm(detectorL)+std::norm(detectorR))*.5f):std::abs(detectorL);break;}
                // sqrt-Hann coherent gain is approximately 2/pi.
                float db=20*std::log10(std::max(1e-10f,magnitude*float(pi/size)));
                float target=(p.enabled&&p.mode==2)?float(p.range)*weights[size_t(band)][size_t(bin)]*std::clamp((db-float(p.threshold))/12.0f,0.0f,1.0f):0;
                auto& change=delta[size_t(band)][size_t(bin)];float coefficient=std::abs(target)>std::abs(change)?attack[size_t(band)]:release[size_t(band)];change=target+(change-target)*coefficient;
                if(std::abs(change)>std::abs(amounts[size_t(band)]))amounts[size_t(band)]=change;
                if(std::abs(change)<1e-5f)continue;float gain=std::pow(10.0f,change/20);
                switch(p.placement){case 1:left*=gain;break;case 2:if(channels>1)right*=gain;break;case 3:case 4:if(channels==1){if(p.placement==3)left*=gain;}else{auto mid=(left+right)*.70710678f,side=(left-right)*.70710678f;if(p.placement==3)mid*=gain;else side*=gain;left=(mid+side)*.70710678f;right=(mid-side)*.70710678f;}break;default:left*=gain;right*=gain;break;}
            }
            const float before=std::norm(originalL)+(channels>1?std::norm(originalR):0),after=std::norm(left)+(channels>1?std::norm(right):0);
            curveDb[size_t(bin)].store(before>1e-18f?10*std::log10(std::max(1e-18f,after)/before):0);
            spectrum[0][size_t(bin)]=left;spectrum[1][size_t(bin)]=right;
            if(bin>0&&bin<size/2){spectrum[0][size_t(size-bin)]=std::conj(left);spectrum[1][size_t(size-bin)]=std::conj(right);}
        }
        for(int ch=0;ch<2;++ch){fft(spectrum[size_t(ch)],true);for(int i=0;i<size;++i)output[size_t(ch)][size_t((clock+1+i)%(2*size))]+=spectrum[size_t(ch)][size_t(i)].real()*window[size_t(i)]*.5f;}
    }
};
