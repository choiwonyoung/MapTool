#include "stdafx.h"
#include "Camera.h"


Camera::Camera()
	: m_fAngleVertical(45.0f)		// 상하 +45도(x축)
	, m_fAngleHorizon(-90.0f)		// 좌우 -90도(y축)
	, m_fMaxDistance(1000.0f)
{
	D3DXVECTOR3 eye( 0.0f , 0.0f , -200.0f );
	D3DXVECTOR3 look( 0.0f , 0.0f , 0.0f );
	D3DXVECTOR3 up( 0.0f , 1.0f , 0.0f );

	_SetViewParam( eye , look , up );
}

Camera::Camera( D3DXVECTOR3& eye , D3DXVECTOR3& look , D3DXVECTOR3& up , int width , int height )
	: m_fAngleVertical( 45.0f )		// 상하 +45도(x축)
	, m_fAngleHorizon( -90.0f )		// 좌우 -90도(y축)
	, m_fMaxDistance( 1000.0f )
{
	_SetViewParam( eye , look , up );
}


Camera::~Camera()
{
}

void Camera::SetProjParam( float fFOV , float fAspect , float fNearPlane , float fFarPlane )
{
	m_fFOV			= fFOV;
	m_fAspect		= fAspect;
	m_fNearPlane	= fNearPlane;
	m_fFarPlane		= fFarPlane;

	D3DXMatrixPerspectiveFovLH( &m_matProj , fFOV , fAspect , fNearPlane , fFarPlane );
}

void Camera::MoveCamera( float fFoward , float fCross )
{

}

void Camera::RotateCamera( float horizon , float vertical )
{

}

void Camera::ZoomCamera( float zoom )
{

}

void Camera::SetupParams()
{

}

void Camera::_SetViewParam( D3DXVECTOR3 &vEyePt , D3DXVECTOR3& vLookatPt , D3DXVECTOR3& vUpVec )
{
	m_vEyePt = vEyePt;
	m_vLookPt = vLookatPt;
	m_vUpVec = vUpVec;

	SetupParams();
}

