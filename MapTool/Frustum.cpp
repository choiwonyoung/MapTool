#include "stdafx.h"
#include "Frustum.h"


Frustum::Frustum()
{
}

Frustum::Frustum( LPDIRECT3DDEVICE9 pDevice )
	: m_pDevice( pDevice )
{
}


Frustum::~Frustum()
{
}

void Frustum::Init()
{
	// ���� ��ȯ�� ��ǥ�� x , y�� -1.0 ~ 1.0 z�� 0 ~ 1 �� ��ȯ
	// �ؿ� ��� ��ǥ ��
	m_vTx[0] = D3DXVECTOR3( -1.0f , -1.0f , 0.0f );
	m_vTx[1] = D3DXVECTOR3(  1.0f , -1.0f , 0.0f );
	m_vTx[2] = D3DXVECTOR3(  1.0f , -1.0f , 1.0f );
	m_vTx[3] = D3DXVECTOR3( -1.0f , -1.0f , 1.0f );
	// �� ��� ��ǥ ��
	m_vTx[4] = D3DXVECTOR3( -1.0f ,  1.0f , 0.0f );
	m_vTx[5] = D3DXVECTOR3(  1.0f ,  1.0f , 0.0f );
	m_vTx[6] = D3DXVECTOR3(  1.0f ,  1.0f ,  1.0f );
	m_vTx[7] = D3DXVECTOR3( -1.0f ,  1.0f ,  1.0f );

	D3DXMATRIX matView , matProj , matViewProj;
	m_pDevice->GetTransform( D3DTS_VIEW , &matView );
	m_pDevice->GetTransform( D3DTS_PROJECTION , &matProj );
	D3DXMatrixMultiply( &matViewProj , &matView , &matProj );
	D3DXMatrixInverse( &matViewProj , NULL , &matViewProj );

	// �� ��� * �������� ����� ����� �ϸ� ���� ��ǥ���� �������� ��ǥ�� ���� �� �ִ�.
	for( int i = 0 ; i < FRUSTUM_VERTEX_MAX ; ++i )
	{
		D3DXVec3TransformCoord( &m_vTx[i] , &m_vTx[i] , &matViewProj );
	}

	/*
		D3DXPlaneFromPoints �� ������ �־����� �� ����� ���ϴ� �Լ�
	*/
	// frustum�� �����Ѵ�. �븻 ������ �ٱ���
	D3DXPlaneFromPoints( &m_plane[0] , &m_vTx[2] , &m_vTx[6] , &m_vTx[7] );		// �� ���(far)
	D3DXPlaneFromPoints( &m_plane[1] , &m_vTx[0] , &m_vTx[3] , &m_vTx[7] );		// �� ���(left)
	D3DXPlaneFromPoints( &m_plane[2] , &m_vTx[1] , &m_vTx[5] , &m_vTx[6] );		// �� ���(right)
	D3DXPlaneFromPoints( &m_plane[3] , &m_vTx[4] , &m_vTx[7] , &m_vTx[6] );		// �� ���(up)
	D3DXPlaneFromPoints( &m_plane[4] , &m_vTx[0] , &m_vTx[1] , &m_vTx[2] );		// �� ���(down)
	D3DXPlaneFromPoints( &m_plane[5] , &m_vTx[0] , &m_vTx[4] , &m_vTx[5] );		// �� ���(near)

}

