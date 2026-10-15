// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "StaticEQ.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

int main() {
    using namespace distillazione;
    int checks=0;
    auto require=[&](bool condition,const char* message){++checks;if(!condition)throw std::runtime_error(message);};
    try {
        for(double sr:{44100.,48000.,96000.}) {
            for(int shape=0;shape<10;++shape)for(double hz:{10.,1000.,sr*0.47})for(double q:{0.1,0.70710678,20.}) {
                BandSettings p{true,static_cast<Shape>(shape),hz,12,q,6};
                auto bank=designStaticBand(p,sr);
                for(double theta:{0.01,0.2,1.0,2.8})require(std::isfinite(std::abs(bank.response(theta))),"Finite frequency response");
                std::array<DigitalSectionState,8> states;
                for(int i=0;i<2048;++i){double sample=i==0?1:0;for(int j=0;j<bank.count;++j)sample=states[size_t(j)].process(sample,bank.sections[size_t(j)]);require(std::isfinite(sample),"Finite impulse");}
                p.enabled=false;auto off=designStaticBand(p,sr);require(std::abs(off.response(1)-std::complex<double>(1,0))<1e-12,"Disabled identity");
            }
            const double theta=2*std::acos(-1.0)*1000/sr;
            auto bell=designStaticBand({true,Shape::Bell,1000,12,0.70710678,1},sr);
            require(std::abs(20*std::log10(std::abs(bell.response(theta)))-12)<1e-8,"Bell gain at centre");
            auto measuredBell=designStaticBand({true,Shape::Bell,1000,6,1,1},sr);
            const double halfFrequencyTheta=theta/2;
            const double halfFrequencyGain=20*std::log10(std::abs(measuredBell.response(halfFrequencyTheta)));
            require(halfFrequencyGain>2.75 && halfFrequencyGain<2.9,"Public Bell Q bandwidth");
            for(auto cutShape:{Shape::LowCut,Shape::HighCut}) {
                auto cut=designStaticBand({true,cutShape,1000,0,0.70710678,1},sr);
                require(std::abs(20*std::log10(std::abs(cut.response(theta)))+6.020599913)<1e-6,"12 dB cut critical corner");
            }
            auto highCut=designStaticBand({true,Shape::HighCut,sr*0.2,0,0.70710678,1},sr);
            const double ratio=2.5;
            const double analogNyquistMagnitude=1/(1+ratio*ratio);
            require(std::abs(std::abs(highCut.response(std::acos(-1.0)))-analogNyquistMagnitude)<1e-8,"Decramped high cut finite Nyquist target");
            for(double gain:{-24.,-12.,0.,12.,24.})for(double hz:{1000.,6000.,sr*.45})for(double q:{.1,1.,4.,20.}){
                auto bell=designStaticBand({true,Shape::Bell,hz,gain,q,1},sr);
                const double error=20*std::log10(std::abs(bell.response(2*std::acos(-1.)*hz/sr)))-gain;if(std::abs(error)>=1e-6)std::cerr<<"Bell sr="<<sr<<" hz="<<hz<<" gain="<<gain<<" q="<<q<<" error="<<error<<"\n";require(std::abs(error)<1e-6,"Decramped Bell centre gain");
                auto c=bell.sections[0];std::complex<double> d=std::sqrt(std::complex<double>(c.a1*c.a1-4*c.a2,0));
                require(std::abs((-c.a1+d)/2.)<1&&std::abs((-c.a1-d)/2.)<1,"Decramped Bell poles stable");
            }
            auto notch=designStaticBand({true,Shape::Notch,1000,0,0.70710678,1},sr);
            require(std::abs(notch.response(theta))<1e-8,"Notch null at centre");
            auto pass=designStaticBand({true,Shape::BandPass,1000,0,0.70710678,1},sr);
            require(std::abs(std::abs(pass.response(theta))-1)<1e-10,"Band pass peak");
            auto phase=designStaticBand({true,Shape::AllPass,1000,0,0.70710678,1},sr);
            require(std::abs(std::abs(phase.response(2.3))-1)<1e-10,"All pass magnitude");
        }
        std::cout<<checks<<" static EQ checks passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
