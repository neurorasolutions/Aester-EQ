// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "StaticEQ.h"
#include <algorithm>
#include <cmath>

namespace distillazione {
std::complex<double> SectionBank::response(double theta) const {
    std::complex<double> result{1, 0};
    for (int i=0; i<count; ++i) result *= sections[size_t(i)].response(theta);
    return result;
}
namespace {
// Matched analog poles plus a minimum-phase numerator fitted at DC, the
// corner, and Nyquist. Unlike a bilinear low-pass, its Nyquist magnitude is
// finite. This is an independent decramping design, not a recovered formula.
DigitalSection decrampedLowPass(double hz, double q, double sr, DigitalSection fallback, double gainDb=0, bool bell=false) {
    using C = std::complex<double>;
    const double pi = std::acos(-1.0), w = 2*pi*hz/sr;
    const double A=std::pow(10.0,gainDb/40), analogQ=q; if(bell)q*=A;
    const C split = w*std::sqrt(C(1/(4*q*q)-1,0));
    const C p1 = std::exp(C(-w/(2*q),0)+split);
    const C p2 = std::exp(C(-w/(2*q),0)-split);
    const double a1=-(p1+p2).real(), a2=(p1*p2).real();
    auto denominatorPower = [&](double theta) {
        return std::norm(C(1,0)+a1*std::exp(C(0,-theta))+a2*std::exp(C(0,-2*theta)));
    };
    const double ratio=pi/w;
    const double nyquistPower=bell?(std::pow(1-ratio*ratio,2)+std::pow(ratio*A/analogQ,2))/(std::pow(1-ratio*ratio,2)+std::pow(ratio/(analogQ*A),2)):1/(std::pow(1-ratio*ratio,2)+std::pow(ratio/q,2));
    const double e0=denominatorPower(0), ep=nyquistPower*denominatorPower(pi);
    const double ec=(bell?std::pow(A,4):q*q)*denominatorPower(w), c=std::cos(w);
    const double c1=(e0-ep)/4, sum=(e0+ep)/2;
    const double c2=(ec-sum-2*c1*c)/(4*(c*c-1)), c0=sum-2*c2;
    if (!std::isfinite(c2) || std::abs(c2)<1e-20) return fallback;
    for (int i=0;i<=32;++i) {
        const double theta=pi*i/32;
        if(c0+2*c1*std::cos(theta)+2*c2*std::cos(2*theta)<-1e-12) return fallback;
    }
    const C discriminant=std::sqrt(C(c1*c1-4*c2*(c0-2*c2),0));
    auto insideZero = [](C t) {
        const C root=std::sqrt(t*t-C(4,0));
        const C first=(t+root)/2.0, second=(t-root)/2.0;
        return std::abs(first)<std::abs(second) ? first : second;
    };
    const C z1=insideZero((-c1+discriminant)/(2*c2));
    const C z2=insideZero((-c1-discriminant)/(2*c2));
    const C b1=-(z1+z2), b2=z1*z2;
    const C scale=(1+a1+a2)/(C(1,0)+b1+b2);
    const C scaledB1=scale*b1, scaledB2=scale*b2;
    if(std::abs(scale.imag())+std::abs(scaledB1.imag())+std::abs(scaledB2.imag())>1e-8
       || !std::isfinite(scale.real()) || !std::isfinite(scaledB1.real()) || !std::isfinite(scaledB2.real())) return fallback;
    const DigitalSection result{scale.real(),scaledB1.real(),scaledB2.real(),a1,a2};
    const double target=bell?std::pow(A,2):q;
    if(std::abs(std::abs(result.response(w))-target)>1e-7*std::max(1.0,target))return fallback;
    return result;
}
DigitalSection biquad(Shape shape, double hz, double db, double q, double sr) {
    if(shape==Shape::Bell&&std::abs(db)<1e-12)return {};
    // The public Bell Q describes a bandwidth wider than the cookbook Q.
    // Independent AU impulse fits at positive and negative gains support
    // this conversion; it does not compensate high-frequency warping.
    if (shape == Shape::Bell) q *= std::sqrt(0.5);
    if (shape == Shape::LowShelf || shape == Shape::HighShelf) q = std::sqrt(q*0.5);
    const double pi = std::acos(-1.0), w = 2*pi*hz/sr;
    const double c = std::cos(w), s = std::sin(w), alpha = s/(2*q);
    const double A = std::pow(10.0, db/40), beta = 2*std::sqrt(A)*alpha;
    double b0=1,b1=0,b2=0,a0=1,a1=0,a2=0;
    switch (shape) {
        case Shape::Bell: b0=1+alpha*A; b1=-2*c; b2=1-alpha*A; a0=1+alpha/A; a1=-2*c; a2=1-alpha/A; break;
        case Shape::LowShelf:
            b0=A*((A+1)-(A-1)*c+beta); b1=2*A*((A-1)-(A+1)*c); b2=A*((A+1)-(A-1)*c-beta);
            a0=(A+1)+(A-1)*c+beta; a1=-2*((A-1)+(A+1)*c); a2=(A+1)+(A-1)*c-beta; break;
        case Shape::HighShelf:
            b0=A*((A+1)+(A-1)*c+beta); b1=-2*A*((A-1)+(A+1)*c); b2=A*((A+1)+(A-1)*c-beta);
            a0=(A+1)-(A-1)*c+beta; a1=2*((A-1)-(A+1)*c); a2=(A+1)-(A-1)*c-beta; break;
        case Shape::LowCut: b0=(1+c)/2; b1=-(1+c); b2=b0; a0=1+alpha; a1=-2*c; a2=1-alpha; break;
        case Shape::HighCut: b0=(1-c)/2; b1=1-c; b2=b0; a0=1+alpha; a1=-2*c; a2=1-alpha; break;
        case Shape::Notch: b0=1; b1=-2*c; b2=1; a0=1+alpha; a1=-2*c; a2=1-alpha; break;
        case Shape::BandPass: b0=alpha; b1=0; b2=-alpha; a0=1+alpha; a1=-2*c; a2=1-alpha; break;
        case Shape::AllPass: b0=1-alpha; b1=-2*c; b2=1+alpha; a0=1+alpha; a1=-2*c; a2=1-alpha; break;
        default: break;
    }
    const DigitalSection fallback{b0/a0,b1/a0,b2/a0,a1/a0,a2/a0};
    return shape==Shape::Bell?decrampedLowPass(hz,q,sr,fallback,db,true):fallback;
}
}
SectionBank designStaticBand(BandSettings p, double sr) {
    SectionBank result;
    if (!p.enabled || !std::isfinite(sr) || sr <= 0) return result;
    if (!std::isfinite(p.frequency) || !std::isfinite(p.q) || !std::isfinite(p.gain)) return result;
    p.frequency = std::clamp(p.frequency, 10.0, sr*0.475);
    p.q = std::clamp(p.q, 0.1, 20.0);
    p.gain = std::clamp(p.gain, -30.0, 30.0);
    const double pi = std::acos(-1.0);
    if (p.shape == Shape::LowCut || p.shape == Shape::HighCut) {
        constexpr int orders[]{1,2,4,6,8,12,16};
        const int order = orders[std::clamp(p.slope,0,6)];
        result.count = (order+1)/2;
        if (order == 1) {
            const double k=std::tan(pi*p.frequency/sr), inv=1/(1+k);
            result.sections[0] = p.shape == Shape::HighCut
                ? DigitalSection{k*inv,k*inv,0,(k-1)*inv,0}
                : DigitalSection{inv,-inv,0,(k-1)*inv,0};
        } else {
            for (int i=0; i<result.count; ++i) {
                const double stageQ=1/(2*std::sin((2*i+1)*pi/(2*order)));
                // For the measured 12 dB/oct shape, Q=.707 is critically
                // damped (-6 dB at the corner), rather than Butterworth.
                const double cornerScale = order == 2 ? std::sqrt(0.5) : 1.0;
                const double q=std::clamp(stageQ*(p.q/0.70710678)*cornerScale,0.1,40.0);
                result.sections[size_t(i)]=biquad(p.shape,p.frequency,0,q,sr);
                if(order==2 && p.shape==Shape::HighCut)
                    result.sections[size_t(i)]=decrampedLowPass(p.frequency,q,sr,result.sections[size_t(i)]);
            }
        }
    } else if (p.shape == Shape::TiltShelf || p.shape == Shape::FlatTilt) {
        result.count=2;
        // Own v1 tilt definitions: opposite shelves; Flat Tilt has a wide span.
        const double low=p.shape == Shape::FlatTilt ? p.frequency/4 : p.frequency;
        const double high=p.shape == Shape::FlatTilt ? p.frequency*4 : p.frequency;
        result.sections[0]=biquad(Shape::LowShelf,std::clamp(low,10.0,sr*0.475),-p.gain/2,p.q,sr);
        result.sections[1]=biquad(Shape::HighShelf,std::clamp(high,10.0,sr*0.475),p.gain/2,p.q,sr);
    } else result.sections[0]=biquad(p.shape,p.frequency,p.gain,p.q,sr);
    return result;
}
}