bool Frustum::VertexIsInFrustum( D3DXVECTOR3* pv )
{
	for( int i = 0 ; i < PLANE_MAX ; ++i )
	{
		float dist = D3DXPlaneDotCoord( &m_plane[i] , pv );
		if( dist > 0.0f )
			return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// pv : Sphere �� �߽��� , raduis �� ������
//////////////////////////////////////////////////////////////////////////
bool Frustum::SphereIsInFrustum( D3DXVECTOR3* pv , float radius )
{
	for( int i = 0 ; i < PLANE_MAX ; ++i )
	{
		float dist = D3DXPlaneDotCoord( &m_plane[i] , pv );
		if( dist > radius )
			return false;
	}

	return true;
}

bool Frustum::Draw()
{
	// �������� �� ���� �ε��� �迭
	WORD index[] = {0 , 1 , 2 ,
					0 , 2 , 3 ,
					4 , 7 , 6 ,
					4 , 6 , 5 ,
					1 , 5 , 6 ,
					1 , 6 , 2 ,
					0 , 3 , 7 ,
					0 , 7 , 4 ,
					0 , 4 , 5 ,
					0 , 5 , 1 ,
					3 , 7 , 6 ,
					3 , 6 , 2 };

	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl , sizeof( D3DMATERIAL9 ) );

	typedef struct tagVTX
	{
		D3DXVECTOR3 p;
	} VTX;

	VTX vtx[8];
	for( int i = 0 ; i < FRUSTUM_VERTEX_MAX ; ++i )
	{
		vtx[i].p = m_vTx[i];
	}

	m_pDevice->SetFVF( D3DFVF_XYZ );

	// ���� ���۸� ��ġ�� ������ ��Ʈ���� ���ε� �Ѵ�.
	m_pDevice->SetStreamSource( 
		0 ,				// ��Ʈ���� �ִ� ���ڱ��� ���� �� �� �ִٴµ� , ���� 0���� ��
		NULL ,			// ���� ���۰� ����� ����Ÿ
		0 ,				// ���� ���ۿ� ���۵� ��Ʈ�� ������ �� , ���� 0�� ����
		sizeof( VTX )	// �������� ũ��� �ռ� ������ ����ü�� ũ�⸦ �־���
		);
	// �ؽ��ĸ� ��ġ�� ���������� �Ҵ��Ѵ�.
	m_pDevice->SetTexture( 
		0 ,				// �ؽ��ĸ� �����ϴ� �������� �ĺ��� , �������� �ĺ��ڴ� 0���� ���� , �����Ǵ� �������� �ִ����, D3DCAPS.MaxSimultaneousTextureStages �� D3DCAPS9.MaxTextureBlendingStages �� 2 ���� �ɷ¿� ���� ��������.		
		NULL			// �����ϴ� �ؽ���
		);

	// �ε��� ����Ÿ�� �����Ѵ�.
	m_pDevice->SetIndices( 
		0				// IDirect3DIndexBuffer9 ������
		);

	// ���� �Ҵ� �� �� �ִ� �ؽ�ó�� ������Ʈ ���� ����
	m_pDevice->SetTextureStageState( 
		0 ,				// ������Ʈ ���� �����ϴ� �ؽ�ó�� �������� �ĺ��� , 0���� ���� �ִ� 8��Ʈ ���� �ؽ�ó�� ���� �� ���� , Stage�� ������ �� �ִ� �ִ�ġ�� 7
		D3DTSS_COLOROP ,// �����ϴ� �ؽ�ó ������Ʈ ,D3DTEXTURESTAGESTATETYPE �������� ������ ��� 
		D3DTOP_DISABLE 
		);
	m_pDevice->SetTextureStageState( 1 , D3DTSS_COLOROP , D3DTOP_DISABLE );

	// ������ ������Ʈ �Ķ���� ����
	m_pDevice->SetRenderState( 
		D3DRS_ALPHABLENDENABLE , // D3DRENDERSTATETYPE �������� ������ ����� ����
		TRUE 
		);
	m_pDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_ONE );
	m_pDevice->SetRenderState( D3DRS_DESTBLEND , D3DBLEND_ONE );

	// �Ķ������� �� , �� ����� �׸���.
	m_pDevice->SetRenderState( D3DRS_LIGHTING , TRUE );
	ZeroMemory( &mtrl , sizeof( D3DMATERIAL9 ) );
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	m_pDevice->SetMaterial( &mtrl );
	// ����� �޸� �����ͷ� �����ǰ� �ִ� �����ͷ�, �����ǰ� �ִ� ������Ʈ�� �⺻������ ������ �Ѵ�.
	m_pDevice->DrawIndexedPrimitiveUP( 
		D3DPT_TRIANGLELIST ,	// D3DPRIMITIVETYPE ������ ���
		0 ,						// 0�� �������� �ϴ� �ּ��� ���� �ε���
		8 ,						// ���Ǵ� ������ ��
		4 ,						// ���Ǵ� �ε��� ��
		index ,					// �ε��� �����Ϳ� ���� ����� �޸� ������
		D3DFMT_INDEX16 ,		// �ε��� ������ ���� - D3DFORMAT ������ ��� , D3DFMT_INDEX16 (16��Ʈ) , D3DFMT_INDEX32 (32��Ʈ) 
		vtx ,					// ���� ��Ʈ�� 0 �� ����ϱ� ���� ���� �������� ����� �޸� ������.
		sizeof( vtx[0] )		// �� ������ ����Ÿ���� ����(����Ʈ ����)
		);

	// ������� ��,�� ����� �׸���.
	ZeroMemory( &mtrl , sizeof( D3DMATERIAL9 ) );
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	m_pDevice->SetMaterial( &mtrl );
	m_pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST , 0 , 8 , 4 , index + ( 4 * 3 ) , D3DFMT_INDEX16 , vtx , sizeof( vtx[0] ) );

	// ���������� ��,�� ����� �׸���.
	ZeroMemory( &mtrl , sizeof( D3DMATERIAL9 ) );
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	m_pDevice->SetMaterial( &mtrl );
	m_pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST , 0 , 8 , 4 , index + ( 8 * 3 ) , D3DFMT_INDEX16 , vtx , sizeof( vtx[0] ) );

	m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE , FALSE );
	m_pDevice->SetRenderState( D3DRS_LIGHTING , FALSE );
	return true;
}
