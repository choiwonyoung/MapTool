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
		//�ε��� ���� ����
		if( FAILED( CreateIB( i ) ) )
		{
			return E_FAIL;
		}
	}

	// �⺻ �ؽ�ó �� ������ �ؽ�ó�� MainFrame LoadTexture���� ������
	// ���� �ؽ�ó�� �ؽ�ó ���ý� ������

	// 0 ~ 1�� �ؽ�ó ���͸��� �ʱ�ȭ �Ѵ�.
	m_pDevice->SetSamplerState( 0 , D3DSAMP_MINFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 0 , D3DSAMP_MAGFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 0 , D3DSAMP_MIPFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 1 , D3DSAMP_MINFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 1 , D3DSAMP_MAGFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 1 , D3DSAMP_MIPFILTER , D3DTEXF_LINEAR );

	return S_OK;
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
		if( ( 0 < LODLevel ) &&
			( 1 <= crackIndex && crackIndex < 5 )
		  )
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

HRESULT Terrain::CreateTexture( int index , string strTexName )
{
	// �ؽ��� ������ �Ѿ�� ���̻� ���Ѵ�.
	if( index > m_MapInfo.m_nTextureNum )
		return E_FAIL;

	if( m_ppTexture[index] == NULL )
	{
		// �ؽ�ó �̸� ����
		m_pStrTextureName[index] = strTexName;

		CString strTemp( strTexName.c_str() );
		/*
		�� �Լ��� �����ϴ� ���� ������ .bmp , .dds , .dib , .jpg , .png , .tga

		D3DXCreateTextureFromFileEx �� ����� ������ �����ս��� �������� , ���� ���� �ǽ��Ѵ�.

		1. �̹����� �����ϸ� �� ���� ��ȯ�� �ε�ÿ� �ǽ��ϸ�,
		ó���� �ð��� �ɸ��� ��찡 �ִ�. �̹�����,
		����� ���� ���� �� �ػ󵵷� �����Ѵ�.
		Ÿ�� �ϵ����� ó���� �� �ִ� ���� 2 �� �ŵ������� ���̸��� ����,
		2 �� �ŵ������� ���̸� ����� �̹����� ���� �� �����Ѵ�.
		2. �ε�ÿ� �Ӹ� �̹����� ���� �ϴ� ����, D3DXFILTER_BOX �� ����� ���͸� �Ѵ�.
		�ڽ� ���ʹ�, D3DXFILTER_TRIANGLE ���� �ٸ� ������ ���ͺ��� ó���� ������.
		DDS ������ ����� �����Ѵ�.
		Microsoft DirectX�� 9.0 �ؽ�Ʈ ������ ��� .dds ������ ����� ǥ���� �� �ֱ� ������,
		.dds ������ Direct3D extension (D3DX)�� �־� �б⳪ ���̸���.
		��, .dds ���Ͽ� �Ӹ��� ������ ���� �־� ������ �Ӹ� ���� �˰����� ����� �̹����� ���� �� �� �ִ�.
		*/
		if( FAILED( D3DXCreateTextureFromFileEx(
			m_pDevice ,								// [in]IDirect3DDevice9 �������̽� ������
			strTemp ,								// [in]���ϸ��� �����ϴ� ���ڿ� ������
			D3DX_DEFAULT ,							// [in]width , �� ���� 0 �Ǵ� D3DX_DEFAULT�� ��� , ���̴� ���Ϸ� ���� ���
			D3DX_DEFAULT ,							// [in]height , �� ���� 0 �Ǵ� D3DX_DEFAULT�� ��� , ���̴� ���Ϸ� ���� ���
			D3DX_DEFAULT ,							// [in]�䱸�Ǵ� MipMapLevel , �� ���� 0 �Ǵ� D3DX_DEFAULT �� ���� , ������ �Ӹ�ü���� ������
			0 ,										// [in]D3DUSAGE_RENDERTARGET���� �����ϸ� , �� ǥ���� ������ Ÿ������ ��� ��, D3DUSAGE_DYNAMIC�� ǥ���� �������� ó���� �ʿ䰡 �ִ°�
			D3DFMT_UNKNOWN ,						// [in]D3DFORMAT ������ ���, �ؽ�ó�� ���ؼ� �䱸�� �ȼ� ������ ��� ��, ���� �޴� �ؽ��� ������ ������ ����� �ٸ� ��찡 ���� , D3DFMT_UNKNOWN�� ���, ������ ���Ϸ� ���� ���
			D3DPOOL_MANAGED ,						// [in]D3DPOOL �������� ���
			D3DX_DEFAULT ,							// [in]�̹����� ���͸� �ϴ� ����� �����ϴ� 1�� Ȥ�� ������ D3DX_FILTER �� �����ϴ� �� , D3DX_DEFAULT �� �����ϴ� ���� D3DX_FILTER_TRIANGLE | D3DX_FILTER_DITHER�� �����ϴ� �Ͱ� ����
			D3DX_DEFAULT ,							// [in]�̹����� ���͸� �ϴ� ����� �����ϴ� 1�� Ȥ�� ������ D3DX_FILTER �� �����ϴ� �� , D3DX_DEFAULT �� �����ϴ� ���� D3DX_FILTER_BOX �� �����ϴ� �Ͱ� ����
			0 ,										// [in]������ �Ǵ� D3DCOLOR�� �� , �÷� Ű�� ��ȿ�� �ϴ� ���� 0�� ����
			NULL ,									// [in,out]�ҽ� �̹��� ���ϳ��� ������ ����� �����ϴ� D3DXIMAGE_INFO ����ü ������
			NULL ,									// [out]���� �ϴ� 256�� �ȷ�Ʈ�� ��Ÿ���� PALETTEENTRY ����ü ������
			&m_ppTexture[index]						// [out]���� �� ť�� �ؽ�ó ��ü�� ��Ÿ���� IDirect3DTexture9 �������̽� ������ �ּ�
			) ) )
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT Terrain::CreateAlphaTexture( int index )
{
	// D3DFMT_A8R8G8B8(���İ��� ���� �ؽ�ó ����)
	// ��ü ���� �ؽ�ó ����(���� ũ�� , ���ؽ� ũ�� �ƴ�)
	if( m_pAlphaTexture[index] == NULL )
	{
		if( FAILED( D3DXCreateTexture(
			m_pDevice ,							// [in]IDirect3DDevice9 �������̽� ������
			m_MapInfo.m_nAlphaTexSize ,			// [in]width , �� ���� 0�� ���, �� 1�� ���ȴ�.
			m_MapInfo.m_nAlphaTexSize ,			// [in]height, �� ���� 0�� ���, �� 1�� ���ȴ�.
			1 ,									// [in]MipMapLevel , �� ���� 0 �Ǵ� D3DX_DEFAULT �� ����, ������ �Ӹ� ü���� ���� �ȴ�.
			0 ,									// [in]D3DUSAGE_RENDERTARGET���� �����ϸ� , �� ǥ���� ������ Ÿ������ ��� ��, D3DUSAGE_DYNAMIC�� ǥ���� �������� ó���� �ʿ䰡 �ִ°�
			D3DFMT_A8R8G8B8 ,					// [in]D3DFORMAT ������ ���, �ؽ�ó�� ���ؼ� �䱸�� �ȼ� ������ ��� ��, ���� �޴� �ؽ��� ������ ������ ����� �ٸ� ��찡 ���� , D3DFMT_UNKNOWN�� ���, ������ ���Ϸ� ���� ���
			D3DPOOL_MANAGED ,					// [in]D3DPOOL �������� ���
			&m_pAlphaTexture[index]				// [out]���� �� ť�� �ؽ�ó ��ü�� ��Ÿ���� IDirect3DTexture9 �������̽� ������ �ּ�
			) ) )
		{
			return E_FAIL;
		}
	}

	D3DLOCKED_RECT alphaTex_Locked;
	::ZeroMemory( &alphaTex_Locked , sizeof( alphaTex_Locked ) );
	if( FAILED( m_pAlphaTexture[index]->LockRect(
		0 ,									// [in] ��״� �ؽ�ó �ҽ��� ���� ����
		&alphaTex_Locked ,					// [out] D3DLOCKED_RECT ����ü ������
		NULL ,								// [in] ��״� ���簢���� ������ , RECT ����ü �����ͷ� ���� , NULL �� �����ϸ� , �ؽ�ó ��ü�� �������� ��Ƽ ������ Ȯ��ȴ�.
		0									// [in] D3DLOCK �÷��� ����
		) ) )
		return E_FAIL;

	BYTE* defBits = (BYTE*)alphaTex_Locked.pBits;
	int i = 0;
	for( int y = 0 ; y < m_MapInfo.m_nAlphaTexSize; ++y )
	{
		for( int x = 0 ; x < m_MapInfo.m_nAlphaTexSize; ++x )
		{
			// ������(0x00)���� ��� ä���.
			defBits[i++] = 0x00;	// B
			defBits[i++] = 0x00;	// G
			defBits[i++] = 0x00;	// R
			defBits[i++] = 0x00;	// A
		}
	}

	if( FAILED( m_pAlphaTexture[index]->UnlockRect( 0 ) ) )
		return E_FAIL;

	return S_OK;

}

HRESULT Terrain::ChangeAlphaTexture( int dest , int src )
{
	// src ���� �ؽ��� ������ dest ���� �ؽ�ó�� �����Ѵ�.
	D3DLOCKED_RECT alpha_dest , alpha_src;
	::ZeroMemory( &alpha_dest , sizeof( alpha_dest ) );
	::ZeroMemory( &alpha_src , sizeof( alpha_src ) );

	if( FAILED( m_pAlphaTexture[dest]->LockRect( 0 , &alpha_dest , NULL , 0 ) ) )
		return E_FAIL;

	if( FAILED( m_pAlphaTexture[src]->LockRect( 0 , &alpha_src , NULL , 0 ) ) )
		return E_FAIL;

	BYTE* pDefBist_dest = (BYTE*)alpha_dest.pBits;
	BYTE* pDefBist_src = (BYTE*)alpha_src.pBits;
	int i = 0 , j = 0;
	for( int y = 0 ; y < m_MapInfo.m_nAlphaTexSize; ++y )
	{
		for( int x = 0; x < m_MapInfo.m_nAlphaTexSize; ++x )
		{
			pDefBist_dest[i++] = pDefBist_src[i++];		// B
			pDefBist_dest[i++] = pDefBist_src[i++];		// G
			pDefBist_dest[i++] = pDefBist_src[i++];		// R
			pDefBist_dest[i++] = pDefBist_src[i++];		// A
		}
	}

	if( FAILED( m_pAlphaTexture[src]->UnlockRect( 0 ) ) )
		return E_FAIL;

	if( FAILED( m_pAlphaTexture[dest]->UnlockRect( 0 ) ) )
		return E_FAIL;

	return S_OK;
}

void Terrain::SaveAlphaValue( int index , BYTE* pRead )
{
	D3DLOCKED_RECT alphaTex_Locked;
	::ZeroMemory( &alphaTex_Locked , sizeof( alphaTex_Locked ) );

	m_pAlphaTexture[index]->LockRect( 0 , &alphaTex_Locked , NULL , 0 );
	BYTE* pDefBits = (BYTE*)alphaTex_Locked.pBits;

	int i = 0 , j = 0;
	for( int y = 0 ; y < m_MapInfo.m_nAlphaTexSize; ++y )
	{
		for( int x = 0 ; x < m_MapInfo.m_nAlphaTexSize; ++x )
		{
			i += 3;

			// ���İ��� ����
			pRead[j++] = pDefBits[i++];
		}
	}

	m_pAlphaTexture[index]->UnlockRect( 0 );
}

void Terrain::LoadAlphaValue( int index , BYTE* pRead )
{
	D3DLOCKED_RECT alphaTex_Locked;
	::ZeroMemory( &alphaTex_Locked , sizeof( alphaTex_Locked ) );

	m_pAlphaTexture[index]->LockRect( 0 , &alphaTex_Locked , NULL , 0 );
	BYTE* pDefBits = (BYTE*)alphaTex_Locked.pBits;

	int i = 0 , j = 0;
	for( int y = 0 ; y < m_MapInfo.m_nAlphaTexSize; ++y )
	{
		for( int x = 0 ; x < m_MapInfo.m_nAlphaTexSize; ++x )
		{
			// �߿��� ���� Alpha�� �ٲٸ� �Ǹ�, RGB ���� ���� �ٲ��� �ʾƵ� �ȴ�.
			// ���� ���İ��� Ȯ���� ���ؼ� ��� ������ ���� �־� ���Ҵ�.
			// ���İ��� �ؽ�ó�� �����ص� �ȳ����Ƿ� RGB�� ���� ���� �ְ� �����ϸ�
			// �ؽ�ó�� ������ �ȴ�.
			pDefBits[i++] = pRead[j];		// B
			pDefBits[i++] = pRead[j];		// G
			pDefBits[i++] = pRead[j];		// R
			pDefBits[i++] = pRead[j];		// A

			j++;
		}
	}

	m_pAlphaTexture[index]->UnlockRect( 0 );

}

void Terrain::SaveAlphaTexture( int index , const char* pFileName )
{
	wchar_t wstr[128];
	swprintf_s( wstr , L"map/alphaTexture/%s%d.bmp" , pFileName , index );

	D3DXSaveTextureToFile(
		wstr ,						// ����� ��ġ�� ���� ��� �� �̸�
		D3DXIFF_BMP ,				// D3DXIMAGE_FILEFORMAT , D3DXIFF_BMP : BMP , D3DXIFF_JPG : JPG , D3DXOFF_TGA : TGA
		m_pAlphaTexture[index] ,	// �����ϴ� �ؽ�ó�� ������ IDirect3DBaseTexture9 �������̽� ������
		NULL						// 256 ���� �ȷ�Ʈ�� ������ PALETTEENTRY ����ü ������ , NULL �� �ص� ��
		);
}

void Terrain::LoadTerrain()
{
	// �ؽ�ó ��� ����( �������� �ʿ� ���� Ŭ���̾�Ʈ�� ����ÿ� ���� )
	if( m_pStrTextureName == NULL )
		m_pStrTextureName = new string[m_MapInfo.m_nTextureNum];

	// �������� ��ü�� ������ ����
	// Ŭ���̾�Ʈ������ ���Ǵ� �ؽ�ó�� �ε�
	if( m_ppTexture == NULL )
	{
		m_ppTexture = new LPDIRECT3DTEXTURE9[m_MapInfo.m_nTextureNum];
		for( int i = 0 ; i < m_MapInfo.m_nTextureNum; ++i )
			m_ppTexture[i] = NULL;
	}

	// ��ü ���� ������ ����
	// ���� ���� ���� �˸� �������� �ڵ� ���� ����
	_SetUpHeightMap();

	int devideCell = m_MapInfo.m_nCells;
	while( devideCell > 2 )
	{
		devideCell /= 2;
		m_nMaxLODLevel++;
	}

	m_ppIB = new LPDIRECT3DINDEXBUFFER9[m_nMaxLODLevel][5];
	for( int i = 0 ; i < m_nMaxLODLevel ; ++i )
	{
		// �ε��� ���� ����
		if( FAILED( CreateIB( i ) ) )
		{
			::MessageBox( NULL , L"CreateIB Error" , L"ERROR" , MB_OK );
			return;
		}
	}

	// 0 ~ 1�� �ؽ��� ���͸��� �ʱ�ȭ �Ѵ�.
	m_pDevice->SetSamplerState( 0 , D3DSAMP_MINFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 0 , D3DSAMP_MAGFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 0 , D3DSAMP_MIPFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 1 , D3DSAMP_MINFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 1 , D3DSAMP_MAGFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 1 , D3DSAMP_MIPFILTER , D3DTEXF_LINEAR );
}

void Terrain::ComputeNormal( POINT pos[3] )
{
	// GPG 3�ǿ� �ִ� ���� ���� �ʵ� ���� ����� ����
	// ���� �ڸ��� ������ ����
	float left = 0.0f;		// ��� ������ �߽����� ����
	float right = 0.0f;		// ��� ������ �߽����� ������
	float up = 0.0f;		// ��� ������ �߽����� ����
	float down = 0.0f;		// ��� ������ �߽����� �Ʒ���
	D3DXVECTOR3 normal( 0.0f , 0.0f , 0.0f );		// ���� ���͸� ������ �ӽ� ����

	// o--A--o // A�� ����
	// B--M--C // B�� ����, M�� ���� ���� , C�� ������
	// o--D--o // D�� �Ʒ���
	int index = 0;
	for( int z = pos[0].y + 1 ; z < pos[2].y - 1; ++z )			// ���̴� Ÿ�� ����(���� �����ڸ��� ����)
	{
		for( int x = pos[0].x + 1 ; z < pos[2].x - 1; ++x )		// ���̴� Ÿ�� ����(���� �����ڸ��� ����)
		{
			index = z * m_MapInfo.m_nTotalVertices;

			left = m_pHeightMap[index - 1].p.y;							// ���� ��ġ�� �߽����� ���� ���̰��� ����
			right = m_pHeightMap[index + 1].p.y;							// ������ 
			up = m_pHeightMap[index - m_MapInfo.m_nTotalVertices].p.y;	// ��
			down = m_pHeightMap[index + m_MapInfo.m_nTotalVertices].p.y;	// �Ʒ�

			normal = D3DXVECTOR3( ( left - right ) , 2 , ( down - up ) );	// ���� ������ ������ �ʰ� �������� ����� ����
			D3DXVec3Normalize( &normal , &normal );							// ���� ���ͷ� ����

			m_pHeightMap[index].n = normal;
		}
	}
}

float Terrain::GetHeight( float x , float z )
{
	x = ( x / m_MapInfo.m_nCellSpacing );
	z = -( z / m_MapInfo.m_nCellSpacing );

	// ���콺 ��ġ�� ���� �Ѿ�� 0.0f�� ����
	if( 0.0f >= x || x >= (float)m_MapInfo.m_nTotalVertices - 1 ||
		0.0f >= z || z >= (float)m_MapInfo.m_nTotalVertices - 1 )
	{
		return 0.0f;
	}

	// x = 126.75f�϶� nx = 126 , floorf(x) = 126.00
	int nx = static_cast<int>( ::floorf( x ) );		// floorf�� �Ҽ����� ���� ������.(�ݿø� ����)
	int nz = static_cast<int>( ::floorf( z ) );

	// A   B
	// *---*
	// | / |
	// *---*
	// C   D
	// ������� y ��ǥ�� ���Ѵ�.
	float A = m_pHeightMap[nz * m_MapInfo.m_nTotalVertices + nx].p.y;
	float B = m_pHeightMap[nz * m_MapInfo.m_nTotalVertices + nx + 1].p.y;
	float C = m_pHeightMap[( nz + 1 )* m_MapInfo.m_nTotalVertices + nx].p.y;
	float D = m_pHeightMap[( nz + 1 )* m_MapInfo.m_nTotalVertices + nx + 1].p.y;

	// x = 126.75f�϶� dx = x(126.75) - nx(126) = 0.75f
	float dx = x - nx;
	float dz = z - nz;

	float height = 0.0f;
	// 1.0f - dx �� A ���� B������ ���̰� 1.0f ��� ������ B���� dx������ ���̸� ����
	// �׷��� �� ABCD������ ���� B���� dx������ ���̰� A���� C������ ������ B���� dx������
	// ���� ��ŭ���� dz�� ũ�� BCD �ﰢ�� ���� �ִٰ� �Ǵ��� �� �ִ� �����̴�.
	if( dz < 1.0f - dx )		// ���� �ﰢ�� ABC
	{
		float uy = B - A;	// A->B
		float vy = C - A;	// A->C

		height = A + _Lerp( 0.0f , uy , dx ) + _Lerp( 0.0f , vy , dz );
	}
	else // �Ʒ��� �ﰢ�� DCB
	{
		float uy = C - D;	// D->C
		float vy = B - D;	// D->B

		height = D + _Lerp( 0.0f , uy , 1.0f - dx ) + _Lerp( 0.0f , vy , 1.0f - dz );
	}

	return height;
}

void Terrain::DrawTerrain()
{
	if( m_bLight )
		m_pDevice->SetRenderState( D3DRS_LIGHTING , TRUE );
	else
		m_pDevice->SetRenderState( D3DRS_LIGHTING , FALSE );

	if( m_bFog )
		m_pDevice->SetRenderState( D3DRS_FOGENABLE , TRUE );
	else
		m_pDevice->SetRenderState( D3DRS_FOGENABLE , FALSE );

	if( m_bWireFrame )
		m_pDevice->SetRenderState( D3DRS_FILLMODE , D3DFILL_WIREFRAME );
	else
		m_pDevice->SetRenderState( D3DRS_FILLMODE , D3DFILL_SOLID );

	for( unsigned int i = 0 ; i < m_vecVisibleTile.size(); ++i )
	{
		// ���̽� �ؽ��� �׸���
		_DrawBaseTile( m_vecVisibleTile[i] );
	}

	// ���÷��� �ؽ�ó �׸���
	// ���̴� Ÿ�Ͼȿ� ���� ��츸 �׸���
	for( unsigned int i = 0; i < m_vecSplattingTile.size() ; ++i )
	{
		bool bVisible = false;
		for( unsigned int j = 0; j < m_vecVisibleTile.size(); ++j )
		{
			if( m_vecVisibleTile[j] == m_vecSplattingTile[i] )
				bVisible = true;
		}

		if( bVisible )
			_DrawSplattingTile( m_vecSplattingTile[i] );
	}

	// ��輱 ����
	if( m_bBoundaryLine )
	{
		for( unsigned int i = 0; i < m_vecVisibleTile.size(); ++i )
		{
			_DrawBoundaryLine( m_vecVisibleTile[i] );
		}
	}
	
	
}

void Terrain::BrushDraw( int count , float outSize , float inSize , D3DXCOLOR outColor , D3DXCOLOR inColor , bool bDrawInBrush )
{
	if( !m_bPickSuccess && m_nBrushType == -1 )
		return;

	m_fBrushOutSize = outSize;
	m_fBrushInSize	= inSize;

	m_pDevice->SetTextureStageState( 0 , D3DTSS_COLOROP		, D3DTOP_SELECTARG1 );
	m_pDevice->SetTextureStageState( 0 , D3DTSS_COLORARG1	, D3DTA_DIFFUSE );
	m_pDevice->SetRenderState( D3DRS_LIGHTING , FALSE );

	if( m_nBrushType == 0 )			// ��
	{
		if( bDrawInBrush )
			_BrushCircleDraw( count , m_fBrushInSize , inColor );

		_BrushCircleDraw( count , m_fBrushOutSize , outColor );
	}
	else if( m_nBrushType == 1 )	// �簢��
	{
		if( bDrawInBrush )
			_BrushRectangleDraw( count , m_fBrushInSize , inColor );

		_BrushRectangleDraw( count , m_fBrushOutSize , outColor );
	}

	m_pDevice->SetRenderState( D3DRS_LIGHTING , TRUE );
}

void Terrain::EditHeightInCircle()
{
	// �� ������ 1�� �����. (�����ϱ� ���ϱ� ����)
	float fx =  ( m_vPickPos.x / m_MapInfo.m_nCellSpacing );
	float fz = -( m_vPickPos.z / m_MapInfo.m_nCellSpacing );

	int startX	= static_cast<int>( fx - m_fBrushOutSize );
	int startZ	= static_cast<int>( fz - m_fBrushOutSize );
	int endX	= static_cast<int>( fx + m_fBrushOutSize );
	int endZ	= static_cast<int>( fz + m_fBrushOutSize );

	if( startX < 0 ) { startX = 0; }
	if( startZ < 0 ) { startZ = 0; }
	if( endX >= m_MapInfo.m_nTotalVertices ) { endX = m_MapInfo.m_nTotalVertices - 1; }		// 0���� �����ϱ� ������ ���� -1
	if( endZ >= m_MapInfo.m_nTotalVertices ) { endZ = m_MapInfo.m_nTotalVertices - 1; }		// 0���� �����ϱ� ������ ���� -1

	D3DXVECTOR3 p( 0.0f , 0.0f , 0.0f );
	for( int z = startZ ; z < endZ ; ++z )
	{
		for( int x = startX; x < endX ; ++x )
		{
			p = m_pHeightMap[z * m_MapInfo.m_nTotalVertices + x].p / (float)m_MapInfo.m_nCellSpacing;
			p.y = 0.0f;

			// �귯�� ũ�� ��ŭ�� ��ġ ���� ���콺 ��ġ������ �Ÿ��� ���Ѵ�.
			D3DXVECTOR3 pix = p - (D3DXVECTOR3( m_vPickPos.x , 0.0f , m_vPickPos.z ) / (float)m_MapInfo.m_nCellSpacing);
			float length = D3DXVec3Length( &pix );

			switch( m_nUpDownMode )
			{
			case UP:
				if( length <= m_fBrushOutSize )
				{
					float y = -1.0f / (float)m_fBrushOutSize * ( length * length ) + (float)m_fBrushOutSize;
					m_pHeightMap[z * m_MapInfo.m_nTotalVertices + x].p.y += ( y / 10.0f ) * m_nHeightRate;
				}
				break;
			case DOWN:
				if( length <= m_fBrushOutSize )
				{
					float y = -1.0f / (float)m_fBrushOutSize * ( length * length ) + (float)m_fBrushOutSize;
					m_pHeightMap[z * m_MapInfo.m_nTotalVertices + x].p.y -= ( y / 10.0f ) * m_nHeightRate;
				}
				break;
			case FLAT:
				if( length <= m_fBrushOutSize )
				{
					m_pHeightMap[z * m_MapInfo.m_nTotalVertices + x].p.y = m_vPickPos.y;
				}
				break;
			case ORIGIN:
				if( length <= m_fBrushOutSize )
				{
					m_pHeightMap[z * m_MapInfo.m_nTotalVertices + x].p.y = 0.0f;
				}
				break;
			}
		}
	}

	// �귯�� �������� 1�� ��ġ�� �� ũ�� ���� Ÿ���� �˻�
	POINT pos[3];
	m_vecBrushAreaTile.clear();
	_SearchTileInPickArea( &pos[0] , &m_vecBrushAreaTile );

	// �˻��� Ÿ�Ϸ� �븻 ���͸� ����Ѵ�.
	ComputeNormal( &pos[0] );

	// �˻��� Ÿ�Ϸ� �����Ѵ�.
	for( unsigned int i = 0 ; i < m_vecBrushAreaTile.size(); ++i )
		CreateVB( m_vecBrushAreaTile[i] , &m_pTileInfo[m_vecBrushAreaTile[i]] );
}

void Terrain::EditHeightInRectangle()
{
	// �� ������ 1�� �����. (�����ϱ� ���ϱ� ����)
	float fx = ( m_vPickPos.x / m_MapInfo.m_nCellSpacing );
	float fz = -( m_vPickPos.z / m_MapInfo.m_nCellSpacing );

	int startX = static_cast<int>( fx - m_fBrushOutSize );
	int startZ = static_cast<int>( fz - m_fBrushOutSize );
	int endX = static_cast<int>( fx + m_fBrushOutSize );
	int endZ = static_cast<int>( fz + m_fBrushOutSize );

	if( startX < 0 ) { startX = 0; }
	if( startZ < 0 ) { startZ = 0; }
	if( endX >= m_MapInfo.m_nTotalVertices ) { endX = m_MapInfo.m_nTotalVertices - 1; }		// 0���� �����ϱ� ������ ���� -1
	if( endZ >= m_MapInfo.m_nTotalVertices ) { endZ = m_MapInfo.m_nTotalVertices - 1; }		// 0���� �����ϱ� ������ ���� -1

	D3DXVECTOR3 p( 0.0f , 0.0f , 0.0f );
	for( int z = startZ ; z < endZ ; ++z )
	{
		for( int x = startX; x < endX ; ++x )
		{
			p = m_pHeightMap[z * m_MapInfo.m_nTotalVertices + x].p / (float)m_MapInfo.m_nCellSpacing;
			p.y = 0.0f;

			D3DXVECTOR3 axisX( 0.0f , 0.0f , 0.0f );
			axisX.x = ( (int)m_vPickPos.x / m_MapInfo.m_nCellSpacing ) - p.x;
			float lenX = D3DXVec3Length( &axisX );

			D3DXVECTOR3 axisZ( 0.0f , 0.0f , 0.0f );
			axisZ.z = ( (int)m_vPickPos.z / m_MapInfo.m_nCellSpacing ) - p.z;
			float lenZ = D3DXVec3Length( &axisZ );

			float length = 0.0f;
			if( lenZ >= lenX )
				length = lenZ;
			else
				length = lenX;

			switch( m_nUpDownMode )
			{
			case UP:
				if( length <= m_fBrushOutSize )
				{
					float y = -1.0f / (float)m_fBrushOutSize * ( length * length ) + (float)m_fBrushOutSize;
					m_pHeightMap[z * m_MapInfo.m_nTotalVertices + x].p.y += ( y / 10.0f ) * m_nHeightRate;
				}
				break;
			case DOWN:
				if( length <= m_fBrushOutSize )
				{
					float y = -1.0f / (float)m_fBrushOutSize * ( length * length ) + (float)m_fBrushOutSize;
					m_pHeightMap[z * m_MapInfo.m_nTotalVertices + x].p.y -= ( y / 10.0f ) * m_nHeightRate;
				}
				break;
			case FLAT:
				if( length <= m_fBrushOutSize )
				{
					m_pHeightMap[z * m_MapInfo.m_nTotalVertices + x].p.y = m_vPickPos.y;
				}
				break;
			case ORIGIN:
				if( length <= m_fBrushOutSize )
				{
					m_pHeightMap[z * m_MapInfo.m_nTotalVertices + x].p.y = 0.0f;
				}
				break;
			}
		}
	}

	// �귯�� �������� 1�� ��ġ�� �� ũ�� ���� Ÿ���� �˻�
	POINT pos[3];
	m_vecBrushAreaTile.clear();
	_SearchTileInPickArea( &pos[0] , &m_vecBrushAreaTile );

	// �˻��� Ÿ�Ϸ� �븻 ���͸� ����Ѵ�.
	ComputeNormal( &pos[0] );

	// �˻��� Ÿ�Ϸ� �����Ѵ�.
	for( unsigned int i = 0 ; i < m_vecBrushAreaTile.size(); ++i )
		CreateVB( m_vecBrushAreaTile[i] , &m_pTileInfo[m_vecBrushAreaTile[i]] );
}

void Terrain::MakeAlphaTexture( int index , int brushType )
{
	// �귯���� ĥ�� ���� ���÷��� ������ �ϱ�
	_TexturePaintInBrushArea();

	int mapSize = m_MapInfo.m_nTiles * m_MapInfo.m_nCells * m_MapInfo.m_nCellSpacing;

	// ���� ���� ���� ������ ���� �ؽ�ó ������ ���Ѵ�.( 1�ؼ��� ũ�⸦ ���Ѵ�.)
	float texelSize = (float)mapSize / (float)m_MapInfo.m_nAlphaTexSize;

	// �ؽ�ó�� ���� �귯�� ũ�⸦ ���Ѵ�.
	int brushSize = (int)( m_fBrushOutSize*m_MapInfo.m_nCellSpacing / texelSize );

	// ���� ���콺 ��ġ�� ���� ���� �ؽ�ó�� tu , tv ���� ���Ѵ�.(0.0f ~ 1.0f)
	float tu =  ( m_vPickPos.x / mapSize );
	float tv = -( m_vPickPos.z / mapSize );

	// ������ ���� tu , tv�� �̿��ؼ� ���� �ؽ�ó�� ��ǥ�� ���Ѵ�.
	// ���� �ؽ�ó�� 256�̸� 0 ~ 255 ���̴�.
	int texPosX = static_cast<int>( m_MapInfo.m_nAlphaTexSize * tu );
	int texPosY = static_cast<int>( m_MapInfo.m_nAlphaTexSize * tv );

	// ���콺 ��ġ�� �߽����� -x ~ +x, -y ~ +y�� �ؽ�ó ��ǥ�� ���Ѵ�.
	int startX	= ( ( texPosX - brushSize ) < 0 ) ? 0 : texPosX - brushSize;
	int startY	= ( ( texPosY - brushSize ) < 0 ) ? 0 : texPosY - brushSize;
	int endX	= ( ( texPosX + brushSize ) >= m_MapInfo.m_nAlphaTexSize ) ? m_MapInfo.m_nAlphaTexSize : texPosX + brushSize;
	int endY	= ( ( texPosY + brushSize ) >= m_MapInfo.m_nAlphaTexSize ) ? m_MapInfo.m_nAlphaTexSize : texPosY + brushSize;

	D3DLOCKED_RECT alphaTex_Locked;
	::ZeroMemory( &alphaTex_Locked , sizeof( alphaTex_Locked ) );
	m_pAlphaTexture[index]->LockRect( 0 , &alphaTex_Locked , NULL , 0 );

	BYTE	data;
	BYTE*	defBits = (BYTE*)alphaTex_Locked.pBits;

	float inSize = 0.0f , outSize = 0.0f;
	if( brushType == 0 )			// �귯�� Ÿ���� ���϶�
	{
		outSize = static_cast<float>( m_fBrushOutSize * m_MapInfo.m_nCellSpacing );
		inSize	= static_cast<float>( m_fBrushInSize * m_MapInfo.m_nCellSpacing );
	}
	else if( brushType == 1 )
	{
		float out	= m_fBrushOutSize * m_MapInfo.m_nCellSpacing;
		float in	= m_fBrushInSize * m_MapInfo.m_nCellSpacing;
		outSize		= sqrt( ( out * out ) + ( out * out ) );
		inSize		= sqrt( ( in * in ) + ( in * in ) );
	}

	// ���콺�� �߽����� ���� �ȼ��� ���鼭 ���� �����Ѵ�.
	for( int y = startY ; y < endY ; ++y )
	{
		for( int x = startX ; x < endX ; ++x )
		{
			// in �� x * t�� ���İ� ��ġ�� �д´�. (BGRA�̱� ����)
			int in = ( alphaTex_Locked.Pitch * y ) + ( x * 4 );

			// �ش� �ȼ��� ���İ��� �о�´�.
			BYTE read = defBits[in];

			// ���� �ؽ�ó�� ���콺 ��ŷ�� ��ġ�� 3D ��ǥ�� ��ȯ�ϰ�,
			// �� ��ġ�� �߽����� ���ʿ�, �ٱ��ʿ����� �Ÿ��� ����Ѵ�.
			D3DXVECTOR3 distance = D3DXVECTOR3( x * texelSize , 0.0f , ( ( m_MapInfo.m_nAlphaTexSize ) - y ) * texelSize ) - D3DXVECTOR3( texPosX * texelSize , 0.0f , ( ( m_MapInfo.m_nAlphaTexSize ) - texPosY )*texelSize );
			float length = D3DXVec3Length( &distance );

			if( length <= inSize )									// ���� ���ȿ� ������
				data = 0xff;										// ���� ������ ������ ��� ���(0xff)
			else if( length <= outSize )							// ū ���ȿ� ���� ��
			{
				length -= inSize;									// ���� ������ �Ÿ��� ���Ѵ�.
				int smooth = static_cast<int>( outSize - inSize );	// ū���� ������ ������ �Ÿ��� ���Ѵ�.

				// ���콺 ��ġ���� �Ÿ��� �ּ��� ����� ��������.
				data = (BYTE)( ( smooth - length ) / (float)smooth * 0xff );
			}
			else
			{
				// �� ���ȿ� �������� ������ �������� �ʴ´�.
				continue;
			}

			// �о�� �ȼ��� ���� ������ ���İ���
			// ���ϱ� �Ǵ� ����⿡ ���� ���� �����Ѵ�.
			if( m_nTextureSplatting == 0 )
				read = ( read < data ) ? data : read;
			else if( m_nTextureSplatting == 1 )
				read = ( ( read - data ) < 0x00 ) ? 0x00 : ( read - data );

			// ���� �ؽ�ó�� ������ ���� ���İ��� �����Ѵ�.
			// �߿��� ���� A�� �ٲٸ� �Ǹ� , RGB���� ���� �ٲ��� �ʾƵ� �ȴ�.
			// ���İ��� �ؽ�ó�� �����ص� �ȳ����Ƿ� RGB�� ���� ���� �ְ� �����ϸ�
			// �ؽ�ó�� ������ �ȴ�.
			defBits[ in++ ] = read;	// B
			defBits[ in++ ] = read;	// G
			defBits[ in++ ] = read;	// R
			defBits[ in++ ] = read;	// A
		}
	}

	m_pAlphaTexture[index]->UnlockRect( 0 );

}

void Terrain::SetupLOD( D3DXVECTOR3 look )
{
	// LOD�� ��ü Ÿ�Ͽ� ���� �����Ѵ�.
	// ���̴� Ÿ�ϸ� �����ϸ� �¿信 ������ �ʴ� Ÿ���� LOD�� �׳� 0�� �ǹǷ� ���η���(ũ��)�� ������ ���� ����.
	for( int lod = 0 ; lod < m_MapInfo.m_nTotalTiles; ++lod )
	{
		// LOD ���� ����
		if( m_bApplyLOD )
			_GetLODLevel( look , lod );
		else
			m_pTileInfo[lod].m_nLODLevel = 0;
	}

	for( unsigned int i = 0 ; i < m_vecVisibleTile.size() ; ++i )
	{
		// LOD ������ �°� ũ�� �ε��� ����
		if( m_bApplyLOD )
			_SetupCrackIndex( m_vecVisibleTile[i] );
		else
		{
			m_pTileInfo[m_vecVisibleTile[i]].m_nLODCrackIndex = 0;
			m_pTileInfo[m_vecVisibleTile[i]].m_nTriangleNum = m_MapInfo.m_nTriangles;
		}
	}
}

void Terrain::PickTerrain( Pick* pPick )
{
	// ���� ī�޶��� ��ġ�� ��� Ÿ������ ���Ѵ�.
	int cameraInTiles = _SearchPositionInTiles( pPick->m_vPickRayOrig );

	// ���� ī�޶��� ��ġ�� Ÿ�Ͼ��� ��� ������ ���Ѵ�.
	EdgeInfo currentMapPosition;
	int cameraInMap = _SearchPositionInTiles( pPick->m_vPickRayOrig , &currentMapPosition );

	// ���� ī�޶��� �� ��ġ�� �߽����� ����� ������ �̵���
	// �̿��� 4���� �簢��(4��)�� �����Ѵ�.
	QuadEdgeInfo currentPart;
	int currentCell = _NearPositionInCells( &currentPart , pPick->m_vPickRayOrig , &currentMapPosition , cameraInMap );

	// ���� ī�޶��� ��ġ�� ���콺 ��ǥ�� �̿��ؼ� ���⺤�͸� �����.
	D3DXVECTOR3 cameraPos = pPick->m_vPickRayOrig;
	cameraPos.y = 0.0f;
	D3DXVECTOR3 cameraDir = pPick->m_vPickRayOrig + pPick->m_vPickRayDir * 1000;
	cameraDir.y = 0.0f;

	m_bPickSuccess = FALSE;
	for( int i = 0; i < 100; ++i )
	{
		// ���� ī�޶� ��ġ�� ������ ������� �������� ���� ������ ��ġ�� ���Ѵ�.
		D3DXVECTOR3 nextPos = cameraDir - cameraPos;
		D3DXVec3Normalize( &nextPos , &nextPos );
		nextPos *= (float)( m_MapInfo.m_nCellSpacing * 2.0f * i );
		nextPos += cameraPos;

		// ���� ������ ��ġ�� Ÿ�Ϲ�ȣ�� ���Ѵ�.
		int nextTileIndex = _SearchPositionInTiles( nextPos );

		// ���� ������ ��ġ�� �ʹ�ȣ�� ���Ѵ�.
		EdgeInfo nextPosition;
		int nextMapIndex = _SearchPositionInMap( nextPos , &nextPosition );

		// �ʹ�ȣ�� �߽����� ����� ������ �̵��� 4���� �簢��(4��)�� ���Ѵ�.
		QuadEdgeInfo nextPart;
		int nextCellIndex = _NearPositionInCells( &nextPart , nextPos , &nextPosition , nextMapIndex );

		int j = 0;
		for( j = 0; j < 4; ++j )
		{
			// 0 = nextPart.part[j].TL
			// 1 = nextPart.part[j].TR
			// 2 = nextPart.part[j].BL
			// 3 = nextPart.part[j].BR
			// �簢���� �� �ﰢ���� �����Ѵ�.(0, 1, 2)
			float dist = 0.0f;
			if( pPick->IntersectTriangle( nextPart.part[j].TL , nextPart.part[j].TR , nextPart.part[j].BL , dist ) )
			{
				m_vPickPos = pPick->m_vPickRayOrig + pPick->m_vPickRayDir * dist;
				m_bPickSuccess = TRUE;
				break;
			}
			// �簢���� �Ʒ� �ﰢ���� �����Ѵ�.(2, 1 ,3)
			if( pPick->IntersectTriangle( nextPart.part[j].BL , nextPart.part[j].TR , nextPart.part[j].BR , dist ) )
			{
				m_vPickPos = pPick->m_vPickRayOrig + pPick->m_vPickRayDir * dist;
				m_bPickSuccess = TRUE;
				break;
			}
		}

		// ���� ������ ��ŷ�� �Ǹ� ����������.(�����͵� ������� �ӵ���� ^^;)
		if( m_bPickSuccess )
			break;
	}
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



void Terrain::_DrawBaseTile( int index )
{
	// �ؽ��� ��Ʈ������ ����� ���
	D3DXMATRIXA16 matTemp;
	D3DXMatrixScaling( &matTemp , 2.0f , 2.0f , 1.0f );
	m_pDevice->SetTextureStageState( 0 , D3DTSS_TEXTURETRANSFORMFLAGS , D3DTTFF_COUNT2 );
	m_pDevice->SetSamplerState( 0 , D3DSAMP_ADDRESSU , D3DTADDRESS_WRAP );
	m_pDevice->SetSamplerState( 0 , D3DSAMP_ADDRESSV , D3DTADDRESS_WRAP );
	m_pDevice->SetTransform( D3DTS_TEXTURE0 , &matTemp );
	m_pDevice->SetTransform( D3DTS_TEXTURE1 , &matTemp );

	// ���̽� �ؽ�ó�� t1.u , t1.v ����( 0 ��° �ؽ�ó �ε���)
	m_pDevice->SetTextureStageState( 0 , D3DTSS_TEXCOORDINDEX , 0 );
	m_pDevice->SetTextureStageState( 0 , D3DTSS_COLOROP		, D3DTOP_MODULATE );
	m_pDevice->SetTextureStageState( 0 , D3DTSS_COLORARG1	, D3DTA_TEXTURE );
	m_pDevice->SetTextureStageState( 0 , D3DTSS_COLORARG2	, D3DTA_DIFFUSE );
	m_pDevice->SetTextureStageState( 0 , D3DTSS_ALPHAOP		, D3DTOP_SELECTARG1 );
	m_pDevice->SetTextureStageState( 0 , D3DTSS_ALPHAARG1	, D3DTA_TEXTURE );

	m_pDevice->SetTexture( 0 , m_ppTexture[m_nTextureIndex[0]] );
	m_pDevice->SetStreamSource( 0 , m_pTileInfo[index].m_pVB , 0 , sizeof( TERRAIN_VERTEX ) );
	m_pDevice->SetFVF( TERRAIN_FVF );
	m_pDevice->SetIndices( m_ppIB[m_pTileInfo[index].m_nLODLevel][m_pTileInfo[index].m_nLODCrackIndex] );
	m_pDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST , 0 , 0 , m_MapInfo.m_nVertices , 0 , m_pTileInfo[index].m_nTriangleNum );
}

void Terrain::_DrawSplattingTile( int index )
{
	/*
		Texture Coordinate Index�� ���� ���� �������� ��ǥ�� (�ִ� 8���� ��ǥ) ���� �� �ִµ� �� �� �ϳ��� �����ϴ°� 
		���� ����� (ex : TERRAIN_VERTEX) �ؽ��� ��ǥ�� ��� ������ �����ϴµ� ���⼭ ����� ������� �ε����� �ο��ȴ�.
	*/
	for( int i = 0; i < m_nSelectTextureNum; ++i )
	{
		// �ؽ�ó ��Ʈ������ ����� ���
		m_pDevice->SetTextureStageState( 
			0 , 
			D3DTSS_TEXTURETRANSFORMFLAGS ,		// �ؽ�ó �������� ������Ʈ�� ���� ����
			D3DTTFF_DISABLE						// D3DTTFF_DISABLE : �ؽ�ó ��ǥ�� �����Ͷ���� ���� �ǳ� �޴´�.
			);	
		m_pDevice->SetTextureStageState( 
			1 , 
			D3DTSS_TEXTURETRANSFORMFLAGS , 
			D3DTTFF_COUNT2						// D3DTTFF_COUNT2 : ������ ������� 2D �� �ؽ�ó ��ǥ�� �����Ѵ�.
			);

		/*
			�ؽ��ĸ� �����ϱ� ���� u,v ��ǥ�� �׻� 0.0 ~ 1.0 ���̿� �ִٰ� �Ͽ���. �׷��� ������ u,v ������
			�� �̻� �Ǵ� �� ������ ���� ���� �� �ִ�. ��巹�� ���� u,v ������ 0.0 ~ 1.0 ���̸� ��� �� �ؽ�ó�� ��� 
			ó���� ���� �����ϴ� ��
		*/
		m_pDevice->SetSamplerState( 0 , D3DSAMP_ADDRESSU , D3DTADDRESS_CLAMP );
		m_pDevice->SetSamplerState( 0 , D3DSAMP_ADDRESSV , D3DTADDRESS_CLAMP );
		m_pDevice->SetSamplerState( 1 , D3DSAMP_ADDRESSU , D3DTADDRESS_WRAP );
		m_pDevice->SetSamplerState( 1 , D3DSAMP_ADDRESSV , D3DTADDRESS_WRAP );

		// texture splatting���� �⺻ �ؽ�ó ���Ŀ� �ö���� �ؽ��Ĵ� 
		// ��Ƽ �ؽ��ĸ� �̿��ؼ� 0�� �ε����� ���ĸ� 1�� �ε����� �̹����� �����Ѵ�.
		// 1�� �ε��������� Į�� OP�� ���ؽ� Į��� �ڽ��� �ؽ��ĸ� �̿��ϸ� 
		// ���� OP�� 0�� �ε����� ���ĸ� ����ؼ� ����Ѵ�.
		// ���� �ؽ��Ĵ� t2.u , t2.v ���� ( 1��° �ؽ��� �ε���)
		m_pDevice->SetTextureStageState( 0 , D3DTSS_TEXCOORDINDEX , 1 );
		m_pDevice->SetTextureStageState( 0 , D3DTSS_COLOROP		, D3DTOP_SELECTARG1 );
		m_pDevice->SetTextureStageState( 0 , D3DTSS_COLORARG1	, D3DTA_TEXTURE );
		m_pDevice->SetTextureStageState( 0 , D3DTSS_ALPHAOP		, D3DTOP_SELECTARG1 );
		m_pDevice->SetTextureStageState( 0 , D3DTSS_ALPHAARG1	, D3DTA_TEXTURE );

		m_pDevice->SetTextureStageState( 1 , D3DTSS_TEXCOORDINDEX , 0 );
		m_pDevice->SetTextureStageState( 1 , D3DTSS_COLOROP		, D3DTOP_MODULATE );
		m_pDevice->SetTextureStageState( 1 , D3DTSS_COLORARG1	, D3DTA_DIFFUSE );
		m_pDevice->SetTextureStageState( 1 , D3DTSS_COLORARG2	, D3DTA_TEXTURE );
		m_pDevice->SetTextureStageState( 1 , D3DTSS_ALPHAOP		, D3DTOP_SELECTARG1 );
		m_pDevice->SetTextureStageState( 1 , D3DTSS_ALPHAARG1	, D3DTA_CURRENT );

		// �⺻ ���� �ɼ�
		/*
			�̹� �׷��� ���� ���ݺ��� ������ �Ϸ��� �ϴ� �������� ���� ������ ���� ������ �ռ�
			���� �ռ� ��� : ������ = ( 1 - a ) * Dest + a * src
		*/
		// D3DRS_SRCBLEND�� �ռ� ���� �� ����
		m_pDevice->SetRenderState( D3DRS_SRCBLEND			, D3DBLEND_SRCALPHA );
		// �̹� ������ �� ������ Ÿ�� ���� �ռ� ����
		// D3DBLEND_INVSRCALPHA �� �ٷ� ( 1 - a ) ��
		m_pDevice->SetRenderState( D3DRS_DESTBLEND			, D3DBLEND_INVSRCALPHA );
		m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE	, TRUE );

		m_pDevice->SetTexture( 0 , m_pAlphaTexture[i] );
		m_pDevice->SetTexture( 1 , m_ppTexture[m_nTextureIndex[i + 1]] );
		m_pDevice->SetStreamSource( 0 , m_pTileInfo[index].m_pVB , 0 , sizeof( TERRAIN_VERTEX ) );
		m_pDevice->SetIndices( m_ppIB[m_pTileInfo[index].m_nLODLevel][m_pTileInfo[index].m_nLODCrackIndex] );
		m_pDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST , 0 , 0 , m_MapInfo.m_nVertices , 0 , m_pTileInfo[index].m_nTriangleNum );
	}

	// ��Ƽ �ؽ��� ����� �����Ѵ�.
	m_pDevice->SetTextureStageState( 0 , D3DTSS_COLOROP , D3DTOP_DISABLE );
	m_pDevice->SetTextureStageState( 0 , D3DTSS_ALPHAOP , D3DTOP_DISABLE );
	m_pDevice->SetTextureStageState( 1 , D3DTSS_COLOROP , D3DTOP_DISABLE );
	m_pDevice->SetTextureStageState( 1 , D3DTSS_ALPHAOP , D3DTOP_DISABLE );

	// ���� ����� �����Ѵ�.
	m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE , FALSE );
}

void Terrain::_DrawBoundaryLine( int index )
{

}













