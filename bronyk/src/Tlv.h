#ifndef TLV_H_
#define TLV_H_

#include "mstd/Delegate.h"
#include "mstd/MemoryBuffer.h"
#include "mstd/Array.h"
#include "mstd/Helpers.h"
#include "mstd/Thread.h"
#include "mstd/Log.h"
#include "mstd/SerialTty.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

class TlvParser {
public:
	typedef Delegate< void(uint8_t type, const void* buffer, unsigned length)> Handler;

	void SetHandler(int type, Handler handler) {
		assert(InRange(type, 0, 255));
		_handlers[type] = handler;
	}

protected:

	bool CallHandler(int type, const void* buffer, unsigned length) {
		if(!InRange(type, 0, 255)) {
			// Out of range
			return false;
		}
		if(!_handlers[type]) {
			// Unknown type
			return false;
		}
		_handlers[type](type, buffer, length);
		return true;
	}

private:
	Handler _handlers[256];
};

class SerialCmdDispatcher : public TlvParser, public Thread {
	ISerial* _serial;
	Log _log;

	struct Statistics {
		unsigned txCommands;
		unsigned rxCommands;
		unsigned txCmdCounters[TLV_TYPE_QTY];
		unsigned rxCmdCounters[TLV_TYPE_QTY];
	} _stats;

	void SendCommand(uint8_t type, const void* data, uint8_t length) {
		//TODO: lock
		TlvHeader tl = { type, length };
		_serial->Write(&tl, sizeof(TlvHeader));

		if (data && length) {
			_serial->Write(data, length);
		}

		_log(LOG_LOW) << "Sent TLV #" << type << "(" << length << "bytes)";
		_log(LOG_DEEP) << LogDump(data, length);

		_stats.txCommands++;
		_stats.txCmdCounters[type]++;
	}
public:
	SerialCmdDispatcher(ISerial* serial)
		: _serial (serial)
		, _log ("SerialCmdDispatcher")
	{
		memset(&_stats, 0x00, sizeof(Statistics));
	}

	virtual ~SerialCmdDispatcher() {
		Send(TLV_CMD_HANGUP);
	}

	void Send(uint8_t type) {
		SendCommand(type, NULL, 0);
	}

	template <typename T>
	void Send(uint8_t type, const T& data) {
		SendCommand(type, &data, sizeof(T));
	}

	void DumpStats() const {
		printf(" TX commands qty: %d\n", _stats.txCommands);
		printf(" RX commands qty: %d\n", _stats.rxCommands);

		static const char* TlvTypesStr[TLV_TYPE_QTY] = {
			"UNDEFINED",

			"NTF_ACK",
			"CMD_PING",
			"NTF_PONG",
			"CMD_HANGUP",
			"NTF_HANGUP",

			"NTF_STARTED",
			"NTF_LOG",

			"CMD_PIN_MODE",
			"CMD_DIGITAL_WRITE",
			"CMD_ANALOG_WRITE",
			//CMD_DIGITAL_READ_REQ,
			//CMD_DIGITAL_READ_REQ,
			//CMD_ANALOG_READ_REQ,
			//CMD_ANALOG_READ_RSP,

			"CMD_SET_GRIPPER",
			"CMD_LIFT_SET_LEVEL",
			"NTF_SENSOR"
			//CMD_INIT_SERIAL,
		};

		printf(" TX commands:\n");
		for(int i =1; i<TLV_TYPE_QTY; i++) {
			printf(" %25s : %d\n", TlvTypesStr[i], _stats.txCmdCounters[i]);
		}

		printf(" RX commands:\n");
		for(int i =1; i<TLV_TYPE_QTY; i++) {
			printf(" %25s : %d\n", TlvTypesStr[i], _stats.rxCmdCounters[i]);
		}
	}

	void Run() {
		TlvHeader tl;
		MemoryBuffer b(128);

		while(_serial->Opened() && !IsStopping()) {
			if(!_serial->ReadExactly(&tl, sizeof(TlvHeader))) break;
			b.Extend(tl.length);
			if(!_serial->ReadExactly(b.Buffer(), tl.length)) break;

			_log(LOG_LOW) << "Received TLV #" << tl.type << "(" << tl.length << "bytes)";
			_log(LOG_DEEP) << LogDump(b.Buffer(), tl.length);

			_stats.rxCommands++;
			_stats.rxCmdCounters[tl.type]++;

			//_log() << "Sent:" << _stats.txCommands << ", Received:" << _stats.rxCommands;

			if(tl.type == TLV_NTF_ACK) {
				// TODO: handle ack
				continue;
			}
			if(tl.type == TLV_NTF_STARTED) {
				// TODO: unlock
				continue;
			}
			if(tl.type == TLV_NTF_HANGUP) {
				_log() << "Hangup";
				break;
			}

			if(!CallHandler(tl.type, b.Buffer(), tl.length)) {
				_log(LOG_WARN) << "Unhandled TLV #" << tl.type;
			}
		}

		_serial->Close();
	}
};

#endif /* TLV_H_ */
