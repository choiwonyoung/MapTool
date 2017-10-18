#include "stdafx.h"
#include "Pick.h"

Pick* g_pPick = NULL;

Pick::Pick( LPDIRECT3DDEVICE9 pDevice , HWND hWnd )
	: m_pDevice(pDevice)
	, m_hWnd(hWnd)
{
	Init();
}


Pick::~Pick()
{
}

bool Pick::Init()
{
	POINT ptCursor;
	// 스크린 좌표에서의 커서 위치를 구한다.
	GetCursorPos( &ptCursor );
	// 핸들에 대한 영역내에 커서로 다시 설정된다.
	ScreenToClient( m_hWnd , &ptCursor );
	m_pDevice->SetCursorPosition( ptCursor.x , ptCursor.y , 0L );

	RECT rect;
	GetClientRect( m_hWnd , &rect );

	if( ( rect.left > ptCursor.x || rect.right < ptCursor.x ) ||
		( rect.top > ptCursor.y || rect.bottom < ptCursor.y ) )
	{
		return false;
	}

	D3DVIEWPORT9 vp;
	m_pDevice->GetViewport( &vp );

	// UnProjection
	D3DXVECTOR3 v;
	D3DXMATRIX matProj;
	m_pDevice->GetTransform( D3DTS_PROJECTION , &matProj );
// 	LONG x = ptCursor.x - vp.X;
// 	float xTemp = static_cast<float>(x) * 2.0f / vp.Width;
// 	float xFinal = xTemp - 1.0f;
// 	v.x = ( xFinal - matProj._31 ) / matProj._11;
// 	LONG y = ptCursor.y - vp.Y;
// 	float yTemp = static_cast<float>(y) * 2.0f / vp.Height;
// 	float yFinal = -( yTemp - 1.0f );
// 	v.y = ( yFinal - matProj._32 ) / matProj._22;

	// 공간좌표에서 화면좌표로의 변환과정은...
	// 사진을 찍은다음( Mproj ) , 보고싶은부분만 오려서( Mclip ) , 화면에 늘려붙이는( Mvs ) 겁니다.
	// 이부분을 역으로 화면좌표로 변환하는 과정이 UnProjection 임
	v.x = ( ( ( ( ptCursor.x - vp.X )* 2.0f / vp.Width ) - 1.0f ) - matProj._31 ) / matProj._11;
	v.y = ( -( ( ( ptCursor.y - vp.Y )* 2.0f / vp.Width ) - 1.0f ) - matProj._32 ) / matProj._22;
	v.z = 1.0f;

	// 카메라 좌표로...
	D3DXMATRIXA16 matView;
	m_pDevice->GetTransform( D3DTS_VIEW , &matView );
	D3DXMatrixInverse( &matView , NULL , &matProj );

	m_vPickRayDir.x = v.x * matView._11 + v.y * matView._21 + v.z * matView._31;
	m_vPickRayDir.y = v.x * matView._12 + v.y * matView._22 + v.z * matView._32;
	m_vPickRayDir.z = v.x * matView._13 + v.y * matView._23 + v.z * matView._33;

	// 카메라의 위치좌표를 Ray의 원점 좌표로 만든다.
	m_vPickRayOrig.x = matView._41;
	m_vPickRayOrig.y = matView._42;
	m_vPickRayOrig.z = matView._43;

	// 월드 좌표로..
	D3DXMATRIX matWorld;
	m_pDevice->GetTransform( D3DTS_WORLD , &matWorld );
	D3DXMatrixInverse( &matWorld , NULL , &matWorld );
	D3DXVec3TransformCoord( &m_vPickRayDir , &m_vPickRayDir , &matWorld );
	D3DXVec3TransformCoord( &m_vPickRayOrig , &m_vPickRayOrig , &matWorld );

	return true;
}

/*
	카메라에서 시작한 반직선이 삼각형에 교차하는지 검사하는 로직
	임의의 세점을 넣어서 삼각형을 만들고 그 삼각형 내에 rayDir 이 교차하면 거리를 리턴해 주는 로직
	http://blog.daum.net/gamza-net/6 에 설명이 되어있음
	
*/
bool Pick::IntersectTriangle( D3DXVECTOR3& v0 , D3DXVECTOR3& v1 , D3DXVECTOR3& v2 , float& dist )
{
	float det , u , v;
	D3DXVECTOR3 pVec , tVec , qVec;
	D3DXVECTOR3 edge1 = v1 - v0;
	D3DXVECTOR3 edge2 = v2 - v0;

	// det 값은 법선 벡터 N 과 변벡터 edge1 의 내적값
	D3DXVec3Cross( &pVec , &m_vPickRayDir , &edge2 );
	det = D3DXVec3Dot( &edge1 , &pVec );

	// 0으로 나누는 에러 방지와 det값과 u , v 값의 비교를 쉽게 해주는 역할
	if( det < 0.0001f )
		return false;
		
	// u 는 0보다 크고 1보다 작아야 한다.
	tVec = m_vPickRayOrig - v0;
	u = D3DXVec3Dot( &tVec , &pVec );
	if( u < 0.0f || u > det )
		return false;

	/*
		외적과 내적의 성질
		A * ( B x C ) == B * ( C x A ) == C * ( A x B )
	*/
	// v = tvec * ( edge1 x dir ) == 
	// tvec * ( edge1 x dir ) == dir * ( tvec x edge1 )
	D3DXVec3Cross( &qVec , &tVec , &edge1 );
	v = D3DXVec3Dot( &m_vPickRayDir , &qVec );
	if( v < 0.0f || u + v > det )
		return false;

	/*
		A x B = -B x A
		t   = edge2 • ( tvec x edge1 ) = tvec • ( edge1 x edge2 ) = -tvec • ( edge2 x edge1 )
	*/
	dist = D3DXVec3Dot( &edge2 , &qVec );
	dist *= ( 1.0f / det );

	return true;
}
