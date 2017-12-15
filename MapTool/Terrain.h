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

enum UPDWON_MODE
{
	UP = 0,
	DOWN,
	FLAT,
	ORIGIN,
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
	HRESULT CreateTexture( int index , string strTexName );

	// 알파 텍스쳐 관련 함수
	HRESULT CreateAlphaTexture( int index );
	HRESULT ChangeAlphaTexture( int dest , int src );
	void	SaveAlphaValue( int index , BYTE* pRead );
	void	LoadAlphaValue( int index , BYTE* pRead );
	void	SaveAlphaTexture( int index , const char* pFileName );

	//지형 관련 (불러올때, 높이 값 얻기 , 노말계산)
	void	LoadTerrain();	
	void	ComputeNormal( POINT pos[3] );	
	float	GetHeight( float x , float z );

	// 그리기 함수
	void	DrawTerrain();
	void	BrushDraw( int count , float outSize , float inSize , D3DXCOLOR outColor , D3DXCOLOR inColor , bool bDrawInBrush );

	// 높이 및 텍스쳐 설정
	void	EditHeightInCircle();
	void	EditHeightInRectangle();
	void	MakeAlphaTexture( int index , int brushType );

	// LOD 설정
	void	SetupLOD( D3DXVECTOR3 look );

	// 최적화 피킹
	void PickTerrain( Pick* pPick );

	Frustum* GetFrustum() const { return m_pFrustum; }

private:
	void _SetUpMapInfo( int tiles , int cells , int cellSpacing , int nTotalTex );
	void _SetUpHeightMap();
	void _DrawBaseTile( int index );
	void _DrawSplattingTile( int index );
	void _DrawBoundaryLine( int index );

	
	// 브러쉬
	inline void _BrushCircleDraw( int count , float size , D3DXCOLOR color );
	inline void _BrushRectangleDraw( int count , float size , D3DXCOLOR color );

	
	// 최적화를 위한 함수( 높이 계산을 위한 , 브러쉬 영역만 그리기 , 브러쉬 영역 확대 찾기(노말 계산용)
	inline float _Lerp( float a , float b , float t );
	inline void _TexturePaintInBrushArea();
	inline void _SearchTileInPickArea( POINT pos[3] , vector<int>* pVecTile );

	// 피킹 최적화를 위한 함수( 보이는 타일 검색 , 타일 번호 찾기 , 셀 번화 찾기 , 가까운 포지션 찾기 )
	inline int _SearchPositionInTiles( D3DXVECTOR3 pos , EdgeInfo* pTile = NULL );
	inline int _SearchPositionInMap( D3DXVECTOR3 pos , EdgeInfo* pMap );
	inline int _NearPositionInCells( QuadEdgeInfo* pPart , D3DXVECTOR3 pos , EdgeInfo* pCell , int cellIdx );

