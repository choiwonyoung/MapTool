#include "stdafx.h"
#include "Camera.h"


Camera::Camera()
	: m_fAngleVertical(45.0f)		// ���� +45��(x��)
	, m_fAngleHorizon(-90.0f)		// �¿� -90��(y��)
	, m_fMaxDistance(1000.0f)
{
	D3DXVECTOR3 eye( 0.0f , 0.0f , -200.0f );
	D3DXVECTOR3 look( 0.0f , 0.0f , 0.0f );
	D3DXVECTOR3 up( 0.0f , 1.0f , 0.0f );

	_SetViewParam( eye , look , up );
}

Camera::Camera( D3DXVECTOR3& eye , D3DXVECTOR3& look , D3DXVECTOR3& up , int width , int height )
	: m_fAngleVertical( 45.0f )		// ���� +45��(x��)
	, m_fAngleHorizon( -90.0f )		// �¿� -90��(y��)
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

//-------------------------------------------------------------------------------------------
// ī�޶� �յ� , �¿�� �̵�
//-------------------------------------------------------------------------------------------
void Camera::MoveCamera( float fFoward , float fCross )
{
	m_vEyePt	+= ( m_vView * fFoward );
	m_vLookPt	+= ( m_vView * fFoward );

	m_vEyePt	-= ( m_vCross * fCross );
	m_vLookPt	-= ( m_vCross * fCross );

	_SetupParams();
}

//-------------------------------------------------------------------------------------------
// ���� lookat�� �߽����� �¿� �� �Ʒ��� ȸ���Ѵ�.
//-------------------------------------------------------------------------------------------
void Camera::RotateCamera( float horizon , float vertical )
{
	// ���� ������ �߰��� ���Ѵ�.
	m_fAngleHorizon		+= horizon;
	m_fAngleVertical	+= vertical;

	// �ִ밪 ����
	// �¿�� 360�� ~ -360��
	if( m_fAngleHorizon > 360.0f )
		m_fAngleHorizon -= 360.0f;
	else if( m_fAngleHorizon < 0.0f )
		m_fAngleHorizon += 360.0f;

	// ���� 85���� ���� ���ϰ�
	if( m_fAngleVertical > 80.0f )
		m_fAngleVertical = 80.0f;

	// �Ʒ��δ� 5���� ���� ���ϰ�
	if( m_fAngleVertical < 8.0f )
		m_fAngleVertical = 8.0f;

	_SetupParams();
}

