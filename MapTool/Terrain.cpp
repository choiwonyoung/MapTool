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

	// �ؽ��� �ʱ�ȭ
	m_nTextureIndex[0]	= baseTexIndex;
	m_pStrTextureName	= new string[ m_MapInfo.m_nTextureNum ];
	m_ppTexture			= new LPDIRECT3DTEXTURE9[ m_MapInfo.m_nTextureNum ];
	for( int i = 0 ; i < m_MapInfo.m_nTextureNum; ++i )
		m_ppTexture[i] = NULL;

	// ���� ���� ����
	_SetUpHeightMap();

	// ���� ������ Ÿ�� ���� �ٽ� ����
	m_pTileInfo = new TileInfo[ m_MapInfo.m_nTotalTiles ];
	for( int i = 0 ; i < m_MapInfo.m_nTotalTiles; ++i )
	{
		if( FAILED( CreateVB( i , &m_pTileInfo[i] ) ) )
			return E_FAIL;
	}

	int devideCell = m_MapInfo.m_nCells;
	while( devideCell > 2 )
	{
		devideCell /= 2;
		m_nMaxLODLevel++;
	}

	m_ppIB = new LPDIRECT3DINDEXBUFFER9[m_nMaxLODLevel][5];
	for( int i = 0 ; i < m_nMaxLODLevel; ++i )
	{

	}
}

