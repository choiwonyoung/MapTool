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
	// 최종 변환된 좌표는 x , y는 -1.0 ~ 1.0 z는 0 ~ 1 로 변환
	// 밑에 평면 좌표 값
	m_vTx[0] = D3DXVECTOR3( -1.0f , -1.0f , 0.0f );
	m_vTx[1] = D3DXVECTOR3(  1.0f , -1.0f , 0.0f );
	m_vTx[2] = D3DXVECTOR3(  1.0f , -1.0f , 1.0f );
	m_vTx[3] = D3DXVECTOR3( -1.0f , -1.0f , 1.0f );
	// 위 평면 좌표 값
	m_vTx[4] = D3DXVECTOR3( -1.0f ,  1.0f , 0.0f );
	m_vTx[5] = D3DXVECTOR3(  1.0f ,  1.0f , 0.0f );
	m_vTx[6] = D3DXVECTOR3(  1.0f ,  1.0f ,  1.0f );
	m_vTx[7] = D3DXVECTOR3( -1.0f ,  1.0f ,  1.0f );

	D3DXMATRIX matView , matProj , matViewProj;
	m_pDevice->GetTransform( D3DTS_VIEW , &matView );
	m_pDevice->GetTransform( D3DTS_PROJECTION , &matProj );
	D3DXMatrixMultiply( &matViewProj , &matView , &matProj );
	D3DXMatrixInverse( &matViewProj , NULL , &matViewProj );

	// 뷰 행렬 * 프로젝션 행렬을 역행렬 하면 월드 좌표계의 프러스텀 좌표를 얻을 수 있다.
	for( int i = 0 ; i < FRUSTUM_VERTEX_MAX ; ++i )
	{
		D3DXVec3TransformCoord( &m_vTx[i] , &m_vTx[i] , &matViewProj );
	}

	/*
		D3DXPlaneFromPoints 는 세점이 주어졌을 때 평면을 구하는 함수
	*/
	// frustum을 구축한다. 노말 방향은 바깥쪽
	D3DXPlaneFromPoints( &m_plane[0] , &m_vTx[2] , &m_vTx[6] , &m_vTx[7] );		// 원 평면(far)
	D3DXPlaneFromPoints( &m_plane[1] , &m_vTx[0] , &m_vTx[3] , &m_vTx[7] );		// 좌 평면(left)
	D3DXPlaneFromPoints( &m_plane[2] , &m_vTx[1] , &m_vTx[5] , &m_vTx[6] );		// 우 평면(right)
	D3DXPlaneFromPoints( &m_plane[3] , &m_vTx[4] , &m_vTx[7] , &m_vTx[6] );		// 상 평면(up)
	D3DXPlaneFromPoints( &m_plane[4] , &m_vTx[0] , &m_vTx[1] , &m_vTx[2] );		// 하 평면(down)
	D3DXPlaneFromPoints( &m_plane[5] , &m_vTx[0] , &m_vTx[4] , &m_vTx[5] );		// 근 평면(near)

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
// pv : Sphere 의 중심점 , raduis 는 반지름
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
	// 프러스텀 각 면의 인덱스 배열
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

	// 정점 버퍼를 장치의 데이터 스트림에 바인드 한다.
	m_pDevice->SetStreamSource( 
		0 ,				// 스트림의 최대 숫자까지 설정 할 수 있다는데 , 보통 0으로 함
		NULL ,			// 정점 버퍼가 저장된 데이타
		0 ,				// 정점 버퍼에 시작될 스트림 오프셋 값 , 보통 0을 지정
		sizeof( VTX )	// 한정점의 크기로 앞서 설정된 구조체의 크기를 넣어줌
		);
	// 텍스쳐를 장치의 스테이지에 할당한다.
	m_pDevice->SetTexture( 
		0 ,				// 텍스쳐를 서정하는 스테이지 식별자 , 스테이지 식별자는 0부터 시작 , 지원되는 스테이지 최대수는, D3DCAPS.MaxSimultaneousTextureStages 및 D3DCAPS9.MaxTextureBlendingStages 의 2 개의 능력에 의해 정해진다.		
		NULL			// 설정하는 텍스쳐
		);

	// 인덱스 데이타를 설정한다.
	m_pDevice->SetIndices( 
		0				// IDirect3DIndexBuffer9 포인터
		);

	// 현재 할당 할 수 있는 텍스처에 스테이트 값을 설정
	m_pDevice->SetTextureStageState( 
		0 ,				// 스테이트 값을 설정하는 텍스처의 스테이지 식별자 , 0부터 시작 최대 8세트 까지 텍스처를 가질 수 있음 , Stage로 지정할 수 있는 최대치는 7
		D3DTSS_COLOROP ,// 설정하는 텍스처 스테이트 ,D3DTEXTURESTAGESTATETYPE 열거형의 임의의 멤버 
		D3DTOP_DISABLE 
		);
	m_pDevice->SetTextureStageState( 1 , D3DTSS_COLOROP , D3DTOP_DISABLE );

	// 렌더링 스테이트 파라미터 설정
	m_pDevice->SetRenderState( 
		D3DRS_ALPHABLENDENABLE , // D3DRENDERSTATETYPE 열거형의 임의의 멤버를 지정
		TRUE 
		);
	m_pDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_ONE );
	m_pDevice->SetRenderState( D3DRS_DESTBLEND , D3DBLEND_ONE );

	// 파란색으로 상 , 하 평면을 그린다.
	m_pDevice->SetRenderState( D3DRS_LIGHTING , TRUE );
	ZeroMemory( &mtrl , sizeof( D3DMATERIAL9 ) );
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	m_pDevice->SetMaterial( &mtrl );
	// 사용자 메모리 포인터로 지정되고 있는 데이터로, 지정되고 있는 지오메트리 기본도형을 렌더링 한다.
	m_pDevice->DrawIndexedPrimitiveUP( 
		D3DPT_TRIANGLELIST ,	// D3DPRIMITIVETYPE 열거형 멤버
		0 ,						// 0을 기준으로 하는 최소의 정점 인덱스
		8 ,						// 사용되는 정점의 수
		4 ,						// 사용되는 인덱스 수
		index ,					// 인덱스 데이터에 대한 사용자 메모리 포인터
		D3DFMT_INDEX16 ,		// 인덱스 데이터 포멧 - D3DFORMAT 열거형 멤버 , D3DFMT_INDEX16 (16비트) , D3DFMT_INDEX32 (32비트) 
		vtx ,					// 정점 스트림 0 에 사용하기 위한 정점 데이터의 사용자 메모리 포인터.
		sizeof( vtx[0] )		// 각 정점의 데이타간의 보폭(바이트 단위)
		);

	// 녹색으로 좌,우 평면을 그린다.
	ZeroMemory( &mtrl , sizeof( D3DMATERIAL9 ) );
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	m_pDevice->SetMaterial( &mtrl );
	m_pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST , 0 , 8 , 4 , index + ( 4 * 3 ) , D3DFMT_INDEX16 , vtx , sizeof( vtx[0] ) );

	// 붉은색으로 원,근 평면을 그린다.
	ZeroMemory( &mtrl , sizeof( D3DMATERIAL9 ) );
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	m_pDevice->SetMaterial( &mtrl );
	m_pDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST , 0 , 8 , 4 , index + ( 8 * 3 ) , D3DFMT_INDEX16 , vtx , sizeof( vtx[0] ) );

	m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE , FALSE );
	m_pDevice->SetRenderState( D3DRS_LIGHTING , FALSE );
	return true;
}
