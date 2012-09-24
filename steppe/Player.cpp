
#include "Globals.h"


#include <Windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

XMFLOAT3 gPlayerPosition;
float gPlayerYaw;
float gPlayerPitch;
float gPlayerFov;

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
	//XMMATRIX RotateVert = XMMatrixRotationX( -1 * gPlayerPitch );

	desiredDir = XMVector4Transform(desiredDir,RotateHoriz);

	XMVECTOR candidateVec = XMLoadFloat3(&gPlayerPosition) + desiredDir*500*gTimeSinceLastUpdate;
	
	XMStoreFloat3(&gPlayerPosition,candidateVec);

	//////
	
	// sinus waves

	float h=0.0;
	float dx=0.0;
	float dz=0.0;
	
	for (float i=64;i<257;i*=1.8f)
	{
		float u=sin(i);
		float v=cos(i);
		h += 0.01f*sqrt(i)*i*(sin((u*gPlayerPosition.x + v*gPlayerPosition.z)/i) + cos((v*gPlayerPosition.x + u*gPlayerPosition.z)/i));
	}

	gPlayerPosition.y=h*1.5f+500.0f;

	//////
	
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

XMVECTOR PlayerEyePosition()
{	
	return XMLoadFloat3(&gPlayerPosition) + XMVectorSet( 0.0f, 1.7f, 0.0f, 0.0f ); 
}

XMMATRIX PlayerWorldMatrix()
{

	 // Initialize the view matrix
	XMVECTOR Eye = PlayerEyePosition();

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
    XMMATRIX Projection = XMMatrixPerspectiveFovLH( gPlayerFov, SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, 10.0f, 2*WORLD_SIZE );

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


	XMStoreFloat3(&gPlayerPosition,XMVectorSet(WORLD_SIZE/2,0,WORLD_SIZE/2,0));
	gPlayerPitch = 0.1f;
	gPlayerYaw = 3.3f;
	gPlayerFov = XM_PIDIV4*1.1;
}

void PlayerTearDown()
{
	    dinkeyboard->Unacquire();    // make sure the keyboard is unacquired
    dinmouse->Unacquire();    // make sure the mouse is unacquired
    din->Release();    // close DirectInput before exiting
}


