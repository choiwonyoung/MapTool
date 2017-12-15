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
// �귯��
struct BRUSH_VERTEX
{
	D3DXVECTOR3 p;
	DWORD color;
	enum { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE };
};

// ����
struct TERRAIN_VERTEX
{
	D3DXVECTOR3 p;
	D3DXVECTOR3 n;
	D3DXVECTOR3 t1;		// Ÿ�� uv
	D3DXVECTOR3 t2;		// Alpha Texture uv
};
const DWORD TERRAIN_FVF = { D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 };

// �� ����
// ��δ� �����ϰ� �ҷ���
struct MapInfo
{
	int m_nTiles;			// ���� Ÿ�� ����(���� or ����)
	int m_nTotalTiles;		// ��ü Ÿ�� ����(���� * ����)
	int m_nTotalVertices;	// ��ü ���ؽ� ����(���� or ����)
	int m_nTotalMapSize;	// ��ü �� ũ��( Ÿ�� * �� * �Ÿ� )
	int m_nAlphaTexSize;	// ���ĸ� ũ��

	int m_nCells;			// 1 Ÿ�� �ȿ� �ִ� ������
	int m_nCellSpacing;		// 1 Ÿ�� �ȿ� 1�� ���� ũ��(�Ÿ�)
	int m_nVertices;		// 1 Ÿ�� �ȿ� �ִ� ���ؽ� �� ����
	int m_nIndices;			// 1 Ÿ�� �ȿ� �ִ� �ε��� �� ����
	int m_nTriangles;		// 1 Ÿ�� �ȿ� �ִ� �ﰢ�� �� ����
	int m_nTextureNum;		// ��ü �ؽ��� ����
};

// Ÿ�� ����
struct TileInfo
{
	TERRAIN_VERTEX*			m_pVertex;			// Ÿ�� �� ����(��ġ , ���� , �ؽ���)
	LPDIRECT3DVERTEXBUFFER9 m_pVB;				// TERRAIN_VERTEX* �� ���ؽ� ���۷� ����
	int						m_nEdge[4];			// Ÿ���� �� �𼭸�( TL , TR , BL , BR )�� �ε���( ��ü ���� �ε��� )
	int						m_nCenter;			// Ÿ���� ��� �ε���(��ü �ε��� ��)
	int						m_nLODLevel;		// LOD ����
	int						m_nLODCrackIndex;	// LOD ������ ����
	int						m_nTriangleNum;		// Ÿ���� �ﰢ�� ����

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

// �簢���� �����ϴ� 4��
struct EdgeInfo
{
	D3DXVECTOR3 TL;		// Top Left
	D3DXVECTOR3 TR;		// Top Right
	D3DXVECTOR3 BL;		// Bottom Left
	D3DXVECTOR3 BR;		// Bottom Right
};

// 4���� �簢��
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

	// ���� �ؽ��� ���� �Լ�
	HRESULT CreateAlphaTexture( int index );
	HRESULT ChangeAlphaTexture( int dest , int src );
	void	SaveAlphaValue( int index , BYTE* pRead );
	void	LoadAlphaValue( int index , BYTE* pRead );
	void	SaveAlphaTexture( int index , const char* pFileName );

	//���� ���� (�ҷ��ö�, ���� �� ��� , �븻���)
	void	LoadTerrain();	
	void	ComputeNormal( POINT pos[3] );	
	float	GetHeight( float x , float z );

	// �׸��� �Լ�
	void	DrawTerrain();
	void	BrushDraw( int count , float outSize , float inSize , D3DXCOLOR outColor , D3DXCOLOR inColor , bool bDrawInBrush );

	// ���� �� �ؽ��� ����
	void	EditHeightInCircle();
	void	EditHeightInRectangle();
	void	MakeAlphaTexture( int index , int brushType );

	// LOD ����
	void	SetupLOD( D3DXVECTOR3 look );

	// ����ȭ ��ŷ
	void PickTerrain( Pick* pPick );

	Frustum* GetFrustum() const { return m_pFrustum; }

private:
	void _SetUpMapInfo( int tiles , int cells , int cellSpacing , int nTotalTex );
	void _SetUpHeightMap();
	void _DrawBaseTile( int index );
	void _DrawSplattingTile( int index );
	void _DrawBoundaryLine( int index );

	
	// �귯��
	inline void _BrushCircleDraw( int count , float size , D3DXCOLOR color );
	inline void _BrushRectangleDraw( int count , float size , D3DXCOLOR color );

	
	// ����ȭ�� ���� �Լ�( ���� ����� ���� , �귯�� ������ �׸��� , �귯�� ���� Ȯ�� ã��(�븻 ����)
	inline float _Lerp( float a , float b , float t );
	inline void _TexturePaintInBrushArea();
	inline void _SearchTileInPickArea( POINT pos[3] , vector<int>* pVecTile );

