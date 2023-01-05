/* ------------------------------------------------------------
name: "square"
Code generated with Faust 2.54.8 (https://faust.grame.fr)
Compilation options: -lang cpp -light -cn OscSquare -es 1 -mcd 16 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __OscSquare_H__
#define  __OscSquare_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

namespace dsp {

#ifndef FAUSTCLASS 
#define FAUSTCLASS OscSquare
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

static float OscSquare_faustpower2_f(float value) {
	return value * value;
}

class OscSquare : public dsp {
	
 private:
	
	int iVec0[2];
	FAUSTFLOAT fEntry0;
	int fSampleRate;
	float fConst1;
	float fRec0[2];
	float fVec1[2];
	float fConst2;
	int IOTA0;
	float fVec2[4096];
	float fConst3;
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("compile_options", "-lang cpp -light -cn OscSquare -es 1 -mcd 16 -single -ftz 0");
		m->declare("filename", "square.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.5");
		m->declare("name", "square");
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
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = FAUSTFLOAT(4.4e+02f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec0[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fVec1[l2] = 0.0f;
		}
		IOTA0 = 0;
		for (int l3 = 0; l3 < 4096; l3 = l3 + 1) {
			fVec2[l3] = 0.0f;
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
	
	virtual OscSquare* clone() {
		return new OscSquare();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("square");
		ui_interface->addNumEntry("freq", &fEntry0, FAUSTFLOAT(4.4e+02f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(0.1f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		float fSlow0 = std::max<float>(float(fEntry0), 23.44895f);
		float fSlow1 = std::max<float>(2e+01f, std::fabs(fSlow0));
		float fSlow2 = fConst1 * fSlow1;
		float fSlow3 = fConst2 / fSlow1;
		float fSlow4 = std::max<float>(0.0f, std::min<float>(2047.0f, fConst3 / fSlow0));
		int iSlow5 = int(fSlow4);
		int iSlow6 = iSlow5 + 1;
		float fSlow7 = std::floor(fSlow4);
		float fSlow8 = fSlow4 - fSlow7;
		float fSlow9 = fSlow7 + (1.0f - fSlow4);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec0[0] = 1;
			fRec0[0] = fSlow2 + (fRec0[1] - std::floor(fSlow2 + fRec0[1]));
			float fTemp0 = OscSquare_faustpower2_f(2.0f * fRec0[0] + -1.0f);
			fVec1[0] = fTemp0;
			float fTemp1 = fSlow3 * float(iVec0[1]) * (fTemp0 - fVec1[1]);
			fVec2[IOTA0 & 4095] = fTemp1;
			output0[i0] = FAUSTFLOAT(fTemp1 - (fSlow9 * fVec2[(IOTA0 - iSlow5) & 4095] + fSlow8 * fVec2[(IOTA0 - iSlow6) & 4095]));
			iVec0[1] = iVec0[0];
			fRec0[1] = fRec0[0];
			fVec1[1] = fVec1[0];
			IOTA0 = IOTA0 + 1;
		}
	}

};

} // namespace dsp

#endif
