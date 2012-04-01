#include "arduino/core/main.h"
#include "arduino/core/Servo.h"

class IController {

public:
	virtual ~IController() {};
	virtual void Exec() {}
	virtual void EmergencyStop() = 0;

	virtual void Process(uint8_t type, const void* data, unsigned len) {}

	void Send(uint8_t type, void* data, unsigned len) {
		struct MsgHdr {
			uint8_t ctrl;
			uint8_t type;
			uint8_t len;
		} msg = { mCtrlId, type, len };

		Serial.write((uint8_t*)&msg, sizeof(msg));
		if (data && len) {
			Serial.write((uint8_t*)data, len);
		}
	}

public:
	uint8_t mCtrlId;
};

class DigitalPin : public IController
{
public:

	DigitalPin(uint8_t pin)
		: mPin(pin)
		, mExpecting(-1)
	{
		pinMode(mPin, OUTPUT);
	}

	void EmergencyStop() {
		digitalWrite(mPin, LOW);
	}

	void Process(uint8_t type, const void* data, unsigned len) {
		switch (type) {
			case SET_MODE_INPUT_CMD: {
				pinMode(mPin, INPUT);
				break;
			}
			case SET_MODE_OUTPUT_CMD: {
				pinMode(mPin, OUTPUT);
				break;
			}
			case SET_VALUE_HIGH_CMD: {
				digitalWrite(mPin, HIGH);
				break;
			}
			case SET_VALUE_LOW_CMD: {
				digitalWrite(mPin, LOW);
				break;
			}
			case SET_VALUE_PWM_CMD: {
				struct CmdStruct {
					uint8_t value;
				} const *cmd = (const CmdStruct*)data;
				analogWrite(mPin, cmd->value?INPUT:OUTPUT);
				break;
			}
			case GET_BOOL_VALUE_REQ: {
				if (digitalRead(mPin) == HIGH) {
					Send(GET_VALUE_HIGH_RSP, NULL, 0);
				} else {
					Send(GET_VALUE_LOW_RSP, NULL, 0);
				}
				break;
			}
			case SET_NTF_VALUE_CMD: {
				struct CmdStruct {
					uint8_t value;
				} const *cmd = (const CmdStruct*)data;
				mExpecting = cmd->value;
				break;
			}
			default: {
				break;
			}
		}
	}

	void Exec() {
		if (digitalRead(mPin) == mExpecting) {
			struct NtfStruct {
				uint8_t value;
			} ntf = { mExpecting };
			mExpecting = -1;
			Send(BOOL_VALUE_NTF, &ntf, sizeof(ntf));
		}
	}

private:
	enum CmdType {
		SET_MODE_INPUT_CMD,
		SET_MODE_OUTPUT_CMD,
		SET_VALUE_HIGH_CMD,
		SET_VALUE_LOW_CMD,
		SET_VALUE_PWM_CMD,
		GET_BOOL_VALUE_REQ,
		GET_VALUE_LOW_RSP,
		GET_VALUE_HIGH_RSP,
		SET_NTF_VALUE_CMD,
		BOOL_VALUE_NTF
	};

private:
	uint8_t mPin;
	uint8_t mExpecting;
};

class AnalogPin : public IController
{
public:

	AnalogPin(uint8_t pin)
		: mPin(pin)
		, mMin(0)
		, mMax(-1)
		, mReportMode(REPORT_NONE)
	{
	}

	void Process(uint8_t type, const void* data, unsigned len) {
		switch (type) {
			case GET_VALUE_REQ: {
				struct RspStruct {
					uint16_t value;
				} rsp = { analogRead(mPin) };
				Send(GET_VALUE_RSP, &rsp, sizeof(rsp));
				break;
			}
			case SET_NTF_VALUE_CMD: {
				struct CmdStruct {
					int16_t min;
					int16_t max;
					uint8_t mode;
				} const *cmd = (const CmdStruct*)data;
				mMin = cmd->min;
				mMax = cmd->max;
				mReportMode = cmd->mode;
				break;
			}
			default: {
				break;
			}
		}
	}

