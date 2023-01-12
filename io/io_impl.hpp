#ifndef _IO_IMPL_HPP_
#define _IO_IMPL_HPP_

#include "lr.hpp"

/*
Message                 Status      Data 1              Data 2
Note Off                8n          Note Number         Velocity
Note On                 9n          Note Number         Velocity
Polyphonic Aftertouch   An          Note Number         Pressure
Control Change          Bn          Controller Number   Data
Program Change          Cn          Program Number      Unused
Channel Aftertouch      Dn          Pressure            Unused
Pitch Wheel             En          LSB                 MSB

Key
n is the MIDI Channel Number (0-F)
LSB is the Least Significant Byte, 7bits
MSB is the Least Significant Byte, 7bits
There are several different types of controller messages.

Control Change (Bn) :
Controller Number           Data 2
01 (Modulation Wheel)       Data
02 (Breath Controller)      Data
04 (Foot Contoller)         Data
05 (Portamento Time)        Data
06 (Data Entry Slider)      Data
07 (Main Volume)            Data
*/

namespace lr { namespace io {

union MidiMessage {
    enum {
        Unknow = 0,
        NoteOff = 8,
        NoteOn = 9,
        PolyAfterTouch = 10,
        ControlChange = 11,
        ProgramChange = 12,
        ChanAfterTouch = 13,
        PitchWhell = 14,

        ModulationWheel = 1,
        BreathControler = 2,
    };
    float value_;
    uint8_t type_;
    struct {
        uint8_t     type_;
        uint8_t     channel_;
        uint8_t     d1_;
        uint8_t     d2_;
    }dd;
    struct {
        uint8_t     type_;
        uint8_t     channel_;
        uint16_t    d_;
    }d;

    static MidiMessage null() {
        MidiMessage msg;
        msg.d.d_ = 0;
        msg.type_ = Unknow;
        return msg;
    }
    static MidiMessage parse(unsigned char* data, size_t size) {
        MidiMessage msg;
        msg.d.d_ = 0;
        msg.type_ = Unknow;
        if ( size != 3 ) {
            return msg;
        }
        msg.type_ = (data[0] & 0xF0) >> 4;
        msg.d.channel_ = data[0] & 0x0F;
        switch( msg.type_ ) {
            case NoteOn:
            case NoteOff:
            case ControlChange:
                msg.dd.d1_ = data[1];
                msg.dd.d2_ = data[2];
                break;
            case PitchWhell:
                msg.d.d_ = (data[2] << 7) + data[1];
                break;
            default:
                msg.type_ = Unknow;
        }
        return msg;
    }
};

void init_words(Enviroment& env);

}}
#endif
