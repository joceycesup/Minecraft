#include "input_handler.h"

#include <cmath>
#include "main.h"

#define M_SQRT2_2 0.70710678118654752440084436210485
#define VK_Z 0x5A
#define VK_Q 0x51
#define VK_S 0x53
#define VK_D 0x44
#define VK_P 0x50

#define MAX_THUMB 32768

float InputHandler::xAxisMove;
float InputHandler::yAxisMove;
float InputHandler::xAxisView;
float InputHandler::yAxisView;
int InputHandler::buttons;
int InputHandler::buttonsDown;
int InputHandler::oldKeyboardState;
int InputHandler::newKeyboardState;
bool InputHandler::mouseClicked;
bool InputHandler::mouseMoved;
int InputHandler::mouseDX;
int InputHandler::mouseDY;
bool InputHandler::controllerPluggedIn;
bool InputHandler::controllerEnabled;
XINPUT_STATE InputHandler::oldControllerState;
XINPUT_STATE InputHandler::newControllerState;

InputHandler::~InputHandler () {
}

void InputHandler::init () {
	buttons = 0x0000;
	buttonsDown = 0x0000;
	oldKeyboardState = 0x0000;
	newKeyboardState = 0x0000;
	newKeyboardState = 0x0000;
	controllerPluggedIn = true;
	mouseMoved = false;
	ZeroMemory (&oldControllerState, sizeof (XINPUT_STATE)); // Zeros out the states of the controller.
	ZeroMemory (&newControllerState, sizeof (XINPUT_STATE)); // Zeros out the states of the controller.
}

void InputHandler::update (float elapsed) {
	buttons = 0x0000;
	buttonsDown = 0x0000;
	xAxisMove = 0.0f;
	yAxisMove = 0.0f;
	xAxisView = 0.0f;
	yAxisView = 0.0f;
	updateController (elapsed);
	if (!(controllerPluggedIn && controllerEnabled))
		updateKeyboardAndMouse (elapsed);
}

void InputHandler::SetMouseButtonClicked (bool clicked) {
	mouseClicked = clicked;
}

void InputHandler::SetMouseDeltaPos (int dx, int dy) {
	mouseMoved = true;
	mouseDX = dx;
	mouseDY = dy;
}

float InputHandler::GetAxis (int axis) {
	float res = 0.0f;
	if (axis == AXIS_X_MOVE) {
		res = xAxisMove;
	} else if (axis == AXIS_Y_MOVE) {
		res = yAxisMove;
	} else if (axis == AXIS_X_VIEW) {
		res = xAxisView;
	} else if (axis == AXIS_Y_VIEW) {
		res = yAxisView;
	}
	return res;
}

bool InputHandler::GetButton (int button) {
	return (buttons & button);
}

bool InputHandler::GetButtonDown (int button) {
	return (buttonsDown & button);
}

void InputHandler::updateKeyboardAndMouse (float elapsed) {
	newKeyboardState = 0x0000;

	if (mouseMoved) {
		printDebug ("dx : " + toString (mouseDX) + " ; dy : " + toString (mouseDY));
		xAxisView = mouseDX;
		xAxisView *= elapsed*0.25f;
		yAxisView = mouseDY;
		yAxisView *= elapsed*0.25f;
		mouseDX = 0x0000;
		mouseDY = 0x0000;
		mouseMoved = false;
	}

	if (GetAsyncKeyState (VK_Z)) {
		newKeyboardState |= BUTTON_UP;
		buttons |= BUTTON_UP;
		if (!(oldKeyboardState & BUTTON_UP)) {
			buttonsDown |= BUTTON_UP;
		}
	}
	if (GetAsyncKeyState (VK_Q)) {
		newKeyboardState |= BUTTON_LEFT;
		buttons |= BUTTON_LEFT;
		if (!(oldKeyboardState & BUTTON_LEFT)) {
			buttonsDown |= BUTTON_LEFT;
		}
	}
	if (GetAsyncKeyState (VK_S)) {
		newKeyboardState |= BUTTON_DOWN;
		buttons |= BUTTON_DOWN;
		if (!(oldKeyboardState & BUTTON_DOWN)) {
			buttonsDown |= BUTTON_DOWN;
		}
	}
	if (GetAsyncKeyState (VK_D)) {
		newKeyboardState |= BUTTON_RIGHT;
		buttons |= BUTTON_RIGHT;
		if (!(oldKeyboardState & BUTTON_RIGHT)) {
			buttonsDown |= BUTTON_RIGHT;
		}
	}
	if (GetAsyncKeyState (VK_SPACE)) {
		newKeyboardState |= BUTTON_JUMP;
		buttons |= BUTTON_JUMP;
		if (!(oldKeyboardState & BUTTON_JUMP)) {
			buttonsDown |= BUTTON_JUMP;
		}
	}
	if (GetAsyncKeyState (VK_SHIFT)) {
		newKeyboardState |= BUTTON_SNEAK;
		buttons |= BUTTON_SNEAK;
		if (!(oldKeyboardState & BUTTON_SNEAK)) {
			buttonsDown |= BUTTON_SNEAK;
		}
	}
	if (GetAsyncKeyState (VK_P)) {
		newKeyboardState |= BUTTON_PHYSICS;
		buttons |= BUTTON_PHYSICS;
		if (!(oldKeyboardState & BUTTON_PHYSICS)) {
			buttonsDown |= BUTTON_PHYSICS;
		}
	}

	if (GetButton (BUTTON_UP) || GetButton (BUTTON_DOWN)) {
		if (GetButton (BUTTON_UP) != GetButton (BUTTON_DOWN)) {
			yAxisMove = GetButton (BUTTON_UP) ? 1.0f : -1.0f;
		}
	}
	if (GetButton (BUTTON_LEFT) || GetButton (BUTTON_RIGHT)) {
		if (GetButton (BUTTON_LEFT) != GetButton (BUTTON_RIGHT)) {
			xAxisMove = GetButton (BUTTON_LEFT) ? -1.0f : 1.0f;
			if (GetButton (BUTTON_UP) || GetButton (BUTTON_DOWN)) {
				xAxisMove *= M_SQRT2_2;
				yAxisMove *= M_SQRT2_2;
			}
		}
	}
	if (mouseClicked) {
		buttons |= BUTTON_ACTION;
		if (!(oldKeyboardState & BUTTON_ACTION)) {
			buttonsDown |= BUTTON_ACTION;
		}
	}

	oldKeyboardState = newKeyboardState;
}

