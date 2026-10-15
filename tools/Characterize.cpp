// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "Processor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <complex>
#include <fstream>
#include <iomanip>
#include <iostream>
int main(int argc,char** argv){
    juce::ScopedJuceInitialiser_GUI gui;
    if(argc!=2)return 2;
    std::ofstream out(argv[1]);if(!out)return 3;
    out<<"case,frequency_hz,gain_db,output_peak,reported_latency_samples\n"<<std::setprecision(10);
    struct Case{const char* name;int shape;float center,gain;};
    for(auto test:{Case{"Neutral",0,1000,0},Case{"Bell +6 dB 1k Q1",0,1000,6},Case{"Low shelf +6 dB 100 Hz",1,100,6},Case{"Low cut 80 Hz 24 dB/oct",2,80,0},Case{"High cut 12k 24 dB/oct",4,12000,0}})
    for(double hz:{20.,50.,100.,200.,500.,1000.,2000.,5000.,10000.,16000.,20000.}){
        auto p=std::make_unique<EQProcessor>();p->setValue(EQProcessor::bandID(0,"on"),1);p->setValue(EQProcessor::bandID(0,"shape"),float(test.shape));p->setValue(EQProcessor::bandID(0,"frequency"),test.center);p->setValue(EQProcessor::bandID(0,"gain"),test.gain);p->setValue(EQProcessor::bandID(0,"slope"),2);p->prepareToPlay(48000,128);
        juce::MidiBuffer midi;juce::AudioBuffer<float> signal(2,128);std::complex<double> amplitude{};int count=0;double peak=0;
        for(int start=0;start<48000;start+=128){const int size=std::min(128,48000-start);signal.setSize(2,size,false,false,true);
            for(int i=0;i<size;++i){float value=float(.05*std::sin(juce::MathConstants<double>::twoPi*hz*(start+i)/48000));signal.setSample(0,i,value);signal.setSample(1,i,value);}p->processBlock(signal,midi);
            for(int i=0;i<size;++i)if(start+i>=24000){double value=signal.getSample(0,i);amplitude+=value*std::polar(1.0,-juce::MathConstants<double>::twoPi*hz*(start+i)/48000);peak=std::max(peak,std::abs(value));++count;}}
        out<<test.name<<','<<hz<<','<<20*std::log10(std::max(1e-15,2*std::abs(amplitude)/count/.05))<<','<<peak<<','<<p->getLatencySamples()<<'\n';
    }
    std::cout<<"55 single-product sine measurements written.\n";
}