	// 카메라 Look 위치와 현재 타일과의 거리값을 기준으로 LOD 값을 구한다.
	inline void _GetLODLevel( D3DXVECTOR3 look , int index );
	inline void _SetupCrackIndex( int index );

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


inline float Terrain::_Lerp( float a , float b , float t )
{
	return a - ( a * t ) + ( b* t );
}

inline void Terrain::_BrushCircleDraw( int count , float size , D3DXCOLOR color )
{
	if( count <= 4 )
		count = 4;

	float radian	= D3DX_PI * 2.0f / count;
	float brushSize = size * m_MapInfo.m_nCellSpacing;			// 사이즈 * 셀크기

	BRUSH_VERTEX	brushLine[2];					// 브러쉬 버텍스[2](2개의 라인으로 원을 그릴것 임)
	D3DXVECTOR3		curPos( 1.0f , 0.0f , 0.0f );	// 현재 위치( 초기 위치를 아무축이나 1.0f )
	D3DXVECTOR3		newPos;							// 다움 위치( 두 위치를 잇는다.) LINELIST로 Draw
	D3DXMATRIX		matRot;							// 다음 위치를 정할 회전 행렬

	// 초기 위치[1]는 ( 1.0f , 0.0f , 0.0f ) * 브러쉬 크기 + 피킹 마우스 위치
	brushLine[1].p		= curPos *brushSize + m_vPickPos;
	brushLine[1].p.y	= GetHeight( brushLine[1].p.x , brushLine[1].p.z ) + 0.5f;
	brushLine[1].color	= brushLine[0].color = color;

	// radian이 0이 되지 않기 위해 1부터 시작
	for( int i = 1; i < count + 1 ; ++i )
	{
		// 처음 위에서 정한 위치[1]를 [0]으로 넣고
		brushLine[0].p = brushLine[1].p;

		// 처음 위치를 중심으로 i * radian 만큼 회전후
		// 새로운 위치로 이동한 벡터에 다시 처음[1] 위치를 넣는다.
		D3DXMatrixRotationY( &matRot , i * radian );
		D3DXVec3TransformCoord( &newPos , &curPos , &matRot );
		D3DXVec3Normalize( &newPos , &newPos );

		// 다시 새로운 위치[1]을 정한다.
		brushLine[1].p = newPos * brushSize + m_vPickPos;
		brushLine[1].p.y = GetHeight( brushLine[1].p.x , brushLine[1].p.z ) + 0.5f;

		// 버텍스 버퍼를 생성하지 않고 그리기( DrawPrimitiveUP )
		m_pDevice->SetFVF( BRUSH_VERTEX::FVF );
		// DrawPrimitiveUP은 vertex buffer를 만들지 않고 그릴 수 있다.
		m_pDevice->DrawPrimitiveUP( D3DPT_LINELIST , 1 , brushLine , sizeof( BRUSH_VERTEX ) );
	}

}

inline void Terrain::_BrushRectangleDraw( int count , float size , D3DXCOLOR color )
{
	if( count <= 4 )
		count = 4;

	int quadSize = count / 4;
	float brushSize = size * m_MapInfo.m_nCellSpacing;			// 사이즈 * 셀 크기

	D3DXVECTOR3 vTL( -1.0f , 0.0f ,  1.0f );
	D3DXVECTOR3 vTR(  1.0f , 0.0f ,  1.0f );
	D3DXVECTOR3 vBL( -1.0f , 0.0f , -1.0f );
	D3DXVECTOR3 vBR(  1.0f , 0.0f , -1.0f );
	BRUSH_VERTEX brushLine[2];									// 브러쉬 버텍스[2] (2개의 라인으로 사각형을 그릴것)
	brushLine[0].color = brushLine[1].color = color;

	D3DXVECTOR3 q1( 0.0f , 0.0f , 0.0f );
	D3DXVECTOR3 q2( 0.0f , 0.0f , 0.0f );

	// 4방향의 꼭지점을 미리 구현 뒤 각 방향에 대해 선형보간으로 라인을 그린다.
	for( int quad = 0; quad < 4 ; ++quad )
	{
		switch( quad )
		{
		case 0: // TL  ->  TR (윗줄)
			q1 = vTL * brushSize + m_vPickPos;
			q2 = vTR * brushSize + m_vPickPos;
			break;

		case 1: // TR  ->  BR (오른쪽줄)
			q1 = vTR * brushSize + m_vPickPos;
			q2 = vBR * brushSize + m_vPickPos;
			break;

		case 2: // BL  ->  BR (아랫줄)
			q1 = vBL * brushSize + m_vPickPos;
			q2 = vBR * brushSize + m_vPickPos;
			break;

		case 3: // TL  ->  TR (왼쪽줄)
			q1 = vTL * brushSize + m_vPickPos;
			q2 = vBL * brushSize + m_vPickPos;
			break;
		}

		// count 를 4군데로 나눠서 나눈 만큼 라인을 만든다.
		// q(t) = ( 1 - t )*q1 + t* q2 : 선형보간
		for( int i = 0 ; i < quadSize ; ++i )
		{
			float t = (float)i / (float)quadSize;
			brushLine[0].p		= ( 1.0f - t )*q1 + (t)*q2;
			brushLine[0].p.y	= GetHeight( brushLine[0].p.x , brushLine[0].p.z ) + 0.5f;

			t = (float)i / (float)quadSize;
			brushLine[1].p		= ( 1.0f - t )*q1 + (t)*q2;
			brushLine[1].p.y	= GetHeight( brushLine[1].p.x , brushLine[1].p.z ) + 0.5f;

			m_pDevice->SetFVF( BRUSH_VERTEX::FVF );
			m_pDevice->DrawPrimitiveUP( D3DPT_LINELIST , 1 , brushLine , sizeof( BRUSH_VERTEX ) );
		}
	}
}

inline void Terrain::_SearchTileInPickArea( POINT pos[3] , vector<int>* pVecTile )
{
	// 브러쉬 크기보다 1타일씩 더 크게 설정한다.
	// 이유는 좀 넉넉하게 계산하기 위해서
	float tileSize	= static_cast<float>( m_MapInfo.m_nCells * m_MapInfo.m_nCellSpacing );
	float maxPos	= static_cast<float>( m_MapInfo.m_nTiles * m_MapInfo.m_nCells * m_MapInfo.m_nCellSpacing );

	D3DXVECTOR3 left	= m_vPickPos;
	D3DXVECTOR3 right	= m_vPickPos;
	D3DXVECTOR3 up		= m_vPickPos;
	D3DXVECTOR3 down	= m_vPickPos;

	left.x	= m_vPickPos.x - ( m_fBrushOutSize * m_MapInfo.m_nCellSpacing ) - tileSize;
	right.x = m_vPickPos.x + ( m_fBrushOutSize * m_MapInfo.m_nCellSpacing ) + tileSize;
	up.z	= m_vPickPos.z + ( m_fBrushOutSize * m_MapInfo.m_nCellSpacing ) + tileSize;
	down.z	= m_vPickPos.z - ( m_fBrushOutSize * m_MapInfo.m_nCellSpacing ) - tileSize;

	if( left.x <= 0.0f )		{ left.x = 1.0f; }
	if( right.x >= maxPos )		{ right.x = maxPos - 1.0f; }
	if( up.z >= 0.0f )			{ up.z = -1.0f; }
	if( down.z <= -( maxPos ) ) { down.z = -(maxPos)+1.0f; }
	
	int curTile		= _SearchPositionInTiles( m_vPickPos );
	int leftTile	= _SearchPositionInTiles( left );
	int rightTile	= _SearchPositionInTiles( right );
	int upTile		= _SearchPositionInTiles( up );
	int downTile	= _SearchPositionInTiles( down );

	int tile = m_MapInfo.m_nTiles;
	int xStartTile = upTile - ( curTile - leftTile );
	int xTileSize = xStartTile + rightTile - leftTile;
	int zTileSize = ( downTile / tile ) - ( upTile / tile );
	int zEndTile = xStartTile + ( ( downTile / tile ) - ( upTile / tile ) ) * tile;

	// 타일 번호를 저장한다.
	// 전체 지형을 타일별로 나눠서 저장하기 위해
	int tileIndex = 0;
	for( int z = 0; z <= zTileSize ; ++z )
	{
		for( int x = xStartTile; x < xTileSize; ++x )
		{
			tileIndex = x + ( z * tile );
			pVecTile->push_back( tileIndex );
		}
	}

	// 마우스의 위치를 중심으로 8방향 중에 맨 꼭지점의 4방향의 버텍스 좌표를 저장한다.
	POINT edge[3];
	edge[0].x = m_pTileInfo[xStartTile].m_nEdge[0];
	edge[0].y = m_pTileInfo[xStartTile].m_nEdge[0];

	edge[1].x = m_pTileInfo[xTileSize].m_nEdge[1];
	edge[1].y = m_pTileInfo[xTileSize].m_nEdge[1];

	edge[2].x = m_pTileInfo[xTileSize].m_nEdge[2];
	edge[2].y = m_pTileInfo[xTileSize].m_nEdge[2];

	for( int i = 0 ; i < 3 ; ++i )
	{
		pos[i].x = edge[i].x % m_MapInfo.m_nTotalVertices;
		pos[i].y = edge[i].y / m_MapInfo.m_nTotalVertices;
	}


}

inline int Terrain::_SearchPositionInTiles( D3DXVECTOR3 pos , EdgeInfo* pTile /*= NULL */ )
{
	// 좌표를 벗어나면 타일 번호를 구하지 않는다.
	if( pos.x < 0 || pos.z > 0 )
		return -1;

	// 현재 위치의 타일 번호를 구한다.
	int x =  static_cast<int>( pos.x * m_MapInfo.m_nTiles ) / m_MapInfo.m_nTotalMapSize;
	int z = -static_cast<int>( pos.z * m_MapInfo.m_nTiles ) / m_MapInfo.m_nTotalMapSize;

	int tileIdx = ( z * m_MapInfo.m_nTiles + x );
	
	// 타일의 모서리 끝점들의 좌표를 알고 싶을때..
	if( pTile != NULL )
	{
		if( 0 <= tileIdx && tileIdx < m_MapInfo.m_nTotalTiles )
		{
			int TL = 0;
			int TR = m_MapInfo.m_nCells;
			int BL = ( m_MapInfo.m_nCells + 1 ) * ( ( m_MapInfo.m_nCells + 1 ) - 1 );
			int BR = ( m_MapInfo.m_nCells + 1 ) * ( m_MapInfo.m_nCells + 1 ) - 1;

			pTile->TL = m_pTileInfo[tileIdx].m_pVertex[TL].p;
			pTile->TR = m_pTileInfo[tileIdx].m_pVertex[TR].p;
			pTile->BL = m_pTileInfo[tileIdx].m_pVertex[BL].p;
			pTile->BR = m_pTileInfo[tileIdx].m_pVertex[BR].p;
		}
	}

	return tileIdx;
}

inline void Terrain::_TexturePaintInBrushArea()
{
	float maxPos = static_cast<float>( m_MapInfo.m_nTiles * m_MapInfo.m_nCells * m_MapInfo.m_nCellSpacing );

	D3DXVECTOR3 left	= m_vPickPos;
	D3DXVECTOR3 right	= m_vPickPos;
	D3DXVECTOR3 up		= m_vPickPos;
	D3DXVECTOR3 down	= m_vPickPos;

	left.x	= m_vPickPos.x - ( m_fBrushOutSize * m_MapInfo.m_nCellSpacing );
	right.x = m_vPickPos.x + ( m_fBrushOutSize * m_MapInfo.m_nCellSpacing );
	up.z	= m_vPickPos.z + ( m_fBrushOutSize * m_MapInfo.m_nCellSpacing );
	down.z	= m_vPickPos.z - ( m_fBrushOutSize * m_MapInfo.m_nCellSpacing );

	if( left.x <= 0.0f )		{ left.x = 1.0f; }
	if( right.x >= maxPos )		{ right.x = maxPos - 1.0f; }
	if( up.z >= 0.0f )			{ up.z = -1.0f; }
	if( down.z <= -( maxPos ) ) { down.z = -(maxPos)+1.0f; }

	int curTile		= _SearchPositionInTiles( m_vPickPos );
	int leftTile	= _SearchPositionInTiles( left );
	int rightTile	= _SearchPositionInTiles( right );
	int upTile		= _SearchPositionInTiles( up );
	int downTile	= _SearchPositionInTiles( down );

	int xStartTile	= upTile - ( curTile - leftTile );
	int xTileSize	= xStartTile + rightTile - leftTile;
	int zTileSize	= ( downTile / m_MapInfo.m_nTiles ) - ( upTile / m_MapInfo.m_nTiles );

	int tileIndex = 0;
	for( int z = 0; z <= zTileSize ; ++z )
	{
		for( int x = xStartTile; x < xTileSize; ++x )
		{
			// 브러쉬 영역의 타일 번호를 구한다.
			tileIndex = x + ( z * m_MapInfo.m_nTiles );

			// 현재 저장된 타일 번호와 비교해서 존재하면 저장하지 않는다.
			bool equal = false;
			for( int i = 0 ; i < (int)m_vecSplattingTile.size() ; ++i )
			{
				if( tileIndex == m_vecSplattingTile[i] )
				{
					equal = true;
					break;
				}
			}

			// 새로운 타일 번호일 경우 저장
			if( !equal )
				m_vecSplattingTile.push_back( tileIndex );
		}
	}
}

inline void Terrain::_GetLODLevel( D3DXVECTOR3 look , int index )
{
	// 카메라 Look 위치와 타일의 중심점과의 거리를 구한다.
	// 정사각현으로 만들기 위해 x축과 z축을 비교한다.
	// 그냥 중점과의 거리를 구하면 원모양으로 LOD 레벨이 정해지는데 
	// 그럴경우 크랙이 16가지가 필요하다.
	// 정사각형으로 만들면 크랙이 4방향만 필요하다.
	D3DXVECTOR3 distX( 0.0f , 0.0f , 0.0f );
	distX.x = look.x - m_pHeightMap[m_pTileInfo[index].m_nCenter].p.x;
	float LenX = D3DXVec3Length( &distX );

	D3DXVECTOR3 distZ( 0.0f , 0.0f , 0.0f );
	distZ.z = look.z - m_pHeightMap[m_pTileInfo[index].m_nCenter].p.z;
	float LenZ = D3DXVec3Length( &distZ );

	float len = 0.0f;
	if( LenX >= LenZ )
		len = LenX;
	else
		len = LenZ;

	for( int i = 0 ; i < m_nMaxLODLevel; ++i )
	{
		if( (float)( i * m_nLODLevelSize ) <= len )
			m_pTileInfo[index].m_nLODLevel = i;
		else
			break;
	}

}


inline void Terrain::_SetupCrackIndex( int index )
{
	int current = m_pTileInfo[index].m_nLODLevel;
	int comp[4] = { 0 , };
	int tiles	= m_MapInfo.m_nTiles;

	// left , up , right , down순으로 검사
	comp[0] = ( index % tiles > 0 )			? m_pTileInfo[index - 1].m_nLODLevel		: m_pTileInfo[index].m_nLODLevel;
	comp[1] = ( index / tiles > 0 )			? m_pTileInfo[index - tiles].m_nLODLevel	: m_pTileInfo[index].m_nLODLevel;
	comp[2] = ( index % tiles < tiles - 1 ) ? m_pTileInfo[index + 1].m_nLODLevel		: m_pTileInfo[index].m_nLODLevel;
	comp[3] = ( index / tiles < tiles - 1 ) ? m_pTileInfo[index + tiles].m_nLODLevel	: m_pTileInfo[index].m_nLODLevel;

	// 각 전체 셀에 LOD레벨에 대한 2의 제곱을 나눈 후 아래에서 삼각형 개수를 설정한다.
	int devide = (int)pow( 2 , m_pTileInfo[index].m_nLODLevel );
	int triNum = m_MapInfo.m_nCells / devide;

	int count = 0;
	for( int i = 0 ; i < 4; ++i )
	{
		// 3 2 6
		// 1 0 4
		// 9 8 12
		// 위의 숫자처럼 left랑 비교해서 레벨이 틀릴경우 count = 1
		// left , up , 2개면 1+2=3이 된다. up , right , 2개면 2+4=6 어떤수도 중복되지 않는다.
		// 즉 2개가 틀릴 경우는 그 위치에 해당하는 대각선 숫자가 나오게 된다.

		// left , up , right , down을 비교하면서 현재레벨이 주변레벨보다 클 경우만 적용한다.
		// 이유는 레벨이 클 수록 세부 레벨로 나눠야 하기 때문이다.
		if( current <= comp[i] )		// 현재 레벨이 주변 레벨보다 작을 경우
			count += 0;
		else if( current > comp[i] )	// 현재 레벨이 주변 레벨보다 클 경우
			count += (int)pow( 2 , 1 );
	}

	// 0 , 3 , 6 , 9 , 12는 낮은 레벨이여서
	if( count % 3 == 0 )
		count = 0;

	switch( count )
	{
	case 0:
		m_pTileInfo[index].m_nLODCrackIndex = LOD_BASIC;
		m_pTileInfo[index].m_nTriangleNum = triNum * triNum * 2;
		break;
	case 1:
		m_pTileInfo[index].m_nLODCrackIndex = LOD_LEFT;
		m_pTileInfo[index].m_nTriangleNum = triNum * triNum * 2 + (triNum * 3);
		break;
	case 2:
		m_pTileInfo[index].m_nLODCrackIndex = LOD_TOP;
		m_pTileInfo[index].m_nTriangleNum = triNum * triNum * 2 + ( triNum * 3 );
		break;
	case 3:
		m_pTileInfo[index].m_nLODCrackIndex = LOD_RIGHT;
		m_pTileInfo[index].m_nTriangleNum = triNum * triNum * 2 + ( triNum * 3 );
		break;
	case 4:
		m_pTileInfo[index].m_nLODCrackIndex = LOD_BOTTOM;
		m_pTileInfo[index].m_nTriangleNum = triNum * triNum * 2 + ( triNum * 3 );
		break;
	default:
		break;
	}
}

inline int Terrain::_SearchPositionInMap( D3DXVECTOR3 pos , EdgeInfo* pMap )
{
	// 좌표를 벗어나면 맵번호를 구하지 않는다.
	if( pos.x < 0 || pos.z > 0 )
		return -1;

	int x = (int)( pos.x * m_MapInfo.m_nTiles * m_MapInfo.m_nCells ) / m_MapInfo.m_nTotalMapSize;
	int z = -(int)( pos.z * m_MapInfo.m_nTiles * m_MapInfo.m_nCells ) / m_MapInfo.m_nTotalMapSize;

	int mapIndex = ( z * m_MapInfo.m_nTotalVertices + x );
	int limit = ( m_MapInfo.m_nTotalVertices + mapIndex + 1 );

	// 가장자리 아래쪽 버텍스가 총 버텍스 갯수를 넘지 않는다.
	if( 0 <= limit && limit < m_MapInfo.m_nTotalVertices*m_MapInfo.m_nTotalVertices )
	{
		pMap->TL = m_pHeightMap[mapIndex].p;
		pMap->TR = m_pHeightMap[mapIndex + 1].p;
		pMap->BL = m_pHeightMap[m_MapInfo.m_nTotalVertices + mapIndex].p;
		pMap->BR = m_pHeightMap[m_MapInfo.m_nTotalVertices + mapIndex + 1].p;
	}

	return mapIndex;
}


inline int Terrain::_NearPositionInCells( QuadEdgeInfo* pPart , D3DXVECTOR3 pos , EdgeInfo* pCell , int cellIdx )
{
	// 좌표를 벗어나면 가까운 셀 번호를 구하지 않는다.
	if( pos.x < 0 || pos.z > 0 )
		return -1;

	//------------------------------------------------------------------------
	// 현재 위치를 중심으로 주변 셀 사각형 점들중에 가까운 점으로 이동한다.
	//------------------------------------------------------------------------
	int nearCellIdx = 0;
	if( ( pos.x - pCell->TL.x ) <= ( m_MapInfo.m_nCellSpacing / 2 ) )
	{
		// 위쪽 (LEFT TOP)
		if( ( pos.z - pCell->TL.x ) <= ( m_MapInfo.m_nCellSpacing / 2 ) )
			nearCellIdx = cellIdx;
		// 아래쪽 (LEFT BOTTOM)
		else
			nearCellIdx = m_MapInfo.m_nTotalVertices + cellIdx;
	}
	else
	{
		// 위쪽(RIGHT TOP)
		if( ( pos.z - pCell->TL.z ) >= -( m_MapInfo.m_nCellSpacing / 2 ) )
			nearCellIdx = cellIdx + 1;
		// 아래쪽 (RIGHT BOTTOM)
		else
			nearCellIdx = m_MapInfo.m_nTotalVertices + cellIdx + 1;
	}

	//---------------------------------------------------------------------------------
	// 1차원 배열로 되었어서 오른쪽 끝 다음위치가 왼쪽 처음위치가 된다.
	// 0-1----------------7-8		( 0->10 , 1->10 , 7->16 , 8->16)
	// 9-10               16-17		( 9->10 , 17->16)
	// . .                 . .
	// . .                 . .
	// 63-64              70-71		( 63->64 , 71->70)
	// 72-73--------------79-80		( 72->64 , 73->64 , 79->70  , 80->70 )
	// 이런 식으로 점을 이동해서 이동한 점을 중심으로 사각형 4개가 만드어 지게 한다.
	//---------------------------------------------------------------------------------
	// 0번 cell
	if( nearCellIdx == 0 )
	{
		nearCellIdx = m_MapInfo.m_nTotalVertices + 1;
	}	
	// 1 ~ 가로끝 - 1
	else if( 1 <= nearCellIdx && nearCellIdx < m_MapInfo.m_nTotalVertices - 1 )
	{
		nearCellIdx += m_MapInfo.m_nTotalVertices;
	}
	// 가로끝
	else if( nearCellIdx == m_MapInfo.m_nTotalVertices - 1 )
	{
		nearCellIdx += m_MapInfo.m_nTotalVertices - 1;
	}
	// 왼쪽 세로 ~ 왼쪽 세로끝 - 1
	else if( nearCellIdx % m_MapInfo.m_nTotalVertices == 0 )
	{
		nearCellIdx += 1;
	}
	// 오른쪽 세로 ~ 오른쪽 세로끝 -1
	else if( nearCellIdx % m_MapInfo.m_nTotalVertices == m_MapInfo.m_nTotalVertices - 1 )
	{
		nearCellIdx -= 1;
	}
	// 왼쪽 아래끝
	else if( nearCellIdx == m_MapInfo.m_nTotalVertices * ( m_MapInfo.m_nTotalVertices - 1 ) )
	{
		nearCellIdx -= m_MapInfo.m_nTotalVertices + 1;
	}
	// 오른쪽 아래끝
	else if( nearCellIdx == m_MapInfo.m_nTotalVertices * m_MapInfo.m_nTotalVertices - 1 )
	{
		nearCellIdx -= m_MapInfo.m_nTotalVertices - 1;
	}
	// 아래쪽
	else if( nearCellIdx >= m_MapInfo.m_nTotalVertices * ( m_MapInfo.m_nTotalVertices - 1 ) &&
			 nearCellIdx <  m_MapInfo.m_nTotalVertices * ( m_MapInfo.m_nTotalVertices - 1 ) )
	{
		nearCellIdx -= m_MapInfo.m_nTotalVertices;
	}

	// 위에서 구한 점을 중심으로 4개의 사각형을 만든다.
	// m_MapInfo.m_nTotalVertices = cells * tiles + 1
	int leftTop		= nearCellIdx - m_MapInfo.m_nTotalVertices - 1;
	int top			= nearCellIdx - m_MapInfo.m_nTotalVertices;
	int rightTop	= nearCellIdx - m_MapInfo.m_nTotalVertices + 1;
	int left		= nearCellIdx - 1;
	int center		= nearCellIdx;
	int right		= nearCellIdx + 1;
	int leftBottom	= nearCellIdx + m_MapInfo.m_nTotalVertices - 1;
	int bottom		= nearCellIdx + m_MapInfo.m_nTotalVertices;
	int rightBottom = nearCellIdx + m_MapInfo.m_nTotalVertices + 1;

	// 가장 아래쪽이 버텍스의 총합계를 넘지 않게 한다.
	if( rightBottom < m_MapInfo.m_nTotalVertices * m_MapInfo.m_nTotalVertices )
	{
		pPart->m_part[0].TL = m_pHeightMap[leftTop].p;
		pPart->m_part[0].TR = m_pHeightMap[top].p;
		pPart->m_part[0].BL = m_pHeightMap[left].p;
		pPart->m_part[0].BR = m_pHeightMap[center].p;

		pPart->m_part[1].TL = m_pHeightMap[top].p;
		pPart->m_part[1].TR = m_pHeightMap[rightTop].p;
		pPart->m_part[1].BL = m_pHeightMap[center].p;
		pPart->m_part[1].BR = m_pHeightMap[right].p;

		pPart->m_part[2].TL = m_pHeightMap[left].p;
		pPart->m_part[2].TR = m_pHeightMap[center].p;
		pPart->m_part[2].BL = m_pHeightMap[leftBottom].p;
		pPart->m_part[2].BR = m_pHeightMap[bottom].p;

		pPart->m_part[3].TL = m_pHeightMap[center].p;
		pPart->m_part[3].TR = m_pHeightMap[right].p;
		pPart->m_part[3].BL = m_pHeightMap[bottom].p;
		pPart->m_part[3].BR = m_pHeightMap[rightBottom].p;
	}

	return nearCellIdx;
}

extern Terrain* g_pTerrain;