	// ��ŷ ����ȭ�� ���� �Լ�( ���̴� Ÿ�� �˻� , Ÿ�� ��ȣ ã�� , �� ��ȭ ã�� , ����� ������ ã�� )
	inline int _SearchPositionInTiles( D3DXVECTOR3 pos , EdgeInfo* pTile = NULL );
	inline int _SearchPositionInMap( D3DXVECTOR3 pos , EdgeInfo* pMap );
	inline int _NearPositionInCells( QuadEdgeInfo* pPart , D3DXVECTOR3 pos , EdgeInfo* pCell , int cellIdx );

	// ī�޶� Look ��ġ�� ���� Ÿ�ϰ��� �Ÿ����� �������� LOD ���� ���Ѵ�.
	inline void _GetLODLevel( D3DXVECTOR3 look , int index );
	inline void _SetupCrackIndex( int index );

public:
	
	BOOL					m_bFrustumLock;			

	// �� ����
	MapInfo					m_MapInfo;				// �� ����
	TileInfo*				m_pTileInfo;			// Ÿ�� ����
	TERRAIN_VERTEX*			m_pHeightMap;			// ��ü ���� ����

	// LOD
	BOOL					m_bApplyLOD;			// LOD ���� ����
	int						m_nMaxLODLevel;			// LOD Max Level
	int						m_nLODLevelSize;		// LOD Level Size

	// Texture
	int						m_nTextureIndex[4];		// 0(�⺻) ~ 3�� �ؽ��� ��ȣ(�ؽ��� ����Ʈ�� ��ȣ)
	int						m_nCurTextureIdx;		// ���� �ؽ��� �� ��ȣ(0 ~ 3��)
	int						m_nSelectTextureNum;	// ���õ� �ؽ��� ����
	int						m_nTextureSplatting;	// ���÷��� �׸��� or �����

	// �귯���� ĥ�� ���� ������ �ϱ� ����
	vector<int>				m_vecSplattingTile;

	// �ؽ��� ��ü ��� ����
	string*					m_pStrTextureName;		// �ؽ��� ��ü �̸�

	// ������ �Ǵ� Ÿ�� ��ȣ
	vector<int>				m_vecVisibleTile;		// ���̴� Ÿ�� ��ȣ��

	// �귯�� �������� 1Ÿ�� �� ū Ÿ�� ��ȣ��(���� �ø��� �귯�� ������ �����ϱ�)
	vector<int>				m_vecBrushAreaTile;		// �귯�� ���� Ÿ��

	// ������ �ɼ�
	BOOL					m_bWireFrame;			// ���̾� ������ ���
	BOOL					m_bBoundaryLine;		// ��輱 �׸���
	BOOL					m_bFog;					// �Ȱ� ���
	BOOL					m_bLight;				// ����Ʈ ���
	int						m_nUpDownMode;			// �ø� , ���� , ��� , ����
	int						m_nHeightRate;			// ���� ����

	// �귯��
	int						m_nBrushType;			// �� , �簢��
	float					m_fBrushOutSize;		// �ٱ�����(�ε巴��)
	float					m_fBrushInSize;			// ���ʶ���(�����ϰ�)

	// ��ŷ
	D3DXVECTOR3				m_vPickPos;				// ���콺 ��ŷ ��ǥ
	BOOL					m_bPickSuccess;			// ��ŷ ���� ����


private:
	// ��������
	Frustum*				m_pFrustum;				// ��������

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
	float brushSize = size * m_MapInfo.m_nCellSpacing;			// ������ * ��ũ��

	BRUSH_VERTEX	brushLine[2];					// �귯�� ���ؽ�[2](2���� �������� ���� �׸��� ��)
	D3DXVECTOR3		curPos( 1.0f , 0.0f , 0.0f );	// ���� ��ġ( �ʱ� ��ġ�� �ƹ����̳� 1.0f )
	D3DXVECTOR3		newPos;							// �ٿ� ��ġ( �� ��ġ�� �մ´�.) LINELIST�� Draw
	D3DXMATRIX		matRot;							// ���� ��ġ�� ���� ȸ�� ���

