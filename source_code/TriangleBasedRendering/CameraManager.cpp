//--------------------------------------------------------------------------------------
// File: CameraManager.cpp
//--------------------------------------------------------------------------------------
#include "CameraManager.h"
using namespace DirectX;
GCamera::GCamera(void)
{
	mPosition = XMFLOAT3(0.0f, 0.0f, -1.0f);
	mTarget = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mUp = GMathVF(GMathFV(mPosition) + GMathFV(XMFLOAT3(0, 1, 0)));

	mAngle = 0.5f;
	mClientWidth = 0.0f;
	mClientHeight = 0.0f;
	mNearest = 0.0f;
	mFarthest = 1000.0f;
	mSpeed = 0.5f;

	mRight = XMFLOAT3(1, 0, 0);
	mUp = XMFLOAT3(0, 1, 0);
	mLook = XMFLOAT3(0, 0, 1);

	XMStoreFloat4x4(&mView, XMMatrixIdentity());
	XMStoreFloat4x4(&mProj, XMMatrixIdentity());
	XMStoreFloat4x4(&mOrtho, XMMatrixIdentity());

	buildViewMatrix();
}

GCamera::GCamera(const GCamera& camera)
{
	*this = camera;
}

GCamera& GCamera::operator=(const GCamera& camera)
{
	mPosition = camera.mPosition;
	mTarget = camera.mTarget;
	mUp = camera.mUp;

	mAngle = camera.mAngle;
	mClientWidth = camera.mClientWidth;
	mClientHeight = camera.mClientHeight;
	mNearest = camera.mNearest;
	mFarthest = camera.mFarthest;

	mView = camera.mView;
	mProj = camera.mProj;
	mOrtho = camera.mOrtho;
	return *this;
}

void GCamera::lookatViewMatrix()
{

	mLook = GMathVF(XMVector3Normalize(XMLoadFloat3(&mTarget) - XMLoadFloat3(&mPosition)));
	XMStoreFloat4x4(&mView, XMMatrixLookAtLH(XMLoadFloat3(&mPosition), XMLoadFloat3(&mTarget),
		XMLoadFloat3(&this->Up())));

}

void GCamera::buildViewMatrix()
{
	XMVECTOR vecLookW = XMVector3Normalize(GMathFV(mLook));
	XMVECTOR vecRightW = XMVector3Normalize(XMVector3Cross(GMathFV(mUp), vecLookW));
	XMVECTOR vecUpW = XMVector3Normalize(XMVector3Cross(vecLookW, vecRightW));
	XMVECTOR vecPosW = GMathFV(mPosition);





	float x = -GMathVF(XMVector3Dot(vecPosW, vecRightW)).x;
	float y = -GMathVF(XMVector3Dot(vecPosW, vecUpW)).x;
	float z = -GMathVF(XMVector3Dot(vecPosW, vecLookW)).x;
	mRight = GMathVF(vecRightW);
	mUp = GMathVF(vecUpW);
	mLook = GMathVF(vecLookW);

	mView(0, 0) = mRight.x;
	mView(1, 0) = mRight.y;
	mView(2, 0) = mRight.z;
	mView(3, 0) = x;

	mView(0, 1) = mUp.x;
	mView(1, 1) = mUp.y;
	mView(2, 1) = mUp.z;
	mView(3, 1) = y;

	mView(0, 2) = mLook.x;
	mView(1, 2) = mLook.y;
	mView(2, 2) = mLook.z;
	mView(3, 2) = z;

	mView(0, 3) = 0.0f;
	mView(1, 3) = 0.0f;
	mView(2, 3) = 0.0f;
	mView(3, 3) = 1.0f;



}

void GCamera::InitProjMatrix(const float angle, const float client_width, const float client_height,
	const float near_plane, const float far_plane)
{
	mAngle = angle;
	mClientWidth = client_width;
	mClientHeight = client_height;
	mNearest = near_plane;
	mFarthest = far_plane;
	XMStoreFloat4x4(&mProj, XMMatrixPerspectiveFovLH(angle, client_width / client_height,
		near_plane, far_plane));


}

void GCamera::Move(XMFLOAT3 direction)
{
	mPosition = GMathVF(XMVector3Transform(GMathFV(mPosition),
		XMMatrixTranslation(direction.x, direction.y, direction.z)));
	mTarget = GMathVF(XMVector3Transform(GMathFV(mTarget),
		XMMatrixTranslation(direction.x, direction.y, direction.z)));
	mUp = GMathVF(XMVector3Transform(GMathFV(mUp),
		XMMatrixTranslation(direction.x, direction.y, direction.z)));

	this->lookatViewMatrix();
}

void GCamera::Rotate(XMFLOAT3 axis, float degrees)
{
	if (XMVector3Equal(GMathFV(axis), XMVectorZero()) ||
		degrees == 0.0f)
		return;

	// Rotate vectors.
	XMFLOAT3 look_at_target = GMathVF(GMathFV(mTarget) - GMathFV(mPosition));
	XMFLOAT3 look_at_up = GMathVF(GMathFV(mUp) - GMathFV(mPosition));
	look_at_target = GMathVF(XMVector3Transform(GMathFV(look_at_target),
		XMMatrixRotationAxis(GMathFV(axis), XMConvertToRadians(degrees))));
	look_at_up = GMathVF(XMVector3Transform(GMathFV(look_at_up),
		XMMatrixRotationAxis(GMathFV(axis), XMConvertToRadians(degrees))));

	// Restore vectors's end points mTarget and mUp from new rotated vectors.
	mTarget = GMathVF(GMathFV(mPosition) + GMathFV(look_at_target));
	mUp = GMathVF(GMathFV(mPosition) + GMathFV(look_at_up));

	this->lookatViewMatrix();
}

