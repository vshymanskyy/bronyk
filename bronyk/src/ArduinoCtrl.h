#ifndef _ARDUINO_CTRL_H_
#define _ARDUINO_CTRL_H_

#include <XDelegate.h>
#include <XMemoryBuffer.h>
#include <XArray.h>
#include <XHelpers.h>
#include <XThread.h>
#include <XLogUtils.h>
#include <XSerialTty.h>

#include <stdlib.h>
#include <stdio.h>

namespace Arduino {

	class ControllerMgr;

	class IController {
	public:
		IController(ControllerMgr* mgr, uint8_t id);
		virtual ~IController();

		virtual void Process(uint8_t type, const void* data, unsigned len) = 0;

		void Send(uint8_t type);

		template <typename T>
		void Send(uint8_t type, const T& data);

		uint8_t GetId() { return mId; }
	private:
		ControllerMgr*	mMgr;
		uint8_t			mId;
	};

	class DigitalPin : public IController {
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

	public:
		typedef XDelegate<void(uint8_t value)> NtfHandler;

		DigitalPin(ControllerMgr *mgr, uint8_t id)
			: IController	(mgr, id)
			, mLastValue	(0)
			, mNtfHandler	(NULL)
		{

		}

		virtual ~DigitalPin() {}

		uint16_t GetCurrentValue() {
			Send(GET_BOOL_VALUE_REQ);
			// Todo: Wait for response
			return mLastValue;
		}

		void UpdateValue() {
			Send(GET_BOOL_VALUE_REQ);
		}

		uint8_t GetLastValue() {
			return mLastValue;
		}

		void SetModeInput() {
			Send(SET_MODE_INPUT_CMD);
		}

		void SetModeOutput() {
			Send(SET_MODE_OUTPUT_CMD);
		}

		void SetBoolValue(bool value) {
			Send(value?SET_VALUE_HIGH_CMD:SET_VALUE_LOW_CMD);
		}


		void Process(uint8_t type, const void* data, unsigned len) {
			switch (type) {
				case GET_VALUE_LOW_RSP: {
					mLastValue = 0;
					// Todo: somehow pass value
					break;
				}
				case GET_VALUE_HIGH_RSP: {
					mLastValue = 1;
					// Todo: somehow pass value
					break;
				}
				default: {
					break;
				}
			}
		}
	private:
		uint8_t		mLastValue;
		NtfHandler	mNtfHandler;
	};

	class AnalogPin : public IController {
	private:
		enum CmdType {
			GET_VALUE_REQ,
			GET_VALUE_RSP,
			SET_NTF_VALUE_CMD,
			VALUE_IN_NTF,
			VALUE_OUT_NTF
		};

	public:
		enum NtfMode {
			REPORT_NONE,
			REPORT_IN_ONCE,
			REPORT_IN_CONT,
			REPORT_OUT_ONCE,
			REPORT_OUT_CONT
		};

		typedef XDelegate<void(uint16_t value)> NtfHandler;

		AnalogPin(ControllerMgr *mgr, uint8_t id)
			: IController	(mgr, id)
			, mLastValue	(0)
			, mNtfHandler	(NULL)
		{

		}

		virtual ~AnalogPin() {}

		uint16_t GetCurrentValue() {
			Send(GET_VALUE_REQ);
			// Todo: Wait for response
			return mLastValue;
		}

		void UpdateValue() {
			Send(GET_VALUE_REQ);
		}

		uint16_t GetLastValue() {
			return mLastValue;
		}

		void SetNotification(int16_t min, int16_t max, NtfMode mode, NtfHandler h) {
			mNtfHandler = h;
			struct CmdStruct {
				int16_t min;
				int16_t max;
				uint8_t mode;
			} cmd = { min, max, (uint8_t)mode };
			Send(SET_NTF_VALUE_CMD, cmd);
		}

		void Process(uint8_t type, const void* data, unsigned len) {
			switch (type) {
				case GET_VALUE_RSP: {
					struct RspStruct {
						uint16_t value;
					} const *rsp = (const RspStruct*)data;
					mLastValue = rsp->value;
					// Todo: somehow pass value
					break;
				}
				case VALUE_IN_NTF: {
					struct NtfStruct {
						uint16_t value;
					} const *ntf = (const NtfStruct*)data;
					mLastValue = ntf->value;
					mNtfHandler(ntf->value);
					break;
				}
				case VALUE_OUT_NTF: {
					struct NtfStruct {
						uint16_t value;
						uint16_t border;
					} const *ntf = (const NtfStruct*)data;
					mLastValue = ntf->value;
					mNtfHandler(ntf->value);
					break;
				}
				default: {
					break;
				}
			}
		}
	private:
		uint16_t	mLastValue;
		NtfHandler	mNtfHandler;
	};

	class ServoMotor : public IController {
	private:
		enum CmdType {
			SET_POSITION_CMD
		};

	public:

		ServoMotor(ControllerMgr *mgr, uint8_t id)
			: IController	(mgr, id)
		{
		}

		virtual ~ServoMotor() {}

		void SetPosition(uint8_t pos) {
			struct CmdStruct {
				uint8_t pos;
			} cmd = { pos };
			Send(SET_POSITION_CMD, cmd);
		}

		void Process(uint8_t type, const void* data, unsigned len) {
		}
	};

	class DcMotor : public IController {
	private:
		enum CmdType {
			CONFIGURE_CMD,
			SET_SPEED_CMD
		};

	public:

		enum Direction {
			DIR_FW = 1,
			DIR_BW = -1,
			DIR_STOP = 0
		};

