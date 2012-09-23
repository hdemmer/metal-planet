
#include "Globals.h"

extern XMFLOAT3 gPlayerPosition;
extern float gPlayerYaw;
extern float gPlayerPitch;
extern float gPlayerFov;

void PlayerUpdate();

void PlayerSetup();

void PlayerTearDown();

XMMATRIX PlayerWorldMatrix();
XMMATRIX PlayerViewMatrix();
XMMATRIX PlayerProjectionMatrix();

XMVECTOR PlayerEyePosition();
