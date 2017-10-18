#pragma once

#include <d3d9.h>
#include <d3dx9.h>

class Frustum
{
	enum
	{	
		PLANE_MAX = 6,
		FRUSTUM_VERTEX_MAX = 8 ,
	};
public:
	Frustum();
	Frustum( LPDIRECT3DDEVICE9 pDevice );
	~Frustum();

	void Init();
	bool VertexIsInFrustum( D3DXVECTOR3* pv );
	bool SphereIsInFrustum( D3DXVECTOR3* pv , float radius );
	bool Draw();

private:
	LPDIRECT3DDEVICE9	m_pDevice;

	D3DXVECTOR3			m_vTx[8];
	D3DXPLANE			m_plane[6];
};

