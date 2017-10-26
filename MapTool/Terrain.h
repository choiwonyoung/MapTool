#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <vector>

#include "Pick.h"
#include "Frustum.h"

using namespace std;

enum LOD_CRACK
{
	LOD_BASIC = 0,
	LOD_LEFT,
	LOD_TOP,
	LOD_RIGHT,
	LOD_BOTTOM,
};
// 브러쉬
struct BRUSH_VERTEX
{
	D3DXVECTOR3 p;
	DWORD color;
	enum { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE };
};

// 지형
struct TERRAIN_VERTEX
{
	D3DXVECTOR3 p;
	D3DXVECTOR3 n;
	D3DXVECTOR3 t1;		// 타일 uv
	D3DXVECTOR3 t2;		// Alpha Texture uv
};
const DWORD TERRAIN_FVF = { D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 };

// 맵 정보
// 모두다 저장하고 불러옴
struct MapInfo
{
	int m_nTiles;			// 한쪽 타일 개수(가로 or 세로)
	int m_nTotalTiles;		// 전체 타일 개수(가로 * 세로)
	int m_nTotalVertices;	// 전체 버텍스 개수(가로 or 세로)
	int m_nTotalMapSize;	// 전체 맵 크기( 타일 * 셀 * 거리 )
	int m_nAlphaTexSize;	// 알파맵 크기

	int m_nCells;			// 1 타일 안에 있는 셀개수
	int m_nCellSpacing;		// 1 타일 안에 1개 셀의 크기(거리)
	int m_nVertices;		// 1 타일 안에 있는 버텍스 총 개수
	int m_nIndices;			// 1 타일 안에 있는 인덱스 총 개수
	int m_nTriangles;		// 1 타일 안에 있는 삼각형 총 개수
	int m_nTextureNum;		// 전체 텍스쳐 개수
};

// 타일 정보
struct TileInfo
{
	TERRAIN_VERTEX*			m_pVertex;			// 타일 별 정보(위치 , 높이 , 텍스쳐)
	LPDIRECT3DVERTEXBUFFER9 m_pVB;				// TERRAIN_VERTEX* 를 버텍스 버퍼로 저장
	int						m_nEdge[4];			// 타일의 각 모서리( TL , TR , BL , BR )의 인덱스( 전체 중의 인덱스 )
	int						m_nCenter;			// 타일의 가운데 인덱스(전체 인덱스 중)
	int						m_nLODLevel;		// LOD 레벨
	int						m_nLODCrackIndex;	// LOD 디테일 레벨
	int						m_nTriangleNum;		// 타일의 삼각형 개수

	TileInfo()
		: m_pVertex( NULL )
		, m_pVB( NULL )
		, m_nCenter( 0 )
		, m_nLODLevel( 0 )
		, m_nLODCrackIndex( 0 )
		, m_nTriangleNum( 0 )
	{
		for( int i = 0 ; i < 4; ++i )
			m_nEdge[i] = -1;
	}

	~TileInfo()
	{
		SAFE_DELETE( m_pVertex );
		SAFE_RELEASE( m_pVB );
	}
};

// 사각형을 구성하는 4점
struct EdgeInfo
{
	D3DXVECTOR3 TL;		// Top Left
	D3DXVECTOR3 TR;		// Top Right
	D3DXVECTOR3 BL;		// Bottom Left
	D3DXVECTOR3 BR;		// Bottom Right
};

// 4개의 사각형
struct QuadEdgeInfo
{
	EdgeInfo m_part[4];
};

class Terrain
{
public:
	Terrain( LPDIRECT3DDEVICE9 pDevice );
	~Terrain();

	HRESULT Init( int tiles , int cells , int cellSpacing , int nTotalTex , int baseTexIndex );
	HRESULT CreateVB( int index , TileInfo* pTileInfo );
	HRESULT CreateIB( int LODLevel );

	Frustum* GetFrustum() const { return m_pFrustum; }

private:
	void _SetUpMapInfo( int tiles , int cells , int cellSpacing , int nTotalTex );
	void _SetUpHeightMap();
	void _DrawBaseTile( int index );
	void _DrawSplattingTile( int index );
	void _DrawBoundaryLine( int index );

public:
	
	BOOL					m_bFrustumLock;			

	// 맵 정보
	MapInfo					m_MapInfo;				// 맵 정보
	TileInfo*				m_pTileInfo;			// 타일 정보
	TERRAIN_VERTEX*			m_pHeightMap;			// 전체 지형 정보

	// LOD
	BOOL					m_bApplyLOD;			// LOD 적용 여부
	int						m_nMaxLODLevel;			// LOD Max Level
	int						m_nLODLevelSize;		// LOD Level Size

	// Texture
	int						m_nTextureIndex[4];		// 0(기본) ~ 3번 텍스쳐 번호(텍스쳐 리스트의 번호)
	int						m_nCurTextureIdx;		// 현재 텍스쳐 셀 번호(0 ~ 3번)
	int						m_nSelectTextureNum;	// 선택된 텍스쳐 개수
	int						m_nTextureSplatting;	// 스플래팅 그리기 or 지우기

	// 브러쉬가 칠한 곳만 렌더링 하기 위해
	vector<int>				m_vecSplattingTile;

	// 텍스쳐 전체 목록 저장
	string*					m_pStrTextureName;		// 텍스쳐 전체 이름

	// 렌더링 되는 타일 번호
	vector<int>				m_vecVisibleTile;		// 보이는 타일 번호들

	// 브러쉬 영역보다 1타일 씩 큰 타일 번호들(지형 올릴때 브러쉬 영역만 저장하기)
	vector<int>				m_vecBrushAreaTile;		// 브러쉬 영역 타일

	// 렌더링 옵션
	BOOL					m_bWireFrame;			// 와이어 프레임 모드
	BOOL					m_bBoundaryLine;		// 경계선 그리기
	BOOL					m_bFog;					// 안개 모드
	BOOL					m_bLight;				// 라이트 모드
	int						m_nUpDownMode;			// 올림 , 내림 , 고원 , 평지
	int						m_nHeightRate;			// 높이 조절

	// 브러쉬
	int						m_nBrushType;			// 원 , 사각형
	float					m_fBrushOutSize;		// 바깥라인(부드럽게)
	float					m_fBrushInSize;			// 안쪽라인(선명하게)

	// 피킹
	D3DXVECTOR3				m_vPickPos;				// 마우스 피킹 좌표
	BOOL					m_bPickSuccess;			// 피킹 성공 여부


private:
	// 프러스텀
	Frustum*				m_pFrustum;				// 프러스텀

	LPDIRECT3DDEVICE9		m_pDevice;
	LPDIRECT3DINDEXBUFFER9	( *m_ppIB )[5];
	LPDIRECT3DTEXTURE9*		m_ppTexture;
	LPDIRECT3DTEXTURE9		m_pAlphaTexture[3];
};