	void Exec() {
		int16_t val = analogRead(mPin);
		switch (mReportMode) {
		case REPORT_OUT_ONCE:
		case REPORT_OUT_CONT:
			if (val < mMin || val > mMax) {
				struct NtfStruct {
					uint16_t value;
					uint16_t border;
				} ntf = { val, (val < mMin) ? mMin : mMax };
				if (mReportMode == REPORT_OUT_ONCE) {
					mReportMode = REPORT_NONE;
				}
				Send(VALUE_OUT_NTF, &ntf, sizeof(ntf));
			}
			break;
		case REPORT_IN_ONCE:
		case REPORT_IN_CONT:
			if (val >= mMin && val <= mMax) {
				struct NtfStruct {
					uint16_t value;
				} ntf = { val };
				if (mReportMode == REPORT_IN_ONCE) {
					mReportMode = REPORT_NONE;
				}
				Send(VALUE_IN_NTF, &ntf, sizeof(ntf));
			}
			break;
		}
	}

private:
	enum CmdType {
		GET_VALUE_REQ,
		GET_VALUE_RSP,
		SET_NTF_VALUE_CMD,
		VALUE_IN_NTF,
		VALUE_OUT_NTF
	};

	enum ReportModes {
		REPORT_NONE,
		REPORT_IN_ONCE,
		REPORT_IN_CONT,
		REPORT_OUT_ONCE,
		REPORT_OUT_CONT
	};

private:
	uint8_t		mPin;
	int16_t		mMin;
	int16_t		mMax;
	uint8_t		mReportMode;
};

class ServoMotor : public IController
{
public:

	ServoMotor(uint8_t pin) {
		mServo.attach(pin);
		mServo.write(90);
	}

	virtual ~ServoMotor() {
		mServo.detach();
	}

	void EmergencyStop() {
		mServo.write(90);
	}

	void Process(uint8_t type, const void* data, unsigned len) {
		switch (type) {
			case SET_POSITION_CMD: {
				struct CmdStruct {
					uint8_t pos;
				} const *cmd = (const CmdStruct*)data;
				mServo.write(cmd->pos);

				break;
			}
			default: {
				break;
			}
		}
	}

private:
	enum CmdType {
		SET_POSITION_CMD
	};

private:
	Servo mServo;
};

class DcMotor : public IController
{
public:

	DcMotor(uint8_t en, uint8_t dirA, uint8_t dirB)
		: mEn(en), mDirA(dirA), mDirB(dirB)
	{
		pinMode(mEn, OUTPUT);
		pinMode(mDirA, OUTPUT);
		pinMode(mDirB, OUTPUT);
	}

	virtual ~DcMotor() {
	}

	void EmergencyStop() {
		digitalWrite(mDirA, LOW);
		digitalWrite(mDirB, LOW);
		analogWrite(mEn, 0);
	}

	void Process(uint8_t type, const void* data, unsigned len) {
		switch (type) {
			case CONFIGURE_CMD: {
				struct CmdStruct {
					int8_t dir;
					uint8_t speed;
				} const *cmd = (const CmdStruct*)data;

				if (cmd->dir < 0) {
					digitalWrite(mDirA, HIGH);
					digitalWrite(mDirB, LOW);
				} else if (cmd->dir > 0) {
					digitalWrite(mDirA, LOW);
					digitalWrite(mDirB, HIGH);
				} else {
					digitalWrite(mDirA, LOW);
					digitalWrite(mDirB, LOW);
				}

				analogWrite(mEn, cmd->speed);
				break;
			}
			case SET_SPEED_CMD: {
				struct CmdStruct {
					uint8_t speed;
				} const *cmd = (const CmdStruct*)data;
				analogWrite(mEn, cmd->speed);
				break;
			}
			default: {
				break;
			}
		}
	}

private:
	enum CmdType {
		CONFIGURE_CMD,
		SET_SPEED_CMD
	};

private:
	uint8_t mEn;
	uint8_t mDirA;
	uint8_t mDirB;
};


class HBridge : public IController
{
public:

