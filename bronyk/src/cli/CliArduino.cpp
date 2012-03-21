#include "XLog.h"
#include "XCmdShell.h"
#include "../ArduinoCtrl.h"

#include <SDL/SDL.h>
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"

using namespace Arduino;

namespace CliArduino {

	void SDL_Print(SDL_Surface *dst, Sint16 x, Sint16 y, const char* text, TTF_Font* font, const SDL_Color& col) {
		if (SDL_Surface *message = TTF_RenderText_Blended(font, text, col)) {
			SDL_Rect offset = { x, y };
			SDL_BlitSurface(message, NULL, dst, &offset);
			SDL_FreeSurface(message);
		}
	}

	static bool keyState[SDLK_LAST] = {};

	static int Drive(int argc, char *argv[])
	{
		XSerialTty serial;
		ControllerMgr ctrl(&serial);

		ServoMotor smSteer(&ctrl, 1);
		DcMotor dcmWheels(&ctrl, 2);
		DigitalPin dpBoost(&ctrl, 3);

		LOG(NULL, "Connecting Arduino");
		if (!serial.Open("/dev/ttyUSB0", 115200)) return 1;
		ctrl.Start();
		ctrl.Connect();

	    if (SDL_Init(SDL_INIT_EVERYTHING) >= 0) {
		    if (SDL_Surface* screen = SDL_SetVideoMode(160, 120, 32, SDL_HWSURFACE)) {
		    	if( TTF_Init() == -1 ) { LOG_WARN(NULL, "TTF not initialised"); }



				SDL_FillRect( SDL_GetVideoSurface(), NULL, 0 );
		    	SDL_Color textColor1 = { 255, 0, 0 };
		    	SDL_Color textColor2 = { 255, 255, 255 };

		    	if (TTF_Font *font = TTF_OpenFont("font.ttf", 30)) {
			    	TTF_SetFontStyle(font, TTF_STYLE_ITALIC);

			    	SDL_Print(screen, 30, 10, "W", font, keyState[SDLK_w]?textColor1:textColor2);
			    	SDL_Print(screen, 10, 40, "A", font, keyState[SDLK_a]?textColor1:textColor2);
			    	SDL_Print(screen, 30, 40, "S", font, keyState[SDLK_s]?textColor1:textColor2);
			    	SDL_Print(screen, 50, 40, "D", font, keyState[SDLK_d]?textColor1:textColor2);
			    	SDL_Print(screen, 100, 40, "B", font, keyState[SDLK_b]?textColor1:textColor2);
			    	SDL_Print(screen, 10, 80, "space", font, keyState[SDLK_SPACE]?textColor1:textColor2);
			    	TTF_CloseFont(font);
		    	}


				SDL_Flip(screen);

		    	SDL_WM_SetCaption("RC", NULL);
				SDL_Event event;
				while(SDL_WaitEvent(&event)) {
					if (!serial.Opened())
						goto stop_drive;

					switch (event.type){
					case SDL_QUIT:
						goto stop_drive;
					case SDL_KEYDOWN: {
						SDLKey key = event.key.keysym.sym;
						keyState[key] = true;

						switch (key) {
						case SDLK_a: smSteer.SetPosition(90-40); break;
						case SDLK_d: smSteer.SetPosition(90+40); break;
						case SDLK_w: dcmWheels.Configure(DcMotor::DIR_FW, 250); break;
						case SDLK_s: dcmWheels.Configure(DcMotor::DIR_BW, 250); break;
						case SDLK_SPACE: dcmWheels.Configure(DcMotor::DIR_STOP, 250); break;
						case SDLK_b: dpBoost.SetBoolValue(true); break;
						case SDLK_q: goto stop_drive;
						default: break;
						}
						break;
					}
					case SDL_KEYUP: {
						SDLKey key = event.key.keysym.sym;
						keyState[key] = false;

						switch (key) {
						case SDLK_a:
						case SDLK_d:
							if (!keyState[SDLK_a] && !keyState[SDLK_d]) {
								smSteer.SetPosition(90);
							}
							break;
						case SDLK_w:
						case SDLK_s:
						case SDLK_SPACE:
							if (!keyState[SDLK_s] && !keyState[SDLK_w] &&! keyState[SDLK_SPACE]) {
								dcmWheels.Configure(DcMotor::DIR_STOP, 0);
							}
							break;
						case SDLK_b:
							dpBoost.SetBoolValue(false);
							break;
						case SDLK_q:
							goto stop_drive;
						default:
							break;
						}
						break;
					}
					default:
						break;
					}

				}
stop_drive:
				SDL_FreeSurface(screen);
		    }
		    SDL_Quit();
	    }

	    LOG(NULL, "Disconnecting Arduino");
		ctrl.Stop();
		serial.Close();

		return 0;
	}

	void SDL_DrawRect(SDL_Surface *dst, int x, int y, int w, int h, int color1) {
		SDL_Rect rect = {Uint16(x), Uint16(y), Uint16(w), Uint16(h)};
		SDL_FillRect(dst, &rect, color1);
	}

	void SDL_DrawRect2(SDL_Surface *dst, int x, int y, int w, int h, int color1, int color2) {
		SDL_Rect rect = {Uint16(x), Uint16(y), Uint16(w), Uint16(h)};
		SDL_Rect rect2 = {Uint16(x+1), Uint16(y+1), Uint16(w-2), Uint16(h-2) };
		SDL_FillRect(dst, &rect, color1);
		SDL_FillRect(dst, &rect2, color2);
	}