void GCamera::Target(XMFLOAT3 new_target)
{

	mTarget = new_target;
	this->lookatViewMatrix();
}

const DirectX::XMFLOAT3 GCamera::Up()
{
	return GMathVF(GMathFV(mUp) - GMathFV(mPosition));
}

DirectX::XMFLOAT3 GCamera::LookAtTarget()
{
	return GMathVF(GMathFV(mTarget) - GMathFV(mPosition));
}

// Set the camera position.
void GCamera::Position(XMFLOAT3& new_position)
{
	mPosition = new_position;

}

void GCamera::Angle(float angle)
{
	mAngle = angle;
	InitProjMatrix(mAngle, mClientWidth, mClientHeight, mNearest, mFarthest);
}

void GCamera::NearestPlane(float nearest)
{
	mNearest = nearest;
	OnResize(mClientWidth, mClientHeight);
}

void GCamera::FarthestPlane(float farthest)
{
	mFarthest = farthest;
	OnResize(mClientWidth, mClientHeight);
}

void GCamera::KeyDown(UINT key)
{

	if (key == 0x57)
		m_forward = true;
	if (key == 0x53)
		m_back = true;
	if (key == 0x44)
		m_right = true;
	if (key == 0x41)
		m_left = true;


}

void GCamera::KeyUp(UINT key)
{
	if (key == 0x57)
		m_forward = false;
	if (key == 0x53)
		m_back = false;
	if (key == 0x44)
		m_right = false;
	if (key == 0x41)
		m_left = false;
}

void GCamera::Update()
{
	XMVECTOR m_moveCommand = GMathFV(XMFLOAT3(0, 0, 0));
	XMVECTOR  forward = GMathFV(mLook);
	XMVECTOR  right = GMathFV(mRight);
	if (m_forward)
		m_moveCommand += forward;
	if (m_back)
		m_moveCommand -= forward;

	if (m_left)
		m_moveCommand -= right;
	if (m_right)
		m_moveCommand += right;

	XMMATRIX R = XMMatrixRotationAxis(GMathFV(mRight), m_pitch);
	XMVECTOR vecLookW = XMVector3TransformCoord(GMathFV(mLook), R);
	XMVECTOR vecUpW = XMVector3TransformCoord(GMathFV(mUp), R);
	R = XMMatrixRotationY(m_yaw);
	XMVECTOR vecRightW = XMVector3TransformCoord(GMathFV(mRight), R);
	vecUpW = XMVector3TransformCoord(vecUpW, R);
	vecLookW = XMVector3TransformCoord(vecLookW, R);
	mRight = GMathVF(vecRightW);
	mUp = GMathVF(vecUpW);
	mLook = GMathVF(vecLookW);

	buildViewMatrix();
	mPosition = GMathVF(m_moveCommand*mSpeed + GMathFV(mPosition));

	m_pitch = 0;
	m_yaw = 0;
	m_forward = false;
	m_back = false;
	m_right = false;
	m_left = false;
}

void GCamera::InputPress(float x, float y)
{
	m_lookLastPoint = XMFLOAT2(x, y);							// Save a point for later move.
	m_lookLastDelta.x = m_lookLastDelta.y = 0;
	m_lookInUse = !m_lookInUse;
}

void GCamera::InputMove(float x, float y)
{
	POINT p;
	if (GetCursorPos(&p))
	{
		x = static_cast<float> (p.x);
		y = static_cast<float>(p.y);

		XMFLOAT2 pointerDelta;

		pointerDelta = GMathVF2(GMathFV(XMFLOAT2(x, y)) - GMathFV(m_lookLastPoint));		// How far did pointer move.
		if (pointerDelta.x < 100 && pointerDelta.y < 100)
		{



			XMFLOAT2 rotationDelta;
			rotationDelta = GMathVF2(GMathFV(pointerDelta) * ROTATION_GAIN);	// Scale for control sensitivity.

															// Update our orientation based on the command.
			m_pitch = rotationDelta.y;						// Mouse y increases down, but pitch increases up.
			m_yaw = rotationDelta.x;						// Yaw defined as CCW around y-axis.
		}
		m_lookLastPoint = XMFLOAT2(x, y);		 			// Save for next time.


	}


}

void GCamera::InputRelease()
{
	m_lookInUse = false;
	m_lookPointerID = 0;
}

void GCamera::InitOrthoMatrix(const float clientWidth, const float clientHeight,
	const float nearZ, const float fartherZ)
{
	XMStoreFloat4x4(&mOrtho, XMMatrixOrthographicLH(clientWidth, clientHeight, nearZ, fartherZ));
}

void GCamera::OnResize(float new_width, float new_height)
{
	mClientWidth = new_width;
	mClientHeight = new_height;
	InitProjMatrix(mAngle, static_cast<float>(new_width), static_cast<float>(new_height), mNearest, mFarthest);
	InitOrthoMatrix(static_cast<float>(new_width), static_cast<float>(new_height), 0.0f, mFarthest);
}