	// �ʱ� ��ġ[1]�� ( 1.0f , 0.0f , 0.0f ) * �귯�� ũ�� + ��ŷ ���콺 ��ġ
	brushLine[1].p		= curPos *brushSize + m_vPickPos;
	brushLine[1].p.y	= GetHeight( brushLine[1].p.x , brushLine[1].p.z ) + 0.5f;
	brushLine[1].color	= brushLine[0].color = color;

	// radian�� 0�� ���� �ʱ� ���� 1���� ����
	for( int i = 1; i < count + 1 ; ++i )
	{
		// ó�� ������ ���� ��ġ[1]�� [0]���� �ְ�
		brushLine[0].p = brushLine[1].p;

		// ó�� ��ġ�� �߽����� i * radian ��ŭ ȸ����
		// ���ο� ��ġ�� �̵��� ���Ϳ� �ٽ� ó��[1] ��ġ�� �ִ´�.
		D3DXMatrixRotationY( &matRot , i * radian );
		D3DXVec3TransformCoord( &newPos , &curPos , &matRot );
		D3DXVec3Normalize( &newPos , &newPos );

		// �ٽ� ���ο� ��ġ[1]�� ���Ѵ�.
		brushLine[1].p = newPos * brushSize + m_vPickPos;
		brushLine[1].p.y = GetHeight( brushLine[1].p.x , brushLine[1].p.z ) + 0.5f;

		// ���ؽ� ���۸� �������� �ʰ� �׸���( DrawPrimitiveUP )
		m_pDevice->SetFVF( BRUSH_VERTEX::FVF );
		// DrawPrimitiveUP�� vertex buffer�� ������ �ʰ� �׸� �� �ִ�.
		m_pDevice->DrawPrimitiveUP( D3DPT_LINELIST , 1 , brushLine , sizeof( BRUSH_VERTEX ) );
	}

}

inline void Terrain::_BrushRectangleDraw( int count , float size , D3DXCOLOR color )
{
	if( count <= 4 )
		count = 4;

	int quadSize = count / 4;
	float brushSize = size * m_MapInfo.m_nCellSpacing;			// ������ * �� ũ��

	D3DXVECTOR3 vTL( -1.0f , 0.0f ,  1.0f );
	D3DXVECTOR3 vTR(  1.0f , 0.0f ,  1.0f );
	D3DXVECTOR3 vBL( -1.0f , 0.0f , -1.0f );
	D3DXVECTOR3 vBR(  1.0f , 0.0f , -1.0f );
	BRUSH_VERTEX brushLine[2];									// �귯�� ���ؽ�[2] (2���� �������� �簢���� �׸���)
	brushLine[0].color = brushLine[1].color = color;

	D3DXVECTOR3 q1( 0.0f , 0.0f , 0.0f );
	D3DXVECTOR3 q2( 0.0f , 0.0f , 0.0f );

	// 4������ �������� �̸� ���� �� �� ���⿡ ���� ������������ ������ �׸���.
	for( int quad = 0; quad < 4 ; ++quad )
	{
		switch( quad )
		{
		case 0: // TL  ->  TR (����)
			q1 = vTL * brushSize + m_vPickPos;
			q2 = vTR * brushSize + m_vPickPos;
			break;

		case 1: // TR  ->  BR (��������)
			q1 = vTR * brushSize + m_vPickPos;
			q2 = vBR * brushSize + m_vPickPos;
			break;

		case 2: // BL  ->  BR (�Ʒ���)
			q1 = vBL * brushSize + m_vPickPos;
			q2 = vBR * brushSize + m_vPickPos;
			break;

		case 3: // TL  ->  TR (������)
			q1 = vTL * brushSize + m_vPickPos;
			q2 = vBL * brushSize + m_vPickPos;
			break;
		}

		// count �� 4������ ������ ���� ��ŭ ������ �����.
		// q(t) = ( 1 - t )*q1 + t* q2 : ��������
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
	// �귯�� ũ�⺸�� 1Ÿ�Ͼ� �� ũ�� �����Ѵ�.
	// ������ �� �˳��ϰ� ����ϱ� ���ؼ�
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

	// Ÿ�� ��ȣ�� �����Ѵ�.
	// ��ü ������ Ÿ�Ϻ��� ������ �����ϱ� ����
	int tileIndex = 0;
	for( int z = 0; z <= zTileSize ; ++z )
	{
		for( int x = xStartTile; x < xTileSize; ++x )
		{
			tileIndex = x + ( z * tile );
			pVecTile->push_back( tileIndex );
		}
	}

	// ���콺�� ��ġ�� �߽����� 8���� �߿� �� �������� 4������ ���ؽ� ��ǥ�� �����Ѵ�.
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
	// ��ǥ�� ����� Ÿ�� ��ȣ�� ������ �ʴ´�.
	if( pos.x < 0 || pos.z > 0 )
		return -1;

	// ���� ��ġ�� Ÿ�� ��ȣ�� ���Ѵ�.
	int x =  static_cast<int>( pos.x * m_MapInfo.m_nTiles ) / m_MapInfo.m_nTotalMapSize;
	int z = -static_cast<int>( pos.z * m_MapInfo.m_nTiles ) / m_MapInfo.m_nTotalMapSize;

	int tileIdx = ( z * m_MapInfo.m_nTiles + x );
	
	// Ÿ���� �𼭸� �������� ��ǥ�� �˰� ������..
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
			// �귯�� ������ Ÿ�� ��ȣ�� ���Ѵ�.
			tileIndex = x + ( z * m_MapInfo.m_nTiles );

			// ���� ����� Ÿ�� ��ȣ�� ���ؼ� �����ϸ� �������� �ʴ´�.
			bool equal = false;
			for( int i = 0 ; i < (int)m_vecSplattingTile.size() ; ++i )
			{
				if( tileIndex == m_vecSplattingTile[i] )
				{
					equal = true;
					break;
				}
			}

			// ���ο� Ÿ�� ��ȣ�� ��� ����
			if( !equal )
				m_vecSplattingTile.push_back( tileIndex );
		}
	}
}

