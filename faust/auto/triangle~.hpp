/* ------------------------------------------------------------
name: "triangle"
Code generated with Faust 2.54.8 (https://faust.grame.fr)
Compilation options: -lang cpp -light -cn OscTriangle -es 1 -mcd 16 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __OscTriangle_H__
#define  __OscTriangle_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

namespace dsp {

#ifndef FAUSTCLASS 
#define FAUSTCLASS OscTriangle
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

static float OscTriangle_faustpower2_f(float value) {
	return value * value;
}

class OscTriangle : public dsp {
	
 private:
	
	int iVec0[2];
	FAUSTFLOAT fEntry0;
	int fSampleRate;
	float fConst1;
	float fRec1[2];
	float fVec1[2];
	float fConst2;
	int IOTA0;
	float fVec2[4096];
	float fConst3;
	float fRec0[2];
	float fConst4;
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("compile_options", "-lang cpp -light -cn OscTriangle -es 1 -mcd 16 -single -ftz 0");
		m->declare("filename", "triangle.dsp");
		m->declare("filters.lib/lowpass0_highpass1", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/pole:author", "Julius O. Smith III");
		m->declare("filters.lib/pole:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/pole:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "0.3");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.5");
		m->declare("name", "triangle");
		m->declare("oscillators.lib/lf_sawpos:author", "Bart Brouns, revised by Stéphane Letz");
		m->declare("oscillators.lib/lf_sawpos:licence", "STK-4.3");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/version", "0.3");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "0.2");
	}

	virtual int getNumInputs() {
		return 0;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		float fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, float(fSampleRate)));
		fConst1 = 1.0f / fConst0;
		fConst2 = 0.25f * fConst0;
		fConst3 = 0.5f * fConst0;
		fConst4 = 4.0f / fConst0;
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = FAUSTFLOAT(4.4e+02f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec1[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fVec1[l2] = 0.0f;
		}
		IOTA0 = 0;
		for (int l3 = 0; l3 < 4096; l3 = l3 + 1) {
			fVec2[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec0[l4] = 0.0f;
		}
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	virtual void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual OscTriangle* clone() {
		return new OscTriangle();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("triangle");
		ui_interface->addNumEntry("freq", &fEntry0, FAUSTFLOAT(4.4e+02f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(0.1f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		float fSlow0 = float(fEntry0);
		float fSlow1 = std::max<float>(fSlow0, 23.44895f);
		float fSlow2 = std::max<float>(2e+01f, std::fabs(fSlow1));
		float fSlow3 = fConst1 * fSlow2;
		float fSlow4 = fConst2 / fSlow2;
		float fSlow5 = std::max<float>(0.0f, std::min<float>(2047.0f, fConst3 / fSlow1));
		int iSlow6 = int(fSlow5);
		int iSlow7 = iSlow6 + 1;
		float fSlow8 = std::floor(fSlow5);
		float fSlow9 = fSlow5 - fSlow8;
		float fSlow10 = fSlow8 + (1.0f - fSlow5);
		float fSlow11 = fConst4 * fSlow0;
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec0[0] = 1;
			fRec1[0] = fSlow3 + (fRec1[1] - std::floor(fSlow3 + fRec1[1]));
			float fTemp0 = OscTriangle_faustpower2_f(2.0f * fRec1[0] + -1.0f);
			fVec1[0] = fTemp0;
			float fTemp1 = fSlow4 * float(iVec0[1]) * (fTemp0 - fVec1[1]);
			fVec2[IOTA0 & 4095] = fTemp1;
			fRec0[0] = 0.999f * fRec0[1] + fTemp1 - (fSlow10 * fVec2[(IOTA0 - iSlow6) & 4095] + fSlow9 * fVec2[(IOTA0 - iSlow7) & 4095]);
			output0[i0] = FAUSTFLOAT(fSlow11 * fRec0[0]);
			iVec0[1] = iVec0[0];
			fRec1[1] = fRec1[0];
			fVec1[1] = fVec1[0];
			IOTA0 = IOTA0 + 1;
			fRec0[1] = fRec0[0];
		}
	}

};

} // namespace dsp

#endif
