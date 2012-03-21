#include "Mouse.h"
#include <stdint.h>

// Documentation: http://www.computer-engineering.org/ps2mouse/
// http://www.win.tue.nl/~aeb/linux/kbd/scancodes-13.html

struct MouseData {
	int deltaX, deltaY, deltaZ;
	bool overflowX, overflowY;
	bool btnL, btnM, btnR, btn4, btn5;
	bool signX, signY;

	void ReadPs2(FILE* fmouse) {
		char input[3];
		fread(input, 3, 1, fmouse);
		btnL = (input[0]&1)>0;
		btnR = (input[0]&2)>0;
		btnM = (input[0]&4)>0;
		signX = (input[0]&16)>0;
		signY = (input[0]&32)>0;
		overflowX = (input[0]&64)>0;
		overflowY = (input[0]&128)>0;
		deltaX = input[1];
		deltaY = input[2];
	}

	void ReadIm(FILE* fmouse) {
		char input[4];
		fread(input, 4, 1, fmouse);
		btnL = (input[0]&1)>0;
		btnR = (input[0]&2)>0;
		btnM = (input[0]&4)>0;
		signX = (input[0]&16)>0;
		signY = (input[0]&32)>0;
		overflowX = (input[0]&64)>0;
		overflowY = (input[0]&128)>0;
		deltaX = input[1];
		deltaY = input[2];
		deltaZ = input[3];
	}

	void ReadImEx(FILE* fmouse) {
		char input[4];
		fread(input, 4, 1, fmouse);
		btnL = (input[0]&1)>0;
		btnR = (input[0]&2)>0;
		btnM = (input[0]&4)>0;
		signX = (input[0]&16)>0;
		signY = (input[0]&32)>0;
		overflowX = (input[0]&64)>0;
		overflowY = (input[0]&128)>0;
		deltaX = input[1];
		deltaY = input[2];
		deltaZ = input[3]&0xF;
		btn4 = (input[3]&16)>0;
		btn5 = (input[3]&32)>0;
	}
};

enum MouseResolution {
	MouseRes1upmm = 0,
	MouseRes2upmm = 1,
	MouseRes4upmm = 2,
	MouseRes8upmm = 3
};

enum MouseType {
	MousePS2			= 0x00,
	MouseBallPoint		= 0x02,
	MouseIntellimouse	= 0x03,
	MouseExplorer		= 0x04,
	Mouse4d				= 0x06,
	Mouse4dPlus			= 0x08,
	MouseTyphoon		= 0x08
};

void Enable(FILE* fmouse) {
	fputc(0xf4, fmouse);		// command
	fgetc(fmouse);				// ack
}

void Disable(FILE* fmouse) {
	fputc(0xf5, fmouse);		// command
	fgetc(fmouse);				// ack
}

void SetDefaults(FILE* fmouse) {
	fputc(0xf6, fmouse);		// command
	fgetc(fmouse);				// ack
}

void Reset(FILE* fmouse) {
	fputc(0xff, fmouse);		// command
	fgetc(fmouse);				// ack
	fgetc(fmouse);				// status
	fgetc(fmouse);				// status
}

void SetScaleing1to1(FILE* fmouse) {
	fputc(0xe6, fmouse);		// command
	fgetc(fmouse);				// ack
}

void SetScaleing2to1(FILE* fmouse) {
	fputc(0xe7, fmouse);		// command
	fgetc(fmouse);				// ack
}

void SetResolution(FILE* fmouse, int id) {
	fputc(0xe8, fmouse);		// command
	fgetc(fmouse);				// ack
	fputc(id, fmouse);			// data
	fgetc(fmouse);				// ack
}

void SetStreamMode(FILE* fmouse) {
	fputc(0xea, fmouse);		// command
	fgetc(fmouse);				// ack
}

void ReadData(FILE* fmouse) {
	fputc(0xeb, fmouse);		// command
	fgetc(fmouse);				// ack
}

void SetEchoMode(FILE* fmouse) {
	fputc(0xee, fmouse);		// command
	fgetc(fmouse);				// ack
}

void ClearEchoMode(FILE* fmouse) {
	fputc(0xee, fmouse);		// command
	fgetc(fmouse);				// ack
}

void SetRemoteMode(FILE* fmouse) {
	fputc(0xf0, fmouse);		// command
	fgetc(fmouse);				// ack
}

void SetSampleRate(FILE* fmouse, int rate) {
	fputc(0xf3, fmouse);		// command
	fgetc(fmouse);				// ack
	fputc(rate, fmouse);		// data
	fgetc(fmouse);				// ack
}

uint8_t ReadMouseID(FILE* fmouse) {
	fputc(0xf2, fmouse);			// command
	fgetc(fmouse);					// ack
	return fgetc(fmouse) & 0xFF;	// device id
}

bool InitModeIm(FILE* fmouse) {
	SetSampleRate(fmouse, 0xc8);
	SetSampleRate(fmouse, 0x64);
	SetSampleRate(fmouse, 0x50);
	return ReadMouseID(fmouse) == 0x03;
}

bool InitModeImEx(FILE* fmouse) {
	SetSampleRate(fmouse, 0xc8);
	SetSampleRate(fmouse, 0xc8);
	SetSampleRate(fmouse, 0x50);
	return ReadMouseID(fmouse) == 0x04;
}

bool InitModeTyphoon(FILE* fmouse) {
	SetSampleRate(fmouse, 0xc8);
	SetSampleRate(fmouse, 0x64);
	SetSampleRate(fmouse, 0x50);
	SetSampleRate(fmouse, 0x3c);
	SetSampleRate(fmouse, 0x28);
	SetSampleRate(fmouse, 0x14);
	return ReadMouseID(fmouse) == 0x08;
}

int MousePs2::Run() {
	if(FILE* fmouse = fopen(mDev, "wb+")) {

		Disable(fmouse);

		InitModeIm(fmouse);

		Enable(fmouse);

		MouseData data;
		while(!IsStopping()) {
			// Read data
			data.ReadIm(fmouse);

			// Process data
			x += data.deltaX;
			y += data.deltaY;
			z += data.deltaZ;

			LOG(mLog, "X: " << x << " Y: " << y << " Z: " << z);
			LOG(mLog, "L: " << data.btnL << " M: " << data.btnL << " R: " << data.btnR);
			LOG(mLog, "4: " << data.btn4 << " 5: " << data.btn5);
			//LOG(mLog, "sX: " << data.signX << " sY: " << data.signY);
			//LOG(mLog, "oX: " << data.overflowX << " oY: " << data.overflowY);
		}
		fclose(fmouse);
	}
	return 0;
}

