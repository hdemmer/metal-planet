
#include "Globals.h"


#include <Windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

XMFLOAT3 gPlayerPosition;
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

	
	if(keystate[DIK_ESCAPE] & 0x80)
		PostMessage(ghWnd, WM_DESTROY, 0, 0);
	
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

	XMVECTOR candidateVec = XMLoadFloat3(&gPlayerPosition) + desiredDir*WORLD_SIZE*0.001;//TODO: speed

	XMStoreFloat3(&gPlayerPosition,candidateVec);
	
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

XMMATRIX PlayerWorldMatrix()
{

	 // Initialize the view matrix
	XMVECTOR Eye = XMLoadFloat3(&gPlayerPosition) + XMVectorSet( 0.0f, 1.7f, 0.0f, 0.0f ); //move to eye position

    XMVECTOR At = Eye + XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );
    XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    XMMATRIX View = XMMatrixLookAtLH( Eye, At, Up );

	return View;
}

XMMATRIX PlayerViewMatrix()
{

	XMMATRIX RotateHoriz = XMMatrixRotationY( gPlayerYaw );
	XMMATRIX RotateVert = XMMatrixRotationX( gPlayerPitch );

	return RotateHoriz*RotateVert;
}

XMMATRIX PlayerProjectionMatrix()
{
    // Initialize the projection matrix
    XMMATRIX Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, 0.1f, WORLD_SIZE );

	return Projection;
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


	XMStoreFloat3(&gPlayerPosition,XMVectorSet(0,WORLD_SIZE *0.1,-10,0));
	gPlayerPitch = 0.0f;
	gPlayerYaw = 0.0f;
}

void PlayerTearDown()
{
	    dinkeyboard->Unacquire();    // make sure the keyboard is unacquired
    dinmouse->Unacquire();    // make sure the mouse is unacquired
    din->Release();    // close DirectInput before exiting
}


