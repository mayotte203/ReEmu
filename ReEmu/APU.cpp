#include "APU.h"
#include "common.h"
#include <SFML/Audio.hpp>
#include <SFML/Main.hpp>
//pulse frequency range ~ 54 - 12428 hz
//triangel frequency range ~ 27 - 6214 hz
const static int SAMPLE_RATE = 48000;
const static int PULSE_SAMPLECOUNT = 8;
const static int TRIANGLE_SAMPLECOUNT = 32;
const static s16 PULSE_WAVEFORM0[PULSE_SAMPLECOUNT] = { 0, 32767, 0, 0, 0, 0, 0, 0 };
const static s16 PULSE_WAVEFORM1[PULSE_SAMPLECOUNT] = { 0, 32767, 32767, 0, 0, 0, 0, 0 };
const static s16 PULSE_WAVEFORM2[PULSE_SAMPLECOUNT] = { 0, 32767, 32767, 32767, 32767, 0, 0, 0 };
const static s16 PULSE_WAVEFORM3[PULSE_SAMPLECOUNT] = { 32767, 0, 0, 32767, 32767, 32767, 32767, 32767 };
const static s16 TRIANGLE_WAVEFORM[TRIANGLE_SAMPLECOUNT] = { 15 * 2184, 14 * 2184, 13 * 2184, 12 * 2184, 11 * 2184, 10 * 2184, 9 * 2184, 8 * 2184,
															  7 * 2184, 6 * 2184, 5 * 2184, 4 * 2184, 3 * 2184, 2 * 2184, 1 * 2184, 0 * 2184,
															  0 * 2184, 1 * 2184, 2 * 2184, 3 * 2184, 4 * 2184, 5 * 2184, 6 * 2184, 7 * 2184,
															  8 * 2184, 9 * 2184, 10 * 2184, 11 * 2184, 12 * 2184, 13 * 2184, 14 * 2184, 15 * 2184 };
const static double CPU_CLOCK = 1789773.0;

namespace APU
{
	sf::SoundBuffer pulseChannelBuffer;
	sf::SoundBuffer triangleChannelBuffer;
	sf::SoundBuffer noiseChannelBuffer;
	sf::Sound pulseChannelSound;
	sf::Sound triangleChannelSound;
	sf::Sound noiseChannelSound;
	//Registers
	//Pulse 1 channel
	u8 pulse1Register0 = 0; //0x4000
	u8 pulse1Register1 = 0; //0x4001
	u8 pulse1Register2 = 0; //0x4002
	u8 pulse1Register3 = 0; //0x4003
	//Pulse 2 channel
	u8 pulse2Register0 = 0; //0x4004
	u8 pulse2Register1 = 0; //0x4005
	u8 pulse2Register2 = 0; //0x4006
	u8 pulse2Register3 = 0; //0x4007
	//Triangle channel
	u8 triangleRegister0 = 0; //0x4008
	u8 triangleRegister1 = 0; //0x400A
	u8 triangleRegister2 = 0; //0x400B
	//Noise channel
	u8 noiseRegister0 = 0; //0x400C
	u8 noiseRegister1 = 0; //0x400E
	u8 noiseRegister2 = 0; //0x400F
	//DMC channel
	u8 DMCRegister0 = 0; //0x4010
	u8 DMCRegister1 = 0; //0x4011
	u8 DMCRegister2 = 0; //0x4012
	u8 DMCRegister3 = 0; //0x4013
	//Other
	u8 controlRegister = 0; //0x4015 write
	u8 statusRegister = 0; //0x4015 read
	u8 frameCounterRegister = 0; //0x4017

	void init()
	{
		pulseChannelBuffer.loadFromSamples(PULSE_WAVEFORM2, PULSE_SAMPLECOUNT, 1, SAMPLE_RATE);
		pulseChannelSound.setBuffer(pulseChannelBuffer);
		pulseChannelSound.setLoop(true);
		pulseChannelSound.play();
		triangleChannelBuffer.loadFromSamples(TRIANGLE_WAVEFORM, PULSE_SAMPLECOUNT, 1, SAMPLE_RATE);
		triangleChannelSound.setBuffer(triangleChannelBuffer);
		triangleChannelSound.setLoop(true);
	}
	void writePulse1Register2(u8 data)
	{
		pulse1Register2 = data;
		int t = (pulse1Register3 & 0x7) * 256 + pulse1Register2;
		double f = CPU_CLOCK / (16 * (t + 1));
		pulseChannelSound.setPitch(f / 6000.0);
	}
	void writePulse1Register3(u8 data)
	{
		pulse1Register3 = data;
		int t = (pulse1Register3 & 0x7) * 256 + pulse1Register2;
		double f = CPU_CLOCK / (16 * (t + 1));
		pulseChannelSound.setPitch(f / 6000.0);
	}
}