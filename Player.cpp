#include "Player.h"



Player::Player()
{
	movementSpeed = 0.1f;

	cameraPosition = Vector3(0.0f, 0.0f, -2.0f);
	lookAt = Vector3(0.0f, 0.0f, 0.0f);
}


Player::~Player()
{
}

void Player::moveCharacter(int factorX, int factorY, int factorZ)
{
	cameraPosition.x += factorX * movementSpeed;
	lookAt.x += factorX * movementSpeed;
		
	cameraPosition.y += factorY * movementSpeed;
	lookAt.y += factorY * movementSpeed;

	cameraPosition.z += factorZ * movementSpeed;
	lookAt.z += factorZ * movementSpeed;
}

void Player::rotateCharacter(bool x, int factorX, bool y, int factorY)
{
	if(x)
		lookAt.x += factorX * rotationSpeed;
	if(y)
		lookAt.y += factorY * rotationSpeed;
}