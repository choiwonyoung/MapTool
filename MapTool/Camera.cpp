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

//-------------------------------------------------------------------------------------------
// 카메라를 앞뒤 , 좌우로 이동
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
// 현재 lookat을 중심으로 좌우 위 아래로 회전한다.
//-------------------------------------------------------------------------------------------
void Camera::RotateCamera( float horizon , float vertical )
{
	// 현재 각도에 추가로 더한다.
	m_fAngleHorizon		+= horizon;
	m_fAngleVertical	+= vertical;

	// 최대값 설정
	// 좌우는 360도 ~ -360도
	if( m_fAngleHorizon > 360.0f )
		m_fAngleHorizon -= 360.0f;
	else if( m_fAngleHorizon < 0.0f )
		m_fAngleHorizon += 360.0f;

	// 위로 85도를 넘지 못하게
	if( m_fAngleVertical > 80.0f )
		m_fAngleVertical = 80.0f;

	// 아래로는 5도를 넘지 못하게
	if( m_fAngleVertical < 8.0f )
		m_fAngleVertical = 8.0f;

	_SetupParams();
}

//-------------------------------------------------------------------------------------------
// 현재 카메라를 멀리 하거나 가까이 한다.
//-------------------------------------------------------------------------------------------
void Camera::ZoomCamera( float zoom )
{
	D3DXVECTOR3 vec = m_vLookPt - m_vEyePt;			// 카메라 벡터를 구한다.

	// 단위 벡터를 만들어서 다음에 적용될(zoom) 벡터의 크기를 구한다.
	D3DXVec3Normalize( &vec , &vec );
	vec *= zoom;

	// 현재의 위치와 적용할 벡터의 거리를 구해서 한계점을 정한다.
	D3DXVECTOR3 checkVec = m_vEyePt - m_vLookPt + vec;
	float length = D3DXVec3Length( &checkVec );
	if( 50.0f < length && length <= m_fMaxDistance )
		m_vEyePt += vec;

	_SetupParams();
}
//-------------------------------------------------------------------------------------------
// Desc : 카메라의 위치에 정직교(보는 곳 , 위 , 오른쪽)을 만들고 카메라 위치를 구함
// 카메라의 위치에서 바라보는 위치까지의 길이 와 수직 , 수평 회전 값을 알때는 구면 좌표계에서 직교 
// 좌표계로의 변환 공식을 사용하면 카메라의 위치를 구하는 공식을 사용할 수 있다.
//-------------------------------------------------------------------------------------------
void Camera::_SetupParams()
{
	//-------------------------------------------------------------------------------------------
	// Look Point 로 부터 현재의 Camera Point 를 구한다.
	//-------------------------------------------------------------------------------------------
	D3DXVECTOR3 vec = m_vEyePt;
	
	/******************************************************************************
	// 알고리즘
	// 구면 좌표계에서 직교 좌표계로의 변환 공식을 사용 
	// https://vvvv.org/blog/polar-spherical-and-geographic-coordinates 에 구면 좌표계를 왼손 좌표계로 바꾸는 공식을 보라
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
	// view(바라보는 쪽 , z축) , up( 위쪽 , y축 ) , cross(오른쪽 , x축) 의 단위 벡터를 구한다.
	//-------------------------------------------------------------------------------------------
	vec = m_vLookPt - m_vEyePt;
	vec.y = 0.0f;

	D3DXVec3Normalize( &m_vView , &vec );				// 바라보는 곳 단위벡터 만들기
	D3DXVec3Cross( &m_vCross , &m_vUpVec , &m_vView );	// cross(오른쪽 단위 벡터 만들기(view 와 up벡터 외적)

	// 카메라 뷰로 적용
	D3DXMatrixLookAtLH( &m_matView , &m_vEyePt , &m_vLookPt , &m_vUpVec );

	/*
		빌보드 효과를 만드는 방법은 2가지
		1. 행렬 이용
			- 만약 우리가 디바이스에 뷰 행렬을 설정하지 않는다면 화면은 월드 좌표의 (-1,-1,0) ~ (1,1,0) 범위의 정점을 연출
			 정점 변환은 월드 행렬 * 뷰 행렬 * 투영 행렬의 행렬을 이용한 변환과 일치하는데 만약 월드 행렬 * 뷰의 역행렬 * 뷰 행렬 * 투영 행렬
			 을 이용한 변환을 거친다면 월드 행렬 * 투영 행렬 변환과 동일하게 되어 월드 좌표의 (-1,-1,0) ~ (1,1,0) 범위의 정점만 그리게 됨
			 장면을 구성할 물체의 위치를 월드 좌표 x축과 y축에 평행하도록 구성 . 그리고 뷰의 역행렬에서 _41, _42, _43 값을 0으로 설정해서 이를 
			 빌보드 행렬이라 한 다음, 정점의 위치를 빌보드 행렬을 통해서 변환
		2. 카메라 축을 이용
			- 행렬을 이용하지 않고 카메라의 x,y축을 이용하면 항상 빌보드 평면이 카메라의 z축과 일치하므로 바로 빌보드 효과를 만들어 낼 수 있음
			카메라의 x, y 축 방향 벡터에 적당한 크기를 곱해 이것을 물체의 위치에 더하면 바로 빌보드 효과를 만들어 낼 수 있음
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