//-------------------------------------------------------------------------------------------
// ���� ī�޶� �ָ� �ϰų� ������ �Ѵ�.
//-------------------------------------------------------------------------------------------
void Camera::ZoomCamera( float zoom )
{
	D3DXVECTOR3 vec = m_vLookPt - m_vEyePt;			// ī�޶� ���͸� ���Ѵ�.

	// ���� ���͸� ���� ������ �����(zoom) ������ ũ�⸦ ���Ѵ�.
	D3DXVec3Normalize( &vec , &vec );
	vec *= zoom;

	// ������ ��ġ�� ������ ������ �Ÿ��� ���ؼ� �Ѱ����� ���Ѵ�.
	D3DXVECTOR3 checkVec = m_vEyePt - m_vLookPt + vec;
	float length = D3DXVec3Length( &checkVec );
	if( 50.0f < length && length <= m_fMaxDistance )
		m_vEyePt += vec;

	_SetupParams();
}
//-------------------------------------------------------------------------------------------
// Desc : ī�޶��� ��ġ�� ������(���� �� , �� , ������)�� ����� ī�޶� ��ġ�� ����
// ī�޶��� ��ġ���� �ٶ󺸴� ��ġ������ ���� �� ���� , ���� ȸ�� ���� �˶��� ���� ��ǥ�迡�� ���� 
// ��ǥ����� ��ȯ ������ ����ϸ� ī�޶��� ��ġ�� ���ϴ� ������ ����� �� �ִ�.
//-------------------------------------------------------------------------------------------
void Camera::_SetupParams()
{
	//-------------------------------------------------------------------------------------------
	// Look Point �� ���� ������ Camera Point �� ���Ѵ�.
	//-------------------------------------------------------------------------------------------
	D3DXVECTOR3 vec = m_vEyePt;
	
	/******************************************************************************
	// �˰���
	// ���� ��ǥ�迡�� ���� ��ǥ����� ��ȯ ������ ��� 
	// https://vvvv.org/blog/polar-spherical-and-geographic-coordinates �� ���� ��ǥ�踦 �޼� ��ǥ��� �ٲٴ� ������ ����
	// dx = cos(vertical)cos(horizon)
	// dy = sin(vertical)
	// dz = cos(vertical)sin(horizon)	
	*******************************************************************************/
	vec -= m_vLookPt;
	float length = D3DXVec3Length( &vec );

	vec.x = length * cosf( D3DXToRadian( m_fAngleVertical ) ) * cosf( D3DXToRadian( m_fAngleHorizon ) );
	vec.y = length * sinf( D3DXToRadian( m_fAngleVertical ) );
	vec.z = length * cosf( D3DXToRadian( m_fAngleVertical ) ) * sinf( D3DXToRadian( m_fAngleHorizon ) );

	vec += m_vLookPt;
	m_vEyePt = vec;

	//-------------------------------------------------------------------------------------------
	// view(�ٶ󺸴� �� , z��) , up( ���� , y�� ) , cross(������ , x��) �� ���� ���͸� ���Ѵ�.
	//-------------------------------------------------------------------------------------------
	vec = m_vLookPt - m_vEyePt;
	vec.y = 0.0f;

	D3DXVec3Normalize( &m_vView , &vec );				// �ٶ󺸴� �� �������� �����
	D3DXVec3Cross( &m_vCross , &m_vUpVec , &m_vView );	// cross(������ ���� ���� �����(view �� up���� ����)

	// ī�޶� ��� ����
	D3DXMatrixLookAtLH( &m_matView , &m_vEyePt , &m_vLookPt , &m_vUpVec );

	/*
		������ ȿ���� ����� ����� 2����
		1. ��� �̿�
			- ���� �츮�� ����̽��� �� ����� �������� �ʴ´ٸ� ȭ���� ���� ��ǥ�� (-1,-1,0) ~ (1,1,0) ������ ������ ����
			 ���� ��ȯ�� ���� ��� * �� ��� * ���� ����� ����� �̿��� ��ȯ�� ��ġ�ϴµ� ���� ���� ��� * ���� ����� * �� ��� * ���� ���
			 �� �̿��� ��ȯ�� ��ģ�ٸ� ���� ��� * ���� ��� ��ȯ�� �����ϰ� �Ǿ� ���� ��ǥ�� (-1,-1,0) ~ (1,1,0) ������ ������ �׸��� ��
			 ����� ������ ��ü�� ��ġ�� ���� ��ǥ x��� y�࿡ �����ϵ��� ���� . �׸��� ���� ����Ŀ��� _41, _42, _43 ���� 0���� �����ؼ� �̸� 
			 ������ ����̶� �� ����, ������ ��ġ�� ������ ����� ���ؼ� ��ȯ
		2. ī�޶� ���� �̿�
			- ����� �̿����� �ʰ� ī�޶��� x,y���� �̿��ϸ� �׻� ������ ����� ī�޶��� z��� ��ġ�ϹǷ� �ٷ� ������ ȿ���� ����� �� �� ����
			ī�޶��� x, y �� ���� ���Ϳ� ������ ũ�⸦ ���� �̰��� ��ü�� ��ġ�� ���ϸ� �ٷ� ������ ȿ���� ����� �� �� ����
	*/
	D3DXMatrixInverse( &m_matBillBoard , NULL , &m_matView );
	m_matBillBoard._41 = 0.0f;
	m_matBillBoard._42 = 0.0f;
	m_matBillBoard._43 = 0.0f;
}


void Camera::_SetViewParam( D3DXVECTOR3 &vEyePt , D3DXVECTOR3& vLookatPt , D3DXVECTOR3& vUpVec )
{
	m_vEyePt = vEyePt;
	m_vLookPt = vLookatPt;
	m_vUpVec = vUpVec;

	_SetupParams();
}

