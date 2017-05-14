#pragma once

#include <windows.h>
#include <XInput.h>

#pragma comment(lib, "XInput9_1_0.lib")

#define BUTTON_UP 0x0001
#define BUTTON_LEFT 0x0002
#define BUTTON_DOWN 0x0004
#define BUTTON_RIGHT 0x0008
#define BUTTON_USE 0x0010
#define BUTTON_ACTION 0x0020
#define BUTTON_JUMP 0x0040
#define BUTTON_SNEAK 0x0080
#define BUTTON_PHYSICS 0x0100
#define AXIS_X_MOVE 0x0001
#define AXIS_Y_MOVE 0x0002
#define AXIS_X_VIEW 0x0004
#define AXIS_Y_VIEW 0x0008

class InputHandler {
public:
	static void init ();
	static void update (float elapsed);

	static float GetAxis (int axis);
	static bool GetButton (int button);
	static bool GetButtonDown (int button);
	static void SetMouseButtonClicked (bool clicked);
	static void SetMouseDeltaPos (int dx, int dy);

private:
	static bool controllerPluggedIn;
	static bool controllerEnabled;
	static XINPUT_STATE oldControllerState;
	static XINPUT_STATE newControllerState;

	static int oldKeyboardState;
	static int newKeyboardState;
	static bool mouseClicked;
	static bool mouseMoved;
	static int mouseDX;
	static int mouseDY;

	static int buttons;
	static int buttonsDown;

	static float xAxisMove;
	static float yAxisMove;
	static float xAxisView;
	static float yAxisView;

	InputHandler ();
	~InputHandler ();

	static void updateController (float elapsed);
	static void updateKeyboardAndMouse (float elapsed);
};