inline void Terrain::_GetLODLevel( D3DXVECTOR3 look , int index )
{
	// ī�޶� Look ��ġ�� Ÿ���� �߽������� �Ÿ��� ���Ѵ�.
	// ���簢������ ����� ���� x��� z���� ���Ѵ�.
	// �׳� �������� �Ÿ��� ���ϸ� ��������� LOD ������ �������µ� 
	// �׷���� ũ���� 16������ �ʿ��ϴ�.
	// ���簢������ ����� ũ���� 4���⸸ �ʿ��ϴ�.
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

	// left , up , right , down������ �˻�
	comp[0] = ( index % tiles > 0 )			? m_pTileInfo[index - 1].m_nLODLevel		: m_pTileInfo[index].m_nLODLevel;
	comp[1] = ( index / tiles > 0 )			? m_pTileInfo[index - tiles].m_nLODLevel	: m_pTileInfo[index].m_nLODLevel;
	comp[2] = ( index % tiles < tiles - 1 ) ? m_pTileInfo[index + 1].m_nLODLevel		: m_pTileInfo[index].m_nLODLevel;
	comp[3] = ( index / tiles < tiles - 1 ) ? m_pTileInfo[index + tiles].m_nLODLevel	: m_pTileInfo[index].m_nLODLevel;

	// �� ��ü ���� LOD������ ���� 2�� ������ ���� �� �Ʒ����� �ﰢ�� ������ �����Ѵ�.
	int devide = (int)pow( 2 , m_pTileInfo[index].m_nLODLevel );
	int triNum = m_MapInfo.m_nCells / devide;

	int count = 0;
	for( int i = 0 ; i < 4; ++i )
	{
		// 3 2 6
		// 1 0 4
		// 9 8 12
		// ���� ����ó�� left�� ���ؼ� ������ Ʋ����� count = 1
		// left , up , 2���� 1+2=3�� �ȴ�. up , right , 2���� 2+4=6 ����� �ߺ����� �ʴ´�.
		// �� 2���� Ʋ�� ���� �� ��ġ�� �ش��ϴ� �밢�� ���ڰ� ������ �ȴ�.

		// left , up , right , down�� ���ϸ鼭 ���緹���� �ֺ��������� Ŭ ��츸 �����Ѵ�.
		// ������ ������ Ŭ ���� ���� ������ ������ �ϱ� �����̴�.
		if( current <= comp[i] )		// ���� ������ �ֺ� �������� ���� ���
			count += 0;
		else if( current > comp[i] )	// ���� ������ �ֺ� �������� Ŭ ���
			count += (int)pow( 2 , 1 );
	}

	// 0 , 3 , 6 , 9 , 12�� ���� �����̿���
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
	// ��ǥ�� ����� �ʹ�ȣ�� ������ �ʴ´�.
	if( pos.x < 0 || pos.z > 0 )
		return -1;

	int x = (int)( pos.x * m_MapInfo.m_nTiles * m_MapInfo.m_nCells ) / m_MapInfo.m_nTotalMapSize;
	int z = -(int)( pos.z * m_MapInfo.m_nTiles * m_MapInfo.m_nCells ) / m_MapInfo.m_nTotalMapSize;

	int mapIndex = ( z * m_MapInfo.m_nTotalVertices + x );
	int limit = ( m_MapInfo.m_nTotalVertices + mapIndex + 1 );

	// �����ڸ� �Ʒ��� ���ؽ��� �� ���ؽ� ������ ���� �ʴ´�.
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
	// ��ǥ�� ����� ����� �� ��ȣ�� ������ �ʴ´�.
	if( pos.x < 0 || pos.z > 0 )
		return -1;

	//------------------------------------------------------------------------
	// ���� ��ġ�� �߽����� �ֺ� �� �簢�� �����߿� ����� ������ �̵��Ѵ�.
	//------------------------------------------------------------------------
	int nearCellIdx = 0;
	if( ( pos.x - pCell->TL.x ) <= ( m_MapInfo.m_nCellSpacing / 2 ) )
	{
		// ���� (LEFT TOP)
		if( ( pos.z - pCell->TL.x ) <= ( m_MapInfo.m_nCellSpacing / 2 ) )
			nearCellIdx = cellIdx;
		// �Ʒ��� (LEFT BOTTOM)
		else
			nearCellIdx = m_MapInfo.m_nTotalVertices + cellIdx;
	}
	else
	{
		// ����(RIGHT TOP)
		if( ( pos.z - pCell->TL.z ) >= -( m_MapInfo.m_nCellSpacing / 2 ) )
			nearCellIdx = cellIdx + 1;
		// �Ʒ��� (RIGHT BOTTOM)
		else
			nearCellIdx = m_MapInfo.m_nTotalVertices + cellIdx + 1;
	}

	//---------------------------------------------------------------------------------
	// 1���� �迭�� �Ǿ�� ������ �� ������ġ�� ���� ó����ġ�� �ȴ�.
	// 0-1----------------7-8		( 0->10 , 1->10 , 7->16 , 8->16)
	// 9-10               16-17		( 9->10 , 17->16)
	// . .                 . .
	// . .                 . .
	// 63-64              70-71		( 63->64 , 71->70)
	// 72-73--------------79-80		( 72->64 , 73->64 , 79->70  , 80->70 )
	// �̷� ������ ���� �̵��ؼ� �̵��� ���� �߽����� �簢�� 4���� ����� ���� �Ѵ�.
	//---------------------------------------------------------------------------------
	// 0�� cell
	if( nearCellIdx == 0 )
	{
		nearCellIdx = m_MapInfo.m_nTotalVertices + 1;
	}	
	// 1 ~ ���γ� - 1
	else if( 1 <= nearCellIdx && nearCellIdx < m_MapInfo.m_nTotalVertices - 1 )
	{
		nearCellIdx += m_MapInfo.m_nTotalVertices;
	}
	// ���γ�
	else if( nearCellIdx == m_MapInfo.m_nTotalVertices - 1 )
	{
		nearCellIdx += m_MapInfo.m_nTotalVertices - 1;
	}
	// ���� ���� ~ ���� ���γ� - 1
	else if( nearCellIdx % m_MapInfo.m_nTotalVertices == 0 )
	{
		nearCellIdx += 1;
	}
	// ������ ���� ~ ������ ���γ� -1
	else if( nearCellIdx % m_MapInfo.m_nTotalVertices == m_MapInfo.m_nTotalVertices - 1 )
	{
		nearCellIdx -= 1;
	}
	// ���� �Ʒ���
	else if( nearCellIdx == m_MapInfo.m_nTotalVertices * ( m_MapInfo.m_nTotalVertices - 1 ) )
	{
		nearCellIdx -= m_MapInfo.m_nTotalVertices + 1;
	}
	// ������ �Ʒ���
	else if( nearCellIdx == m_MapInfo.m_nTotalVertices * m_MapInfo.m_nTotalVertices - 1 )
	{
		nearCellIdx -= m_MapInfo.m_nTotalVertices - 1;
	}
	// �Ʒ���
	else if( nearCellIdx >= m_MapInfo.m_nTotalVertices * ( m_MapInfo.m_nTotalVertices - 1 ) &&
			 nearCellIdx <  m_MapInfo.m_nTotalVertices * ( m_MapInfo.m_nTotalVertices - 1 ) )
	{
		nearCellIdx -= m_MapInfo.m_nTotalVertices;
	}

	// ������ ���� ���� �߽����� 4���� �簢���� �����.
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

	// ���� �Ʒ����� ���ؽ��� ���հ踦 ���� �ʰ� �Ѵ�.
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