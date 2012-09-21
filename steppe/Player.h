
#include "Globals.h"

extern XMFLOAT3 gPlayerPosition;

void PlayerUpdate();

void PlayerSetup();

void PlayerTearDown();

XMMATRIX PlayerWorldMatrix();
XMMATRIX PlayerViewMatrix();
XMMATRIX PlayerProjectionMatrix();

XMVECTOR PlayerEyePosition();
