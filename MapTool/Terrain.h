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

	Frustum* GetFrustum() const { return m_pFrustum; }

private:
	void _SetUpMapInfo( int tiles , int cells , int cellSpacing , int nTotalTex );
	void _SetUpHeightMap();
	void _DrawBaseTile( int index );
	void _DrawSplattingTile( int index );
	void _DrawBoundaryLine( int index );

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

