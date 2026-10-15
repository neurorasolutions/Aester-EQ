// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#pragma once
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

// One audio producer, one editor consumer. A full queue drops analysis frames,
// never blocks audio; FFT and windowing run only on the message thread.
class SpectrumTap {
public:
    static constexpr int size=2048;
    struct Frame { std::array<std::array<float,size>,4> samples{}; double rate=48000; };
    std::atomic<bool> enabled{false};
    void prepare(double rate) noexcept { pending.rate=rate; position=0; captured=0; hop=0; }
    void push(float preL,float preR,float postL,float postR) noexcept {
        if(!enabled.load(std::memory_order_relaxed)){position=0;captured=0;hop=0;return;}
        pending.samples[0][size_t(position)]=preL;pending.samples[1][size_t(position)]=preR;
        pending.samples[2][size_t(position)]=postL;pending.samples[3][size_t(position)]=postR;
        position=(position+1)%size;captured=std::min(size,captured+1);
        if(++hop<512||captured<size)return;hop=0;
        int start,n,unused,unusedCount;fifo.prepareToWrite(1,start,n,unused,unusedCount);
        if(n==1){auto& frame=frames[size_t(start)];frame.rate=pending.rate;for(int ch=0;ch<4;++ch)for(int i=0;i<size;++i)frame.samples[size_t(ch)][size_t(i)]=pending.samples[size_t(ch)][size_t((position+i)%size)];fifo.finishedWrite(1);}
    }
    bool latest(Frame& result) noexcept {
        bool received=false;
        const int ready=fifo.getNumReady();
        for(int i=0;i<ready;++i){int start,n,unused,unusedCount;fifo.prepareToRead(1,start,n,unused,unusedCount);if(n==1){result=frames[size_t(start)];fifo.finishedRead(1);received=true;}}
        return received;
    }
private:
    juce::AbstractFifo fifo{4};
    std::array<Frame,4> frames;
    Frame pending;
    int position=0,captured=0,hop=0;
};

class SpectrumDisplay {
public:
    static constexpr int bins=SpectrumTap::size/2+1;
    SpectrumDisplay(){pre.fill(-100);post.fill(-100);}
    bool update(SpectrumTap& tap) {
        if(!tap.latest(frame)){if(++idleTicks>6)for(auto* values:{&pre,&post})for(auto& db:*values)db=std::max(-100.0f,db-.6f);return false;}idleTicks=0;
        rate=frame.rate;
        std::array<std::array<float,bins>,2> energy{};
        for(int channel=0;channel<4;++channel){data.fill(0);for(int i=0;i<SpectrumTap::size;++i)data[size_t(i)]=frame.samples[size_t(channel)][size_t(i)];
            window.multiplyWithWindowingTable(data.data(),SpectrumTap::size);
            fft.performFrequencyOnlyForwardTransform(data.data(),true);
            for(int i=0;i<bins;++i){float magnitude=data[size_t(i)]*(2.0f/SpectrumTap::size);energy[size_t(channel/2)][size_t(i)]+=magnitude*magnitude*0.5f;}
        }
        for(int i=0;i<bins;++i)for(int stage=0;stage<2;++stage){float db=juce::Decibels::gainToDecibels(std::sqrt(energy[size_t(stage)][size_t(i)]),-100.0f);auto& smoothed=(stage==0?pre:post)[size_t(i)];smoothed+=(db-smoothed)*(db>smoothed?0.45f:0.12f);}
        return true;
    }
    float displayDb(int stage,double bin) const {
        const auto& values=stage==0?pre:post;const int centre=juce::jlimit(1,bins-2,int(bin));
        const int radius=std::max(1,int(bin*.018));double power=0,weight=0;
        for(int i=std::max(1,centre-radius);i<=std::min(bins-1,centre+radius);++i){const double w=std::max(0.0,1-std::abs(i-bin)/double(radius+1));power+=w*std::pow(10.0,values[size_t(i)]/10);weight+=w;}
        return float(10*std::log10(std::max(1e-10,power/std::max(1e-10,weight))));
    }
    // Apply visual tilt only to energy above the analyzer floor. Tilting the
    // floor itself creates a false rising tail even on nearly silent bins.
    static float tiltedDb(float db,double hz,float tilt) {
        const double signal=std::max(0.0,std::pow(10.0,double(db)/10)-1e-10);
        const double factor=std::pow(10.0,double(tilt)*std::log2(std::max(1.0,hz)/1000)/10);
        return float(10*std::log10(1e-10+signal*factor));
    }
    std::array<float,bins> pre,post;
    double rate=48000;
private:
    int idleTicks=0;
    SpectrumTap::Frame frame;
    juce::dsp::FFT fft{11};
    juce::dsp::WindowingFunction<float> window{SpectrumTap::size,juce::dsp::WindowingFunction<float>::hann,true};
    std::array<float,2*SpectrumTap::size> data{};
};