	static int JoyMouse(int argc, char *argv[])
	{
		XSerialTty serial;
		ControllerMgr ctrl(&serial);

		ServoMotor smSteer(&ctrl, 1);
		DcMotor dcmWheels(&ctrl, 2);
		DigitalPin dpBoost(&ctrl, 3);

		LOG(NULL, "Connecting Arduino");
		if (!serial.Open("/dev/ttyUSB0", 115200)) return 1;
		ctrl.Start();
		ctrl.Connect();

	    if (SDL_Init(SDL_INIT_EVERYTHING) >= 0) {
		    if (SDL_Surface* screen = SDL_SetVideoMode(512, 512, 32, SDL_HWSURFACE)) {
		    	int deadZone = 24;

		    	SDL_DrawRect2(screen, 0, 0, 512, 512, 0, 0x888888);
		    	SDL_DrawRect2(screen, 0, 0, 256-deadZone, 256-deadZone, 0, 0xFFFFFF);
		    	SDL_DrawRect2(screen, 0, 256+deadZone, 256-deadZone, 256-deadZone, 0, 0xFFFFFF);
		    	SDL_DrawRect2(screen, 256+deadZone, 0, 256-deadZone, 256-deadZone, 0, 0xFFFFFF);
		    	SDL_DrawRect2(screen, 256+deadZone, 256+deadZone, 256-deadZone, 256-deadZone, 0, 0xFFFFFF);

				SDL_Flip(screen);

		    	SDL_WM_SetCaption("RC - Joystick", NULL);
				SDL_Event event;
				while(SDL_WaitEvent(&event)) {
					if (!serial.Opened())
						goto stop_drive;

					switch (event.type){
					case SDL_QUIT:
						goto stop_drive;
					case SDL_MOUSEMOTION: {
						if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
							int x = event.motion.x-256;
							int y = event.motion.y-256;
							if (x > deadZone) {
								x = LinearMap(x, deadZone+1, 256, 0, 256);
							} else if (x < -deadZone) {
								x = LinearMap(x, -deadZone-1, -256, 0, -256);
							} else {
								x = 0;
							}
							if (y > deadZone) {
								y = LinearMap(y, deadZone+1, 256, 0, -256);
							} else if (y < -deadZone) {
								y = LinearMap(y, -deadZone-1, -256, 0, 256);
							} else {
								y = 0;
							}
							int steer_left = 90-45;
							int steer_right = 90+45;
							int steer_center = 90;

							int steer = LinearMap(x, -255, 255, steer_left, steer_right);
							int motor = LinearMap(y, -255, 255, -255, 255);

							static int steerPrev = -1024;
							static int motorPrev = -1024;
							if(steer_left <= steer && steer <= steer_right && steer != steerPrev) {
								smSteer.SetPosition(steer);
								steerPrev = steer;
							}

							if(-256 < motor && motor < 256 && motor != motorPrev) {
								if (motor < 0) {

									dcmWheels.Configure(DcMotor::DIR_BW, -motor);
								} else if (motor > 0) {
									dcmWheels.Configure(DcMotor::DIR_FW, motor);
								} else {
									dcmWheels.Configure(DcMotor::DIR_STOP, 250);
								}
								motorPrev = motor;
							}
						}

						break;
					}
					case SDL_MOUSEBUTTONDOWN: {
						SDL_WM_GrabInput(SDL_GRAB_ON);

						break;
					}
					case SDL_MOUSEBUTTONUP: {
						dcmWheels.Configure(DcMotor::DIR_STOP, 250);
						SDL_WM_GrabInput(SDL_GRAB_OFF);

						break;
					}
					default:
						break;
					}
				}
stop_drive:
				SDL_FreeSurface(screen);
		    }
		    SDL_Quit();
	    }

	    LOG(NULL, "Disconnecting Arduino");
		ctrl.Stop();
		serial.Close();

		return 0;
	}
/*
	static int HallSensorValues;
	static void HallNtfHandler(uint16_t value) {
		printf("Hall Sensor value: %d\n", value);
		HallSensorValues++;
	}

	static int TestHall(int argc, char *argv[]) {
		printf("Testing Hall Sensor\n");

		HallSensorValues = 0;
		hallPin.SetNotification(0, 1024, AnalogPin::REPORT_IN_CONT, &HallNtfHandler);

		Thread::SleepMs(1000);

		printf("Got %d Hall Sensor values\n", HallSensorValues);
		hallPin.SetNotification(0, 10, AnalogPin::REPORT_IN_ONCE, &HallNtfHandler);

		return 0;
	}


	static void HallNtfHandler2(uint16_t value) {
		printf("Hall Sensor value: %d\n", value);
		DcMotor(&ctrl, 7).Configure(DcMotor::DIR_FW, value/4);
		hallPin.SetNotification(value-10, value+10, AnalogPin::REPORT_OUT_ONCE, &HallNtfHandler2);
	}

	static int TestHallDc(int argc, char *argv[]) {
		printf("Testing Hall Sensor\n");

		HallSensorValues = 0;
		hallPin.SetNotification(0, 1024, AnalogPin::REPORT_IN_CONT, &HallNtfHandler2);

		Thread::SleepMs(30000);

		printf("Got %d Hall Sensor values\n", HallSensorValues);
		hallPin.SetNotification(0, 10, AnalogPin::REPORT_NONE, NULL);

		return 0;
	}

*/
	void Init(XShell& shell) {
		shell.RegisterCommand("drive", XShell::Handler(&Drive));
		shell.RegisterCommand("jm", XShell::Handler(&JoyMouse));
	}

}
