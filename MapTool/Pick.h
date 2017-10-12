#pragma once

#include <d3d9.h>
#include <d3dx9.h>

//////////////////////////////////////////////////////////////////////////
// Picking ���� Ŭ����
//////////////////////////////////////////////////////////////////////////
/*
	��ŷ�� �Ϸ��� 2D ���콺 ��ǥ�� 3D ���� ��ǥ�� ��ȯ�� �ؾ� ��
	�׷��� ���ؼ��� ������ ���� ������ �����ϸ��
	1. ���콺 ��ǥ�� ������ ������ �������� �����.(����������) - ���� ������ http://blog.daum.net/gamza-net/6 �� �о�� ���ص�
	2. �������� �����ϴ� ������ �� ���� ����� ���� ���Ѵ�.( Picking )
	3. ������ �������� �������� ���Ѵ�( 3D ���콺 ��ǥ )
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