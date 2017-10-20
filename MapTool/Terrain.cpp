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
	, m_nCurTextureIdx(-1)			// 현재 스플래팅 할 텍스쳐 번호
	, m_nSelectTextureNum(1)		// 선택된 텍스쳐 개수
	, m_nTextureSplatting(-1)		// 스플래팅 더하기 or 지우기
	, m_fBrushOutSize(0.0f)
	, m_fBrushInSize(0.0f)
	, m_nBrushType(-1)				// 원 , 사각형
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

	// 총 4개의 텍스쳐 사용가능(0번 기본 , 1~3 스플래팅 할 텍스쳐)
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
	tile : 셀을 포함하는 개념
	cell : 타일 안에 존재하는 사각형 한개(2개의 삼각형(폴리곤)을 포함)
	cellSpacing: 1개 셀의 크기(거리)
	nTotalTex : 전체 텍스쳐 개수
	baseTexIndex : 기본 텍스쳐 인덱스
*/
HRESULT Terrain::Init( int tiles , int cells , int cellSpacing , int nTotalTex , int baseTexIndex )
{
	// 맵 정보 ,  타일 정보 세팅
	_SetUpMapInfo( tiles , cells , cellSpacing , nTotalTex );
}

void Terrain::_SetUpMapInfo( int tiles , int cells , int cellSpacing , int nTotalTex )
{
	m_MapInfo.m_nTiles			= tiles;							// 한쪽 타일 개수( 가로 or 세로 )
	m_MapInfo.m_nTotalTiles		= tiles * tiles;					// 전체 타일 개수( 가로 * 세로 )
	m_MapInfo.m_nTotalVertices	= tiles * cells + 1;				// 한쪽 버텍스 개수( 가로 or 세로 )

	m_MapInfo.m_nCells			= cells;							// 1타일 안에 있는 셀 개수
	m_MapInfo.m_nCellSpacing	= cellSpacing;						// 1타일 안에 1개 셀의 크기(거리)
	m_MapInfo.m_nVertices		= ( cells + 1 ) * ( cells + 1 );	// 1타일 안에 있는 버텍스 총 개수
	m_MapInfo.m_nIndices		= cells * cells * 2 * 3;			// 1타일 안에 있는 인덱스 총 개수
	m_MapInfo.m_nTriangles		= cells * cells * 2;				// 1타일 안에 있는 삼각형 총 개수
	m_MapInfo.m_nTextureNum		= nTotalTex;						// 전체 텍스쳐 개수

	m_MapInfo.m_nTotalMapSize	= tiles * cells * cellSpacing;		// 맵 가로 or 세로 사이즈
	m_MapInfo.m_nAlphaTexSize	= tiles * cells * 2;				// 알파 텍스쳐 크기(2 배로 하면 좀더 섬세함)
}