HRESULT Terrain::CreateVB( int index , TileInfo* pTileInfo )
{
	// ó������ ���ؽ��� �����Ѵ�.
	if( pTileInfo->m_pVB == NULL && pTileInfo->m_pVertex == NULL )
	{
		pTileInfo->m_pVertex = new TERRAIN_VERTEX[m_MapInfo.m_nVertices];
		if( FAILED( m_pDevice->CreateVertexBuffer(
			m_MapInfo.m_nVertices * sizeof( TERRAIN_VERTEX ) ,					// ���� ���� ������( ����Ʈ ���� )
			0 ,																	// �⺻ 0 , D3DUSAGE Ÿ���� ���� �� ����
			TERRAIN_FVF ,														// D3DFVF �� ������ �ʿ��� ������ �����ؼ� ���
			D3DPOOL_DEFAULT ,													// D3DPOOL ������ ���
			&pTileInfo->m_pVB ,													// ������ ���� ���� ���ҽ��� ��Ÿ���� IDirect3DVertexBuffer9 �������̽��� ������ �ּ�
			NULL																// ������ ���� ����
			) ) )
		{
			return S_OK;
		}
	}

	// x�� ���� = (Ÿ�Ϲ�ȣ % Ÿ�ϰ���) * �� ����
	// y�� ���� = (Ÿ�Ϲ�ȣ / Ÿ�ϰ���) * (���� ��ü ���ؽ� * �� ����)
	int nextTileX = ( index % m_MapInfo.m_nTiles ) * m_MapInfo.m_nCells;
	int nextTileZ = ( index / m_MapInfo.m_nTiles ) * ( m_MapInfo.m_nTotalVertices * m_MapInfo.m_nCells );

	// ���� �ؽ��� ��ǥ�� �����ϱ� ����( ���� �ؽ�ó�� 1���� ��� Ÿ�Ͽ� ����ȴ�.)
	float alphaTu = static_cast<float>( index % m_MapInfo.m_nTiles ) / static_cast<float>( m_MapInfo.m_nTiles );
	float alphaTv = static_cast<float>( index / m_MapInfo.m_nTiles ) / static_cast<float>( m_MapInfo.m_nTiles );

	int i = 0;
	for( int z = 0 ; z < m_MapInfo.m_nCells + 1; ++z )
	{
		for( int x = 0 ; x < m_MapInfo.m_nCells + 1; ++z )
		{
			// ��ü ���ؽ��� Ÿ�Ϻ��� �����Ѵ�.
			pTileInfo->m_pVertex[i] = m_pHeightMap[nextTileZ + nextTileX + ( z * m_MapInfo.m_nTotalVertices ) + x];
			pTileInfo->m_pVertex[i].t1.x = static_cast<float>( x ) / static_cast<float>( m_MapInfo.m_nCells );			// Ÿ�Ϻ��� 0.0 ~ 1.0 ���� ��´�.
			pTileInfo->m_pVertex[i].t1.y = static_cast<float>( z ) / static_cast<float>( m_MapInfo.m_nCells );			// Ÿ�Ϻ��� 0.0 ~ 1.0 ���� ��´�.
			pTileInfo->m_pVertex[i].t2.x = alphaTu + static_cast<float>( x ) / static_cast<float>( m_MapInfo.m_nTotalVertices - 1 );
			pTileInfo->m_pVertex[i].t2.y = alphaTv + static_cast<float>( z ) / static_cast<float>( m_MapInfo.m_nTotalVertices - 1 );
			i++;
		}
	}

	// Ÿ�Ͼȿ� �ִ� ���߿� �𼭸� 4���� ���͸� ����Ѵ�.
	// Ÿ�Ͼȿ� �ε����� �ƴ� ��ü ���� �ε��� ���� ���´�.
	pTileInfo->m_nEdge[0] = nextTileZ + nextTileX;
	pTileInfo->m_nEdge[1] = nextTileZ + nextTileX + m_MapInfo.m_nCells;
	pTileInfo->m_nEdge[2] = nextTileZ + nextTileX + ( m_MapInfo.m_nCells * m_MapInfo.m_nTotalVertices );
	pTileInfo->m_nEdge[3] = nextTileZ + nextTileX + ( m_MapInfo.m_nCells * m_MapInfo.m_nTotalVertices ) + m_MapInfo.m_nCells;

	pTileInfo->m_nCenter = ( pTileInfo->m_nEdge[0] + pTileInfo->m_nEdge[1] + pTileInfo->m_nEdge[2] + pTileInfo->m_nEdge[3] ) / 4;

	pTileInfo->m_nLODLevel = 0;
	pTileInfo->m_nLODCrackIndex = 0;
	pTileInfo->m_nTriangleNum = m_MapInfo.m_nCells * m_MapInfo.m_nCells * 2;

	TERRAIN_VERTEX* pV;
	if( FAILED( pTileInfo->m_pVB->Lock(
		0 ,														// ��״� �ε��� �����Ϳ� ������(offset)( ����Ʈ ���� ) , �ε��� ���� ��ü�� ��׷��� SizeLock�� OffsetToLock�� ���� ����� �Ķ���Ϳ� 0 ����
		m_MapInfo.m_nVertices * sizeof( TERRAIN_VERTEX ) ,		// ��״� �ε��� �����Ϳ� ������( ����Ʈ ���� )
		(void**)&pV ,											// ������ �ε��� �����͸� ������ �޸� ���ۿ��� void* ������
		NULL													// ����� ���� D3DLock �÷���
		) ) )
	{
		return E_FAIL;
	}

	memcpy( pV , pTileInfo->m_pVertex , m_MapInfo.m_nVertices * sizeof( TERRAIN_VERTEX ) );
	pTileInfo->m_pVB->Unlock();

	return S_OK;
}

