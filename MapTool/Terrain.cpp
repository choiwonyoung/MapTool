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

	// 텍스쳐 초기화
	m_nTextureIndex[0]	= baseTexIndex;
	m_pStrTextureName	= new string[ m_MapInfo.m_nTextureNum ];
	m_ppTexture			= new LPDIRECT3DTEXTURE9[ m_MapInfo.m_nTextureNum ];
	for( int i = 0 ; i < m_MapInfo.m_nTextureNum; ++i )
		m_ppTexture[i] = NULL;

	// 지형 정보 세팅
	_SetUpHeightMap();

	// 지형 정보를 타일 별로 다시 세팅
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
	// 처음에만 버텍스를 생성한다.
	if( pTileInfo->m_pVB == NULL && pTileInfo->m_pVertex == NULL )
	{
		pTileInfo->m_pVertex = new TERRAIN_VERTEX[m_MapInfo.m_nVertices];
		if( FAILED( m_pDevice->CreateVertexBuffer(
			m_MapInfo.m_nVertices * sizeof( TERRAIN_VERTEX ) ,					// 정점 버퍼 사이즈( 바이트 단위 )
			0 ,																	// 기본 0 , D3DUSAGE 타입을 넣을 수 있음
			TERRAIN_FVF ,														// D3DFVF 에 내용중 필요한 내용을 정의해서 사용
			D3DPOOL_DEFAULT ,													// D3DPOOL 열거형 멤버
			&pTileInfo->m_pVB ,													// 생성된 정점 버퍼 리소스를 나타내는 IDirect3DVertexBuffer9 인터페이스의 포인터 주소
			NULL																// 예약이 끝난 상태
			) ) )
		{
			return S_OK;
		}
	}

	// x축 증가 = (타일번호 % 타일개수) * 셀 개수
	// y축 증가 = (타일번호 / 타일개수) * (한쪽 전체 버텍스 * 셀 개수)
	int nextTileX = ( index % m_MapInfo.m_nTiles ) * m_MapInfo.m_nCells;
	int nextTileZ = ( index / m_MapInfo.m_nTiles ) * ( m_MapInfo.m_nTotalVertices * m_MapInfo.m_nCells );

	// 알파 텍스쳐 좌표를 설정하기 위해( 알파 텍스처는 1장이 모든 타일에 적용된다.)
	float alphaTu = static_cast<float>( index % m_MapInfo.m_nTiles ) / static_cast<float>( m_MapInfo.m_nTiles );
	float alphaTv = static_cast<float>( index / m_MapInfo.m_nTiles ) / static_cast<float>( m_MapInfo.m_nTiles );

	int i = 0;
	for( int z = 0 ; z < m_MapInfo.m_nCells + 1; ++z )
	{
		for( int x = 0 ; x < m_MapInfo.m_nCells + 1; ++z )
		{
			// 전체 버텍스를 타일별로 저장한다.
			pTileInfo->m_pVertex[i] = m_pHeightMap[nextTileZ + nextTileX + ( z * m_MapInfo.m_nTotalVertices ) + x];
			pTileInfo->m_pVertex[i].t1.x = static_cast<float>( x ) / static_cast<float>( m_MapInfo.m_nCells );			// 타일별로 0.0 ~ 1.0 으로 찍는다.
			pTileInfo->m_pVertex[i].t1.y = static_cast<float>( z ) / static_cast<float>( m_MapInfo.m_nCells );			// 타일별로 0.0 ~ 1.0 으로 찍는다.
			pTileInfo->m_pVertex[i].t2.x = alphaTu + static_cast<float>( x ) / static_cast<float>( m_MapInfo.m_nTotalVertices - 1 );
			pTileInfo->m_pVertex[i].t2.y = alphaTv + static_cast<float>( z ) / static_cast<float>( m_MapInfo.m_nTotalVertices - 1 );
			i++;
		}
	}

	// 타일안에 있는 점중에 모서리 4점과 센터를 계산한다.
	// 타일안에 인덱스가 아닌 전체 중의 인덱스 값을 갖는다.
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
		0 ,														// 잠그는 인덱스 데이터에 오프셋(offset)( 바이트 단위 ) , 인덱스 버퍼 전체를 잠그려면 SizeLock와 OffsetToLock의 양쪽 모두의 파라미터에 0 지정
		m_MapInfo.m_nVertices * sizeof( TERRAIN_VERTEX ) ,		// 잠그는 인덱스 데이터에 사이즈( 바이트 단위 )
		(void**)&pV ,											// 돌려줄 인덱스 데이터를 포함한 메모리 버퍼에의 void* 포인터
		NULL													// 잠금의 종류 D3DLock 플래그
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
	// 레벨에 따라 인덱스 버퍼 사이즈를 다르게 해야 함
	// 만약 1타일 안에 셀 개수가 8이면
	// Level 0 = 인덱스 개수(원래 개수    ) , 인덱스 버퍼 순서(1씩 증가)
	// Level 1 = 인덱스 개수(원래 개수 / 4) , 인덱스 버퍼 순서(2씩 증가)
	// Level 2 = 인덱스 개수(원래 개수 /16) , 인덱스 버퍼 순서(4씩 증가)
	// Level 3 = 셀 개수가 8로 가정할 경우 없음( 1타일 안에 2*2가 최하 셀 개수로 설정해서 )
	int level	= static_cast<int>( pow( 4 , LODLevel ) );		// 레벨 마다 ( 1, 4, 16으로 4의 제곱으로 커져야 한다. )
	int size	= m_MapInfo.m_nIndices / level;					// 인덱스 사이즈( 레벨마다 4의 제곱 승으로 나눠야 한다. )
	int next	= static_cast<int>( pow( 2 , LODLevel ) );		// 다음 버텍스로 넘어가는 크기

	// LOD 레벨별로 크랙 인덱스 버퍼를 만듬	
	for( int crackIndex = 0; crackIndex < 5 ; ++crackIndex )
	{
		// 0 번째 레벨일 경우 크랙 인덱스 버퍼가 필요 없으므로 패스
		if( LODLevel == 0 && crackIndex > 0 )
		{
			m_ppIB[LODLevel][crackIndex] = NULL;
			continue;
		}

		// 0은 기본 크랙 인덱스 버퍼
		int indexSize = size;
		// crackIndex 1 ~ 4 는 각각 cell 당 3개의 삼각형 추가
		if( 0 < LODLevel &&
			( 1 <= crackIndex && crackIndex < 5 ) )
		{
			// 셀의 가장자리 방향의 3개의 삼각형 추가( 1삼각형 당 3 인덱스 )
			indexSize += ( m_MapInfo.m_nCells / next ) * 3 * 3;
		}

		// 인덱스는 타일마다 공통이기 때문에 1타일만 생성해서 공유
		// 단 , LOD를 적용할 경우 LOD 레셀에 맞게 인덱스 버퍼를 생성
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

void Terrain::_SetUpHeightMap()
{
	// 맵의 전체 정보를 가질 HeightMap 생성
	if( m_pHeightMap == NULL )
		m_pHeightMap = new TERRAIN_VERTEX[ m_MapInfo.m_nTotalVertices * m_MapInfo.m_nTotalVertices ];

	TERRAIN_VERTEX v;
	int i = 0;
	for( int z = 0 ; z < m_MapInfo.m_nTotalVertices; ++z )
	{
		for( int x = 0 ; m_MapInfo.m_nTotalVertices; ++x )
		{
			v.p.x = static_cast<float>( x * m_MapInfo.m_nCellSpacing );		// x는 0 ~ +로 생성
			v.p.y = 0.0f;
			v.p.z = -(static_cast<float>( z * m_MapInfo.m_nCellSpacing ));	// z는 0 ~ -로 생성

			v.n.x = 0.0f;
			v.n.y = 1.0f;
			v.n.z = 0.0f;
			D3DXVec3Normalize( &v.n , &v.n );

			// 그냥 전체를 0.0 ~ 1.0으로 찍는다.
			// 전체 uv는 필요없음 , 타일별로 찍을 것이기 때문
			v.t1.x = 0.0f;
			v.t1.y = 0.0f;
			v.t2.x = 0.0f;
			v.t2.y = 0.0f;

			m_pHeightMap[i++] = v;
		}
	}
}