		DcMotor(ControllerMgr *mgr, uint8_t id)
			: IController	(mgr, id)
		{
		}

		virtual ~DcMotor() {}

		void SetSpeed(uint8_t speed) {
			struct CmdStruct {
				uint8_t speed;
			} cmd = { speed };
			Send(SET_SPEED_CMD, cmd);
		}

		void Configure(Direction dir, uint8_t speed) {
			struct CmdStruct {
				int8_t dir;
				uint8_t speed;
			} cmd = { uint8_t(dir), speed };
			Send(CONFIGURE_CMD, cmd);
		}


		void Process(uint8_t type, const void* data, unsigned len) {
		}
	};

	class ControllerMgr : public XThread {

		enum CtrlCommands {
			CTRL_STARTUP_REQ,
			CTRL_STARTUP_RSP,
			CTRL_HANGUP_REQ,
			CTRL_HANGUP_RSP
		};

	public:
		ControllerMgr(ISerial* serial)
			: mSerial		(serial)
			, mConnected	(false)
			, mLog			("ControllerMgr")//, LOG_DEEP)

		{
			memset(&mStats, 0x00, sizeof(Statistics));
			memset(&mControllers, 0x00, sizeof(mControllers));
		}

		virtual ~ControllerMgr() {
			Send(0, CTRL_HANGUP_REQ);
		}

		void Send(uint8_t device, uint8_t type) {
			SendCommand(device, type, NULL, 0);
		}

		template <typename T>
		void Send(uint8_t device, uint8_t type, const T& data) {
			SendCommand(device, type, &data, sizeof(T));
		}

		void Connect() {
			Send(0, CTRL_STARTUP_REQ);
		}

		void Register(IController* ctrl) {
			if (mControllers[ctrl->GetId()]) {
				LOG_WARN(mLog, "Could not register controller");
			} else {
				mControllers[ctrl->GetId()] = ctrl;
			}
		}

		void Unregister(IController* ctrl) {
			if (mControllers[ctrl->GetId()]) {
				mControllers[ctrl->GetId()] = NULL;
			} else {
				LOG_WARN(mLog, "Could not unregister controller");
			}
		}


		int Run() {
			struct MsgHeader {
				uint8_t ctrl;
				uint8_t type;
				uint8_t len;
			} hdr;
			XMemoryBuffer b(128);

			while(mSerial->Opened() && !IsStopping()) {
				if (mSerial->ReadExactly(&hdr, sizeof(MsgHeader)) != sizeof(MsgHeader)) {
					LOG_WARN(mLog, "Arduino disconnected");
					mSerial->Close();
					return 1;
				}
				b.Extend(hdr.len);
				if (mSerial->ReadExactly(b.Buffer(), hdr.len) != hdr.len) {
					LOG_WARN(mLog, "Arduino disconnected");
					mSerial->Close();
					return 1;
				}

				LOG_LOTS(mLog, "Received " << hdr.ctrl << ":" << hdr.type << " (" << hdr.len << "bytes)");
				LOG_DEEP(mLog, XLogDump(b.Buffer(), hdr.len));

				mStats.rxCommands++;

				if (hdr.ctrl) {
					if (mControllers[hdr.ctrl]) {
						mControllers[hdr.ctrl]->Process(hdr.type, b.Buffer(), hdr.len);
					} else {
						LOG_WARN(mLog, "Controller not available");
					}
				} else {
					switch (hdr.type) {
						case CTRL_STARTUP_REQ: {
							LOG(mLog, "CTRL_STARTUP_REQ");

							mConnected = true;
							Send(0, CTRL_STARTUP_RSP);
							break;
						}
						case CTRL_STARTUP_RSP: {
							mConnected = true;
							LOG(mLog, "CTRL_STARTUP_RSP");
							break;
						}
						case CTRL_HANGUP_REQ: {
							LOG(mLog, "CTRL_HANGUP_REQ");

							mConnected = false;
							Send(0, CTRL_HANGUP_RSP);
							break;
						}
						case CTRL_HANGUP_RSP: {
							mConnected = false;
							LOG(mLog, "CTRL_STARTUP_RSP");
							break;
						}

						default: {
							break;
						}
					}
				}

			}
			return 0;
		}

		void SendCommand(uint8_t device, uint8_t cmd, const void* data, uint8_t length) {
			//TODO: lock
			struct MsgHeader {
				uint8_t ctrl;
				uint8_t type;
				uint8_t length;
			} tl = { device, cmd, length };
			mSerial->Write(&tl, sizeof(MsgHeader));

			if (data && length) {
				mSerial->Write(data, length);
			}

			LOG_LOTS(mLog, "Sent " << device << ":" << cmd << " (" << length << "bytes)");
			LOG_DEEP(mLog, XLogDump(data, length));

			mStats.txCommands++;
		}

	private:
		struct Statistics {
			unsigned txCommands;
			unsigned rxCommands;
		};

		ISerial*		mSerial;
		IController*	mControllers[256];
		bool			mConnected;
		Statistics		mStats;
		XLog				mLog;
	};

	inline IController::IController(ControllerMgr* mgr, uint8_t id)
		: mMgr		(mgr)
		, mId		(id)
	{
		mMgr->Register(this);
	}

	inline IController::~IController() {
		mMgr->Unregister(this);
	}

	inline void IController::Send(uint8_t type) {
		mMgr->SendCommand(mId, type, NULL, 0);
	}

	template <typename T>
	inline void IController::Send(uint8_t type, const T& data) {
		mMgr->SendCommand(mId, type, &data, sizeof(T));
	}
}

#endif /* _ARDUINO_CTRL_H_ */