void InputHandler::updateController (float elapsed) {
	DWORD dwResult;         // Used to store if a controller is connected
	dwResult = XInputGetState (0, &newControllerState);

	if (dwResult != ERROR_SUCCESS) {
		controllerPluggedIn = false;
		return;
	}

	ZeroMemory (&newControllerState, sizeof (XINPUT_STATE)); // Zeros out the states of the controller.
	dwResult = XInputGetState (0, &newControllerState);

	//*
	if (newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START) {
		if (!(oldControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START)) {
			controllerEnabled = !controllerEnabled;
		}
	}//*/
	if (controllerEnabled) {
		//check Left Joystick Y axis deadzone
		if (newControllerState.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
			newControllerState.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			newControllerState.Gamepad.sThumbLY = 0;
		} else {
			yAxisMove = newControllerState.Gamepad.sThumbLY;
			yAxisMove /= MAX_THUMB;
		}

		//check Left Joystick X axis deadzone
		if (newControllerState.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
			newControllerState.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			newControllerState.Gamepad.sThumbLX = 0;
		} else {
			xAxisMove = newControllerState.Gamepad.sThumbLX;
			xAxisMove /= MAX_THUMB;
		}

		//check Left Joystick Y axis deadzone
		if (newControllerState.Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
			newControllerState.Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			newControllerState.Gamepad.sThumbRY = 0;
		} else {
			yAxisView = newControllerState.Gamepad.sThumbRY;
			yAxisView /= MAX_THUMB;
			yAxisView *= elapsed;
		}

		//check Left Joystick X axis deadzone
		if (newControllerState.Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
			newControllerState.Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			newControllerState.Gamepad.sThumbRX = 0;
		} else {
			xAxisView = newControllerState.Gamepad.sThumbRX;
			xAxisView /= MAX_THUMB;
			xAxisView *= elapsed;
		}

		/*Jump button
		if (newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
		buttons |= BUTTON_JUMP;
		if (!(oldControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) {
		buttonsDown |= BUTTON_JUMP;
		}
		}//*/
		/*
		if (newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B && !(oldControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B)) {}
		if (newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X && !(oldControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X)) {}
		if (newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y && !(oldControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y)) {}
		//*/
		//Use button
		if (newControllerState.Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
			newControllerState.Gamepad.bLeftTrigger = 0;
		} else {
			buttons |= BUTTON_USE;
			if (oldControllerState.Gamepad.bLeftTrigger == 0) {
				buttonsDown |= BUTTON_USE;
			}
		}
		//Action button
		if (newControllerState.Gamepad.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
			newControllerState.Gamepad.bRightTrigger = 0;
		} else {
			buttons |= BUTTON_ACTION;
			if (oldControllerState.Gamepad.bRightTrigger == 0) {
				buttonsDown |= BUTTON_ACTION;
			}
		}

		//*
		if (newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
			buttons |= BUTTON_JUMP;
			if (!(oldControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
				buttonsDown |= BUTTON_JUMP;
			}
		}//*/
		 /*
		 if (newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
		 buttons |= BUTTON_JUMP;
		 if (!(oldControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)) {
		 buttonsDown |= BUTTON_JUMP;
		 }
		 }//*/
	}

	oldControllerState = newControllerState;
}