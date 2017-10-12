#pragma once

#include <d3d9.h>
#include <d3dx9.h>

//////////////////////////////////////////////////////////////////////////
// Picking 관련 클래스
//////////////////////////////////////////////////////////////////////////
/*
	픽킹을 하려면 2D 마우스 좌표를 3D 공간 좌표로 변환을 해야 함
	그러기 위해서는 다음과 같은 순서로 진행하면됨
	1. 마우스 좌표를 가지고 공간상에 반직선을 만든다.(언프로젝션) - 관련 내용은 http://blog.daum.net/gamza-net/6 을 읽어보면 이해됨
	2. 반직선과 교차하는 폴리곤 중 가장 가까운 것을 택한다.( Picking )
	3. 직선과 폴리곤의 교차점을 구한다( 3D 마우스 좌표 )
*/
class Pick
{
public:
	Pick( LPDIRECT3DDEVICE9 pDevice , HWND hWnd );
	~Pick();

	bool Init();
	bool IntersectTriangle( D3DXVECTOR3& v0 , D3DXVECTOR3& v1 , D3DXVECTOR3& v2 , float& dist );

public:
	D3DXVECTOR3			m_vPickRayOrig;
	D3DXVECTOR3			m_vPickRayDir;

private:
	LPDIRECT3DDEVICE9	m_pDevice;
	HWND				m_hWnd;

};

extern Pick* g_pPick;