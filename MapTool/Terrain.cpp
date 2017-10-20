#include "stdafx.h"
#include "Terrain.h"


Terrain::Terrain( LPDIRECT3DDEVICE9 pDevice )
	: m_pDevice(pDevice)
	, m_ppIB(NULL)
	, m_ppTexture(NULL)
	, m_pFrustum(new Frustum(pDevice))
	, m_bFrustumLock(FALSE)
	, m_pTileInfo(NULL)
	, m_pHeightMap(NULL)
	, m_bApplyLOD(FALSE)
	, m_nMaxLODLevel(1)
	, m_nLODLevelSize(1000)
	, m_nCurTextureIdx(-1)			// ���� ���÷��� �� �ؽ��� ��ȣ
	, m_nSelectTextureNum(1)		// ���õ� �ؽ��� ����
	, m_nTextureSplatting(-1)		// ���÷��� ���ϱ� or �����
	, m_fBrushOutSize(0.0f)
	, m_fBrushInSize(0.0f)
	, m_nBrushType(-1)				// �� , �簢��
	, m_bWireFrame(FALSE)
	, m_bBoundaryLine(FALSE)
	, m_bFog(TRUE)
	, m_bLight(TRUE)
	, m_nUpDownMode(-1)
	, m_nHeightRate(5)
	, m_pStrTextureName(NULL)
	, m_vPickPos(D3DXVECTOR3(0.0f , 0.0f , 0.0f))
	, m_bPickSuccess(FALSE)
{
	for( int i = 0 ; i < 3 ; ++i )
	{
		m_pAlphaTexture[i] = NULL;
	}

	::ZeroMemory( &m_MapInfo , sizeof( m_MapInfo ) );

	// �� 4���� �ؽ��� ��밡��(0�� �⺻ , 1~3 ���÷��� �� �ؽ���)
	for( int i = 0 ; i < 4 ; ++i )
	{
		m_nTextureIndex[i] = -1;
	}

	m_vecSplattingTile.clear();
	m_vecVisibleTile.clear();
	m_vecBrushAreaTile.clear();
}


Terrain::~Terrain()
{
	if( m_ppTexture )
	{
		for( int i = 0 ; i < m_MapInfo.m_nTextureNum; ++i )
			SAFE_RELEASE( m_ppTexture[i] );

		SAFE_DELETE( m_ppTexture );
	}

	for( int i = 0 ; i < 3 ; ++i )
		SAFE_RELEASE( m_pAlphaTexture[i] );

	for( int i = 0 ; i < m_nMaxLODLevel ; ++i )
	{
		for( int j = 0 ; j < 5 ; ++j )
		{
			SAFE_RELEASE( m_ppIB[i][j] );
		}
	}
	SAFE_ARRAY_DELETE( m_ppIB );

	SAFE_DELETE( m_pFrustum );
	SAFE_ARRAY_DELETE( m_pTileInfo );
	SAFE_ARRAY_DELETE( m_pHeightMap );
	SAFE_ARRAY_DELETE( m_pStrTextureName );
}

/*
	tile : ���� �����ϴ� ����
	cell : Ÿ�� �ȿ� �����ϴ� �簢�� �Ѱ�(2���� �ﰢ��(������)�� ����)
	cellSpacing: 1�� ���� ũ��(�Ÿ�)
	nTotalTex : ��ü �ؽ��� ����
	baseTexIndex : �⺻ �ؽ��� �ε���
*/
HRESULT Terrain::Init( int tiles , int cells , int cellSpacing , int nTotalTex , int baseTexIndex )
{
	// �� ���� ,  Ÿ�� ���� ����
	_SetUpMapInfo( tiles , cells , cellSpacing , nTotalTex );
}

void Terrain::_SetUpMapInfo( int tiles , int cells , int cellSpacing , int nTotalTex )
{
	m_MapInfo.m_nTiles			= tiles;							// ���� Ÿ�� ����( ���� or ���� )
	m_MapInfo.m_nTotalTiles		= tiles * tiles;					// ��ü Ÿ�� ����( ���� * ���� )
	m_MapInfo.m_nTotalVertices	= tiles * cells + 1;				// ���� ���ؽ� ����( ���� or ���� )

	m_MapInfo.m_nCells			= cells;							// 1Ÿ�� �ȿ� �ִ� �� ����
	m_MapInfo.m_nCellSpacing	= cellSpacing;						// 1Ÿ�� �ȿ� 1�� ���� ũ��(�Ÿ�)
	m_MapInfo.m_nVertices		= ( cells + 1 ) * ( cells + 1 );	// 1Ÿ�� �ȿ� �ִ� ���ؽ� �� ����
	m_MapInfo.m_nIndices		= cells * cells * 2 * 3;			// 1Ÿ�� �ȿ� �ִ� �ε��� �� ����
	m_MapInfo.m_nTriangles		= cells * cells * 2;				// 1Ÿ�� �ȿ� �ִ� �ﰢ�� �� ����
	m_MapInfo.m_nTextureNum		= nTotalTex;						// ��ü �ؽ��� ����

	m_MapInfo.m_nTotalMapSize	= tiles * cells * cellSpacing;		// �� ���� or ���� ������
	m_MapInfo.m_nAlphaTexSize	= tiles * cells * 2;				// ���� �ؽ��� ũ��(2 ��� �ϸ� ���� ������)
}