	HBridge(uint8_t la, uint8_t lb, uint8_t ha, uint8_t hb)
		: mLA(la), mLB(lb), mHA(ha), mHB(hb)
	{
		pinMode(mLA, OUTPUT);
		pinMode(mLB, OUTPUT);
		pinMode(mHA, OUTPUT);
		pinMode(mHB, OUTPUT);
	}

	virtual ~HBridge() {
	}

	void EmergencyStop() {
		digitalWrite(mLA, LOW);
		digitalWrite(mLB, LOW);
		digitalWrite(mHA, LOW);
		digitalWrite(mHB, LOW);
	}

	void Process(uint8_t type, const void* data, unsigned len) {
		switch (type) {
			case CONFIGURE_CMD: {
				struct CmdStruct {
					int8_t dir;
					uint8_t speed;
				} const *cmd = (const CmdStruct*)data;

				//if (cmd->dir != mCurDir) {
					if (cmd->dir > 0) {
						digitalWrite(mLA, LOW);
						digitalWrite(mHB, LOW);
						delayMicroseconds(5);
						digitalWrite(mLB, HIGH);
						analogWrite(mHA, cmd->speed);
					} else if (cmd->dir < 0) {
						digitalWrite(mHA, LOW);
						digitalWrite(mLB, LOW);
						delayMicroseconds(5);
						digitalWrite(mLA, HIGH);
						analogWrite(mHB, cmd->speed);
					} else {
						digitalWrite(mLA, LOW);
						digitalWrite(mLB, LOW);
						delayMicroseconds(5);
						analogWrite(mHA, cmd->speed);
						analogWrite(mHB, cmd->speed);
					}
				//}

				/*mCurDir = cmd->dir;
				mCurSpeed = cmd->speed;
				mCurTime = 0;

				if (mCurSpeed == 0) {
					digitalWrite(mHA, LOW);
					digitalWrite(mHB, LOW);
					digitalWrite(mLA, LOW);
					digitalWrite(mLB, LOW);
				}*/
				break;
			}
			/*case SET_SPEED_CMD: {
				struct CmdStruct {
					uint8_t speed;
				} const *cmd = (const CmdStruct*)data;
				mCurSpeed = cmd->speed;
				mCurTime = 0;
				break;
			}*/
			default: {
				break;
			}
		}
	}

	/*void Exec() {
		if (mCurSpeed == 255) return;
		if (mCurSpeed == 0) return;
		if (mCurDir > 0) {
			if (mCurTime == mCurSpeed) {
				digitalWrite(mLB, LOW);
			} else if (mCurTime == 0) {
				digitalWrite(mLB, HIGH);
			}
		} else if (mCurDir < 0) {
			if (mCurTime == mCurSpeed) {
				digitalWrite(mLA, LOW);
			} else if (mCurTime == 0) {
				digitalWrite(mLA, HIGH);
			}
		} else {
			if (mCurTime == mCurSpeed) {
				digitalWrite(mLB, LOW);
			} else if (mCurTime == 0) {
				digitalWrite(mLB, HIGH);
			}
		}
		mCurTime = (mCurTime+1) % 255;
		//delay(5);
	}*/

private:
	enum CmdType {
		CONFIGURE_CMD,
		SET_SPEED_CMD
	};

private:
	uint8_t mLA;
	uint8_t mLB;
	uint8_t mHA;
	uint8_t mHB;
	//int8_t mCurDir;
	//int16_t mCurSpeed;
	//int16_t mCurTime;
};

class ControllerMgr {

	enum CtrlCommands {
		CTRL_STARTUP_REQ,
		CTRL_STARTUP_RSP,
		CTRL_HANGUP_REQ,
		CTRL_HANGUP_RSP
	};

public:
	static ControllerMgr& Get() { static ControllerMgr mgr; return mgr; }

	void Register(IController* ctrl) {
		for(int i=1; i<255; i++) {
			if(!mControllers[i]) {
				mControllers[i] = ctrl;
				ctrl->mCtrlId = i;
				break;
			}
		}
	}

