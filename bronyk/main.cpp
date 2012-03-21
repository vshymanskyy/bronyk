#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

//#include <opencv/cv.h>
//#include <opencv/highgui.h>

#include <XTimeCounter.h>
#include <XList.h>
#include <XSerialTty.h>

#include "src/ctrl/hw/Mouse.h"

#include "src/ctrl/virt/World.h"

#include <XCmdShell.h>

#include "src/cli/Cli.h"

/*
void ArduinoLogHandler(uint8_t type, const void* buffer, unsigned length) {
	static Log log("Arduino");
	static char buff[512];
	memcpy(buff, buffer, length);
	buff[length] = '\0';
	log() << buff;
}


class DataReceiver :  public Thread {
	ISerial* _serial;
	FILE* _f;

public:
	DataReceiver(ISerial* serial)
		: _serial (serial)
	{
		_f = fopen("/data/test2.bin", "wb+");
	}

	void Run() {
		int total = 0;
		char buff[512];
		while(!IsStopping()) {
			const ssize_t len = _serial->Read(&buff, 512);
			if (len > 0) {
				total += fwrite(buff, 1, len, _f);
				LogManager::Get().GetDefaultLog()() << "Got  " << len << " bytes";
			}
		}
		LogManager::Get().GetDefaultLog()() << "Total: " << total;
		fclose(_f);
	}
};

int testTransfer(int argc, char *argv[]) {
	printf("Testing transfer\n");

	SerialTty serial("/dev/ttyUSB0", 115200);
	//serial.flush_rx();

	DataReceiver serRecv(&serial);
	serRecv.Start();

	//Thread::SleepMs(50);

	if(FILE* f = fopen("/data/test.bin", "rb")) {
		char buff[1024];
		while(!feof(f)) {
			const int len = fread(buff, 1, 1024, f);
			if (len > 0) {
				serial.WriteExactly(buff, len);
				LogManager::Get().GetDefaultLog()() << "Sent " << len << " bytes";
			} else {
				break;
			}
		}
		fclose(f);
	}

	Thread::SleepMs(500);

	serial.DumpStats();

	serial.Close();
	serRecv.Stop();

	return 0;
}
*/

int testMouse(int argc, char *argv[]) {
	printf("Testing mouse\n");

	MousePs2 mouse0("/dev/input/mice");
	//MousePs2 mouse1("/dev/input/mouse2");
	mouse0.Start();
	//mouse1.Start();

	XThread::SleepMs(20000);
	mouse0.Stop();
	mouse0.Wait();
	//mouse1.Stop();
	//mouse1.Wait();

	return 0;
}


int main(int argc, char *argv[])
{
	XShell shell("bronyk");
	CliInit(shell);

	shell.RegisterCommand("testMouse", XShell::Handler(&testMouse));
	//shell.RegisterCommand("testTransfer", Shell::Handler(&testTransfer));


	shell.Run();

	exit(0);
	return 0;
}
