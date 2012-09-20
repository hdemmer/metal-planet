
#include "Globals.h"

#include <dinput.h>

D3DXVECTOR3 gPlayerPosition;
float gPlayerYaw;
float gPlayerPitch;

	// DirectInput variables
	LPDIRECTINPUT8 din;    // the pointer to our DirectInput interface
	LPDIRECTINPUTDEVICE8 dinkeyboard;    // the pointer to the keyboard device
	LPDIRECTINPUTDEVICE8 dinmouse;    // the pointer to the mouse device
	BYTE keystate[256];    // the storage for the key-information
	DIMOUSESTATE mousestate;    // the storage for the mouse-information

void PlayerUpdate()
{
	    // get access if we don't have it already
    dinkeyboard->Acquire();
    dinmouse->Acquire();

    // get the input data
    dinkeyboard->GetDeviceState(256, (LPVOID)keystate);
    dinmouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mousestate);

	//// 
	// move player


	
	// Handle input
	
	XMVECTOR desiredDir = XMVectorSet( 0.0f, 0.0f, 0.0f , 0.0f);
	if (keystate[DIK_LEFT]  & 0x80)
		desiredDir =+ XMVectorSet( -1.0f, 0.0f, 0.0f ,0.0f);
	if (keystate[DIK_RIGHT]  & 0x80)
		desiredDir =+ XMVectorSet( 1.0f, 0.0f, 0.0f ,0.0f);
	if (keystate[DIK_UP]  & 0x80)
		desiredDir =+ XMVectorSet( 0.0f, 0.0f, 1.0f ,0.0f);
	if (keystate[DIK_DOWN]  & 0x80)
		desiredDir =+ XMVectorSet( 0.0f, 0.0f, -1.0f ,0.0f);

	desiredDir = XMVector4Normalize(desiredDir);

	XMMATRIX RotateHoriz = XMMatrixRotationY( -1* gPlayerYaw );

	desiredDir = XMVector4Transform(desiredDir,RotateHoriz);

	XMVECTOR candidateVec = gPlayerPosition + desiredDir;//TODO: speed

	gPlayerPosition = candidateVec;
	
	gPlayerYaw -= (float)mousestate.lX*0.001f;
	gPlayerPitch -= (float)mousestate.lY*0.001f;

	if (gPlayerYaw > XM_PI*2)
		gPlayerYaw -= XM_PI*2;

	if (gPlayerYaw < 0)
		gPlayerYaw += XM_PI*2;

	if (gPlayerPitch > XM_PI/2)
		gPlayerPitch = XM_PI/2;

	if (gPlayerPitch < -1*XM_PI/2)
		gPlayerPitch = -1*XM_PI/2;
}

void PlayerSetup()
{
	 // create the DirectInput interface
    DirectInput8Create(ghInstance,    // the handle to the application
                       DIRECTINPUT_VERSION,    // the compatible version
                       IID_IDirectInput8,    // the DirectInput interface version
                       (void**)&din,    // the pointer to the interface
                       NULL);    // COM stuff, so we'll set it to NULL

    // create the devices
    din->CreateDevice(GUID_SysKeyboard,    // the default keyboard ID being used
                      &dinkeyboard,    // the pointer to the device interface
                      NULL);    // COM stuff, so we'll set it to NULL
    din->CreateDevice(GUID_SysMouse,
                      &dinmouse,
                      NULL);

    // set the data formats
    dinkeyboard->SetDataFormat(&c_dfDIKeyboard);
    dinmouse->SetDataFormat(&c_dfDIMouse);

    // set the control you will have over the devices
    dinkeyboard->SetCooperativeLevel(ghWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    dinmouse->SetCooperativeLevel(ghWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);


	gPlayerPosition = D3DXVECTOR3(0,10,-10);
	gPlayerPitch = 0.0f;
	gPlayerYaw = 0.0f;
}

void PlayerTearDown()
{
	    dinkeyboard->Unacquire();    // make sure the keyboard is unacquired
    dinmouse->Unacquire();    // make sure the mouse is unacquired
    din->Release();    // close DirectInput before exiting
}