	void Exec() {

		// Check for emergency shutdown

		if (digitalRead(53) == LOW) {
			// Execute controller emergency
			for(int i=1; i<255; i++) {
				if (mControllers[i]) {
					mControllers[i]->EmergencyStop();
				}
			}

			// Blink
			do {
				digitalWrite(13, HIGH);
				delay(100);
				digitalWrite(13, LOW);
				delay(900);
			} while (digitalRead(53) == LOW);

			// Flush serial buffer to start new session
			while (Serial.available()) {
				Serial.read();
			}
			Serial.flush();
		}

		for(int i=1; i<255; i++) {

			// Execute controller
			if (mControllers[i]) {
				mControllers[i]->Exec();
			}

			// Try executing command

			struct CmdHeader {
				uint8_t ctrl;
				uint8_t type;
				uint8_t length;
			};

			if (Serial.available() >= (int)sizeof(CmdHeader)) {
				CmdHeader hdr;
				Recv(&hdr, sizeof(CmdHeader));
				Recv(mCmdBuffer, hdr.length);

				if (hdr.ctrl != 0) {
					if (mConnected && mControllers[hdr.ctrl]) {
						mControllers[hdr.ctrl]->Process(hdr.type, mCmdBuffer, hdr.length);
					}
				} else {
					switch (hdr.type) {
						case CTRL_STARTUP_REQ: {
							mConnected = true;
							struct MsgHdr {
								uint8_t ctrl;
								uint8_t type;
								uint8_t len;
							} msg = { 0, CTRL_STARTUP_RSP, 0 };
							Serial.write((uint8_t*)&msg, sizeof(msg));
							break;
						}
						case CTRL_STARTUP_RSP: {
							mConnected = true;
							break;
						}
						case CTRL_HANGUP_REQ: {
							mConnected = false;
							struct MsgHdr {
								uint8_t ctrl;
								uint8_t type;
								uint8_t len;
							} msg = { 0, CTRL_HANGUP_RSP, 0 };
							Serial.write((uint8_t*)&msg, sizeof(msg));
							break;
						}
						case CTRL_HANGUP_RSP: {
							mConnected = false;
							break;
						}
						default: {
							break;
						}
					}
				}
			}
		}

	}
private:

	ControllerMgr()
		: mConnected(false)
	{
		memset(mControllers, 0, sizeof(mControllers));

		// Emergency pin
		pinMode(53, INPUT);
		digitalWrite(53, HIGH);

		struct MsgHdr {
			uint8_t ctrl;
			uint8_t type;
			uint8_t len;
		} msg = { 0, CTRL_STARTUP_REQ, 0 };
		Serial.write((uint8_t*)&msg, sizeof(msg));
	}

	void Send(const void* data, unsigned length) {
		if(!data || !length) return;
		const uint8_t* bytes = (const uint8_t*)data;
		for(unsigned i=0; i<length; i++) {
			Serial.write(bytes[i]);
		}
	}

	bool Recv(void* data, unsigned length) {
		if(!data || !length) return false;
		uint8_t* bytes = (uint8_t*)data;
		for(unsigned i=0; i<length; i++) {
			while(!Serial.available());
			bytes[i] = Serial.read() & 0xFF;
		}
		return true;
	}

private:
	bool			mConnected;
	uint8_t			mCmdBuffer[256];
	IController* 	mControllers[256];
};


void setup() {
	interrupts();
	pinMode(13, OUTPUT);
	Serial.begin(115200);
}

void loop() {
	ServoMotor smSteer(11);
	//DcMotor dcmWheels(9, 40, 41);
	DigitalPin dpBoost(42);
	HBridge hbrWheels(30, 32, 3, 4);

	ControllerMgr::Get().Register(&smSteer);
	ControllerMgr::Get().Register(&hbrWheels);
	ControllerMgr::Get().Register(&dpBoost);

	while(true) {

		// Indicate cycles
		{
			static int oddCycle = HIGH;
			digitalWrite(13, oddCycle = (oddCycle==LOW)?HIGH:LOW);
		}

		ControllerMgr::Get().Exec();
	}
}