HRESULT Terrain::CreateIB( int LODLevel )
{
	// ������ ���� �ε��� ���� ����� �ٸ��� �ؾ� ��
	// ���� 1Ÿ�� �ȿ� �� ������ 8�̸�
	// Level 0 = �ε��� ����(���� ����    ) , �ε��� ���� ����(1�� ����)
	// Level 1 = �ε��� ����(���� ���� / 4) , �ε��� ���� ����(2�� ����)
	// Level 2 = �ε��� ����(���� ���� /16) , �ε��� ���� ����(4�� ����)
	// Level 3 = �� ������ 8�� ������ ��� ����( 1Ÿ�� �ȿ� 2*2�� ���� �� ������ �����ؼ� )
	int level	= static_cast<int>( pow( 4 , LODLevel ) );		// ���� ���� ( 1, 4, 16���� 4�� �������� Ŀ���� �Ѵ�. )
	int size	= m_MapInfo.m_nIndices / level;					// �ε��� ������( �������� 4�� ���� ������ ������ �Ѵ�. )
	int next	= static_cast<int>( pow( 2 , LODLevel ) );		// ���� ���ؽ��� �Ѿ�� ũ��

	// LOD �������� ũ�� �ε��� ���۸� ����	
	for( int crackIndex = 0; crackIndex < 5 ; ++crackIndex )
	{
		// 0 ��° ������ ��� ũ�� �ε��� ���۰� �ʿ� �����Ƿ� �н�
		if( LODLevel == 0 && crackIndex > 0 )
		{
			m_ppIB[LODLevel][crackIndex] = NULL;
			continue;
		}

		// 0�� �⺻ ũ�� �ε��� ����
		int indexSize = size;
		// crackIndex 1 ~ 4 �� ���� cell �� 3���� �ﰢ�� �߰�
		if( 0 < LODLevel &&
			( 1 <= crackIndex && crackIndex < 5 ) )
		{
			// ���� �����ڸ� ������ 3���� �ﰢ�� �߰�( 1�ﰢ�� �� 3 �ε��� )
			indexSize += ( m_MapInfo.m_nCells / next ) * 3 * 3;
		}

		// �ε����� Ÿ�ϸ��� �����̱� ������ 1Ÿ�ϸ� �����ؼ� ����
		// �� , LOD�� ������ ��� LOD ������ �°� �ε��� ���۸� ����
		if( FAILED( m_pDevice->CreateIndexBuffer(
			indexSize * sizeof( WORD ) ,
			0 ,
			D3DFMT_INDEX16 ,
			D3DPOOL_DEFAULT ,
			&m_ppIB[LODLevel][crackIndex] ,
			NULL
			) ) )
		{
			return E_FAIL;
		}

		WORD* pI;
		if( FAILED( m_ppIB[LODLevel][crackIndex]->Lock(
			0 ,
			indexSize * sizeof( WORD ) ,
			(void**)&pI ,
			NULL ) ) )
		{
			return E_FAIL;
		}

		int half = next / 2;
		for( int z = 0; z < m_MapInfo.m_nCells; z += next )
		{
			for( int x = 0 ; x < m_MapInfo.m_nCells; x += next )
			{
				// 0--1--2
				// 3--4--5
				// 6--7--8
				int _0 = z * ( m_MapInfo.m_nCells + 1 ) + x;
				int _1 = z * ( m_MapInfo.m_nCells + 1 ) + x + half;
				int _2 = z * ( m_MapInfo.m_nCells + 1 ) + x + next;

				int _3 = ( z + half ) * ( m_MapInfo.m_nCells + 1 ) + x;
				int _4 = ( z + half ) * ( m_MapInfo.m_nCells + 1 ) + x + half;
				int _5 = ( z + half ) * ( m_MapInfo.m_nCells + 1 ) + x + next;

				int _6 = ( z + next ) * ( m_MapInfo.m_nCells + 1 ) + x;
				int _7 = ( z + next ) * ( m_MapInfo.m_nCells + 1 ) + x + half;
				int _8 = ( z + next ) * ( m_MapInfo.m_nCells + 1 ) + x + next;

				bool bCheck = false;
				switch( crackIndex )
				{
				case LOD_BASIC:
					bCheck = true;
					break;

				case LOD_LEFT:
					{
						if( x == 0 )
						{
							*pI++ = _4; *pI++ = _0; *pI++ = _2;		//4 , 0 , 2
							*pI++ = _4; *pI++ = _2; *pI++ = _8;		//4 , 2 , 8
							*pI++ = _4; *pI++ = _8; *pI++ = _6;		//4 , 8 , 6
							*pI++ = _4; *pI++ = _6; *pI++ = _3;		//4 , 6 , 3
							*pI++ = _4; *pI++ = _3; *pI++ = _0;		//4 , 3 , 0
							bCheck = false;
						}
						else
							bCheck = true;
					}
					break;
				case LOD_TOP:
					{
						if( z == 0 )
						{
							*pI++ = _4;		*pI++ = _0;		*pI++ = _1;	// 4, 0, 1
							*pI++ = _4;		*pI++ = _1;		*pI++ = _2;	// 4, 1, 2
							*pI++ = _4;		*pI++ = _2;		*pI++ = _8;	// 4, 2, 8
							*pI++ = _4;		*pI++ = _8;		*pI++ = _6;	// 4, 8, 6
							*pI++ = _4;		*pI++ = _6;		*pI++ = _0;	// 4, 6, 0
							bCheck = false;
						}
						else { bCheck = true; }
					}
					break;
				case LOD_RIGHT:
					{
						if( x == m_MapInfo.m_nCells - next )
						{
							*pI++ = _4;		*pI++ = _0;		*pI++ = _2;	// 4, 0, 2
							*pI++ = _4;		*pI++ = _2;		*pI++ = _5;	// 4, 2, 5
							*pI++ = _4;		*pI++ = _5;		*pI++ = _8;	// 4, 5, 8
							*pI++ = _4;		*pI++ = _8;		*pI++ = _6;	// 4, 8, 6
							*pI++ = _4;		*pI++ = _6;		*pI++ = _0;	// 4, 6, 0
							bCheck = false;
						}
						else { bCheck = true; }
					}
					break;
				case LOD_BOTTOM:
					{
						if( z == m_MapInfo.m_nCells - next )
						{
							*pI++ = _4;		*pI++ = _0;		*pI++ = _2;	// 4, 0, 2
							*pI++ = _4;		*pI++ = _2;		*pI++ = _8;	// 4, 2, 8
							*pI++ = _4;		*pI++ = _8;		*pI++ = _7;	// 4, 8, 7
							*pI++ = _4;		*pI++ = _7;		*pI++ = _6;	// 4, 7, 6
							*pI++ = _4;		*pI++ = _6;		*pI++ = _0;	// 4, 6, 0
							bCheck = false;
						}
						else { bCheck = true; }
					}
					break;
				} // end case

				if( bCheck )
				{
					*pI++ = _0;		*pI++ = _2;		*pI++ = _6;	// 0, 2, 6
					*pI++ = _6;		*pI++ = _2;		*pI++ = _8;	// 6, 2, 8
				}
			}

			m_ppIB[LODLevel][crackIndex]->Unlock();
		}
	}

	return S_OK;
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

void Terrain::_SetUpHeightMap()
{
	// ���� ��ü ������ ���� HeightMap ����
	if( m_pHeightMap == NULL )
		m_pHeightMap = new TERRAIN_VERTEX[ m_MapInfo.m_nTotalVertices * m_MapInfo.m_nTotalVertices ];

	TERRAIN_VERTEX v;
	int i = 0;
	for( int z = 0 ; z < m_MapInfo.m_nTotalVertices; ++z )
	{
		for( int x = 0 ; m_MapInfo.m_nTotalVertices; ++x )
		{
			v.p.x = static_cast<float>( x * m_MapInfo.m_nCellSpacing );		// x�� 0 ~ +�� ����
			v.p.y = 0.0f;
			v.p.z = -(static_cast<float>( z * m_MapInfo.m_nCellSpacing ));	// z�� 0 ~ -�� ����

			v.n.x = 0.0f;
			v.n.y = 1.0f;
			v.n.z = 0.0f;
			D3DXVec3Normalize( &v.n , &v.n );

			// �׳� ��ü�� 0.0 ~ 1.0���� ��´�.
			// ��ü uv�� �ʿ���� , Ÿ�Ϻ��� ���� ���̱� ����
			v.t1.x = 0.0f;
			v.t1.y = 0.0f;
			v.t2.x = 0.0f;
			v.t2.y = 0.0f;

			m_pHeightMap[i++] = v;
		}
	}
}



