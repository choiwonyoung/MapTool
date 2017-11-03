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
		//인덱스 버퍼 생성
		if( FAILED( CreateIB( i ) ) )
		{
			return E_FAIL;
		}
	}

	// 기본 텍스처 및 나머지 텍스처는 MainFrame LoadTexture에서 생성함
	// 알파 텍스처도 텍스처 선택시 생성함

	// 0 ~ 1번 텍스처 필터링을 초기화 한다.
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
		if( ( 0 < LODLevel ) &&
			( 1 <= crackIndex && crackIndex < 5 )
		  )
		{
			// 셀의 가장자리 방향의 3개의 삼각형 추가( 1삼각형 당 3 인덱스 )
			indexSize += ( m_MapInfo.m_nCells / next ) * 3 * 3;
		}

		// 인덱스는 타일마다 공통이기 때문에 1타일만 생성해서 공유
		// 단 , LOD를 적용할 경우 LOD 레벨에 맞게 인덱스 버퍼를 생성
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
	// 텍스쳐 개수를 넘어가면 더이상 안한다.
	if( index > m_MapInfo.m_nTextureNum )
		return E_FAIL;

	if( m_ppTexture[index] == NULL )
	{
		// 텍스처 이름 저장
		m_pStrTextureName[index] = strTexName;

		CString strTemp( strTexName.c_str() );
		/*
		이 함수가 지원하는 파일 포멧은 .bmp , .dds , .dib , .jpg , .png , .tga

		D3DXCreateTextureFromFileEx 를 사용해 최적인 퍼포먼스를 얻으려면 , 다음 일을 실시한다.

		1. 이미지의 스케일링 및 포맷 변환을 로드시에 실시하면,
		처리에 시간이 걸리는 경우가 있다. 이미지는,
		사용할 때의 포맷 및 해상도로 보존한다.
		타겟 하드웨어로 처리할 수 있는 것이 2 의 거듭제곱의 넓이만의 경우는,
		2 의 거듭제곱의 넓이를 사용해 이미지를 생성 해 보존한다.
		2. 로드시에 밉맵 이미지를 생성 하는 경우는, D3DXFILTER_BOX 를 사용해 필터링 한다.
		박스 필터는, D3DXFILTER_TRIANGLE 등의 다른 종류의 필터보다 처리가 빠르다.
		DDS 파일의 사용을 검토한다.
		Microsoft DirectX® 9.0 텍스트 포맷은 모두 .dds 파일을 사용해 표현할 수 있기 때문에,
		.dds 파일은 Direct3D extension (D3DX)에 있어 읽기나 들이마셔.
		또, .dds 파일에 밉맵을 보존할 수가 있어 임의의 밉맵 생성 알고리즘을 사용해 이미지를 생성 할 수 있다.
		*/
		if( FAILED( D3DXCreateTextureFromFileEx(
			m_pDevice ,								// [in]IDirect3DDevice9 인터페이스 포인터
			strTemp ,								// [in]파일명을 지정하는 문자열 포인터
			D3DX_DEFAULT ,							// [in]width , 이 값이 0 또는 D3DX_DEFAULT의 경우 , 넓이는 파일로 부터 취득
			D3DX_DEFAULT ,							// [in]height , 이 값이 0 또는 D3DX_DEFAULT의 경우 , 높이는 파일로 부터 취득
			D3DX_DEFAULT ,							// [in]요구되는 MipMapLevel , 이 값이 0 또는 D3DX_DEFAULT 일 경우는 , 완전한 밉맵체인이 생성됨
			0 ,										// [in]D3DUSAGE_RENDERTARGET으로 설정하면 , 그 표면은 렌더링 타겟으로 사용 됨, D3DUSAGE_DYNAMIC는 표면을 동적으로 처리할 필요가 있는것
			D3DFMT_UNKNOWN ,						// [in]D3DFORMAT 열거형 멤버, 텍스처에 대해서 요구된 픽셀 포멧을 기술 함, 돌려 받는 텍스쳐 포멧은 지정한 포멧과 다른 경우가 있음 , D3DFMT_UNKNOWN의 경우, 포멧은 파일로 부터 취득
			D3DPOOL_MANAGED ,						// [in]D3DPOOL 열거형의 멤버
			D3DX_DEFAULT ,							// [in]이미지를 필터링 하는 방법을 제어하는 1개 혹은 복수의 D3DX_FILTER 를 지정하는 것 , D3DX_DEFAULT 를 지정하는 것은 D3DX_FILTER_TRIANGLE | D3DX_FILTER_DITHER를 지정하는 것과 동일
			D3DX_DEFAULT ,							// [in]이미지를 필터링 하는 방법을 제어하는 1개 혹은 복수의 D3DX_FILTER 를 지정하는 것 , D3DX_DEFAULT 를 지정하는 것은 D3DX_FILTER_BOX 를 지정하는 것과 동일
			0 ,										// [in]투명이 되는 D3DCOLOR의 값 , 컬러 키를 무효로 하는 경우는 0을 지정
			NULL ,									// [in,out]소스 이미지 파일내의 데이터 기술을 저장하는 D3DXIMAGE_INFO 구조체 포인터
			NULL ,									// [out]저장 하는 256색 팔레트를 나타내는 PALETTEENTRY 구조체 포인터
			&m_ppTexture[index]						// [out]생성 된 큐브 텍스처 개체를 나타내는 IDirect3DTexture9 인터페이스 포인터 주소
			) ) )
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT Terrain::CreateAlphaTexture( int index )
{
	// D3DFMT_A8R8G8B8(알파값을 갖는 텍스처 생성)
	// 전체 알파 텍스처 생성(맵의 크기 , 버텍스 크기 아님)
	if( m_pAlphaTexture[index] == NULL )
	{
		if( FAILED( D3DXCreateTexture(
			m_pDevice ,							// [in]IDirect3DDevice9 인터페이스 포인터
			m_MapInfo.m_nAlphaTexSize ,			// [in]width , 이 값이 0인 경우, 값 1이 사용된다.
			m_MapInfo.m_nAlphaTexSize ,			// [in]height, 이 값이 0인 경우, 값 1이 사용된다.
			1 ,									// [in]MipMapLevel , 이 값이 0 또는 D3DX_DEFAULT 인 경우는, 완전한 밉맵 체인이 생성 된다.
			0 ,									// [in]D3DUSAGE_RENDERTARGET으로 설정하면 , 그 표면은 렌더링 타겟으로 사용 됨, D3DUSAGE_DYNAMIC는 표면을 동적으로 처리할 필요가 있는것
			D3DFMT_A8R8G8B8 ,					// [in]D3DFORMAT 열거형 멤버, 텍스처에 대해서 요구된 픽셀 포멧을 기술 함, 돌려 받는 텍스쳐 포멧은 지정한 포멧과 다른 경우가 있음 , D3DFMT_UNKNOWN의 경우, 포멧은 파일로 부터 취득
			D3DPOOL_MANAGED ,					// [in]D3DPOOL 열거형의 멤버
			&m_pAlphaTexture[index]				// [out]생성 된 큐브 텍스처 개체를 나타내는 IDirect3DTexture9 인터페이스 포인터 주소
			) ) )
		{
			return E_FAIL;
		}
	}

	D3DLOCKED_RECT alphaTex_Locked;
	::ZeroMemory( &alphaTex_Locked , sizeof( alphaTex_Locked ) );
	if( FAILED( m_pAlphaTexture[index]->LockRect(
		0 ,									// [in] 잠그는 텍스처 소스의 레벨 지정
		&alphaTex_Locked ,					// [out] D3DLOCKED_RECT 구조체 포인터
		NULL ,								// [in] 잠그는 직사각형의 포인터 , RECT 구조체 포인터로 지정 , NULL 로 지정하면 , 텍스처 전체를 가리도록 더티 영역이 확대된다.
		0									// [in] D3DLOCK 플래그 설정
		) ) )
		return E_FAIL;

	BYTE* defBits = (BYTE*)alphaTex_Locked.pBits;
	int i = 0;
	for( int y = 0 ; y < m_MapInfo.m_nAlphaTexSize; ++y )
	{
		for( int x = 0 ; x < m_MapInfo.m_nAlphaTexSize; ++x )
		{
			// 검정색(0x00)으로 모두 채운다.
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
	// src 알파 텍스쳐 내용을 dest 알파 텍스처로 저장한다.
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

			// 알파값만 저장
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
			// 중요한 것은 Alpha만 바꾸면 되며, RGB 값은 굳이 바꾸지 않아도 된다.
			// 물론 알파값의 확인을 위해서 모두 동일한 값을 넣어 놓았다.
			// 알파값은 텍스처로 저장해도 안나오므로 RGB로 같은 값을 넣고 저장하면
			// 텍스처에 나오게 된다.
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
		wstr ,						// 저장될 위치의 파일 경로 및 이름
		D3DXIFF_BMP ,				// D3DXIMAGE_FILEFORMAT , D3DXIFF_BMP : BMP , D3DXIFF_JPG : JPG , D3DXOFF_TGA : TGA
		m_pAlphaTexture[index] ,	// 보존하는 텍스처를 포함한 IDirect3DBaseTexture9 인터페이스 포인터
		NULL						// 256 색의 팔레트를 포함한 PALETTEENTRY 구조체 포인터 , NULL 로 해도 됨
		);
}

void Terrain::LoadTerrain()
{
	// 텍스처 목록 저장( 맵툴에선 필요 없고 클라이언트로 저장시에 사용됨 )
	if( m_pStrTextureName == NULL )
		m_pStrTextureName = new string[m_MapInfo.m_nTextureNum];

	// 맵툴에선 전체를 가지고 있음
	// 클라이언트에서는 사용되는 텍스처만 로드
	if( m_ppTexture == NULL )
	{
		m_ppTexture = new LPDIRECT3DTEXTURE9[m_MapInfo.m_nTextureNum];
		for( int i = 0 ; i < m_MapInfo.m_nTextureNum; ++i )
			m_ppTexture[i] = NULL;
	}

	// 전체 지형 정보를 설정
	// 보통 높이 값만 알면 나머지는 자동 생성 가능
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
		// 인덱스 버퍼 생성
		if( FAILED( CreateIB( i ) ) )
		{
			::MessageBox( NULL , L"CreateIB Error" , L"ERROR" , MB_OK );
			return;
		}
	}

	// 0 ~ 1번 텍스쳐 필터링을 초기화 한다.
	m_pDevice->SetSamplerState( 0 , D3DSAMP_MINFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 0 , D3DSAMP_MAGFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 0 , D3DSAMP_MIPFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 1 , D3DSAMP_MINFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 1 , D3DSAMP_MAGFILTER , D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 1 , D3DSAMP_MIPFILTER , D3DTEXF_LINEAR );
}

void Terrain::ComputeNormal( POINT pos[3] )
{
	// GPG 3권에 있는 빠른 높이 필드 법선 계산을 참조
	// 가장 자리의 정점은 제외
	float left = 0.0f;		// 가운데 정점을 중심으로 왼쪽
	float right = 0.0f;		// 가운데 정점을 중심으로 오른쪽
	float up = 0.0f;		// 가운데 정점을 중심으로 위쪽
	float down = 0.0f;		// 가운데 정점을 중심으로 아래쪽
	D3DXVECTOR3 normal( 0.0f , 0.0f , 0.0f );		// 법선 벡터를 저장할 임시 벡터

	// o--A--o // A는 위쪽
	// B--M--C // B는 왼쪽, M은 현재 정점 , C는 오른쪽
	// o--D--o // D는 아래쪽
	int index = 0;
	for( int z = pos[0].y + 1 ; z < pos[2].y - 1; ++z )			// 보이는 타일 세로(끝쪽 가장자리는 제외)
	{
		for( int x = pos[0].x + 1 ; z < pos[2].x - 1; ++x )		// 보이는 타일 가로(끝쪽 가장자리는 제외)
		{
			index = z * m_MapInfo.m_nTotalVertices;

			left = m_pHeightMap[index - 1].p.y;							// 현재 위치를 중심으로 왼쪽 높이값을 저장
			right = m_pHeightMap[index + 1].p.y;							// 오른쪽 
			up = m_pHeightMap[index - m_MapInfo.m_nTotalVertices].p.y;	// 위
			down = m_pHeightMap[index + m_MapInfo.m_nTotalVertices].p.y;	// 아래

			normal = D3DXVECTOR3( ( left - right ) , 2 , ( down - up ) );	// 직접 외적을 구하지 않고 공식으로 평균을 구함
			D3DXVec3Normalize( &normal , &normal );							// 단위 벡터로 만듬

			m_pHeightMap[index].n = normal;
		}
	}
}

float Terrain::GetHeight( float x , float z )
{
	x = ( x / m_MapInfo.m_nCellSpacing );
	z = -( z / m_MapInfo.m_nCellSpacing );

	// 마우스 위치가 맵을 넘어가면 0.0f로 셋팅
	if( 0.0f >= x || x >= (float)m_MapInfo.m_nTotalVertices - 1 ||
		0.0f >= z || z >= (float)m_MapInfo.m_nTotalVertices - 1 )
	{
		return 0.0f;
	}

	// x = 126.75f일때 nx = 126 , floorf(x) = 126.00
	int nx = static_cast<int>( ::floorf( x ) );		// floorf는 소수점을 없애 버린다.(반올림 안함)
	int nz = static_cast<int>( ::floorf( z ) );

	// A   B
	// *---*
	// | / |
	// *---*
	// C   D
	// 순서대로 y 좌표를 구한다.
	float A = m_pHeightMap[nz * m_MapInfo.m_nTotalVertices + nx].p.y;
	float B = m_pHeightMap[nz * m_MapInfo.m_nTotalVertices + nx + 1].p.y;
	float C = m_pHeightMap[( nz + 1 )* m_MapInfo.m_nTotalVertices + nx].p.y;
	float D = m_pHeightMap[( nz + 1 )* m_MapInfo.m_nTotalVertices + nx + 1].p.y;

	// x = 126.75f일때 dx = x(126.75) - nx(126) = 0.75f
	float dx = x - nx;
	float dz = z - nz;

	float height = 0.0f;
	// 1.0f - dx 는 A 에서 B까지의 길이가 1.0f 라고 했을때 B에서 dx까지의 길이를 말함
	// 그런데 위 ABCD도형을 보면 B에서 dx까지의 길이가 A에서 C에서의 길이중 B에서 dx까지의
	// 길이 만큼보다 dz가 크면 BCD 삼각형 내에 있다고 판단할 수 있는 내용이다.
	if( dz < 1.0f - dx )		// 위쪽 삼각형 ABC
	{
		float uy = B - A;	// A->B
		float vy = C - A;	// A->C

		height = A + _Lerp( 0.0f , uy , dx ) + _Lerp( 0.0f , vy , dz );
	}
	else // 아래쪽 삼각형 DCB
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
		// 베이스 텍스쳐 그리기
		_DrawBaseTile( m_vecVisibleTile[i] );
	}

	// 스플래팅 텍스처 그리기
	// 보이는 타일안에 있을 경우만 그리기
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

	// 경계선 모드시
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

	if( m_nBrushType == 0 )			// 원
	{
		if( bDrawInBrush )
			_BrushCircleDraw( count , m_fBrushInSize , inColor );

		_BrushCircleDraw( count , m_fBrushOutSize , outColor );
	}
	else if( m_nBrushType == 1 )	// 사각형
	{
		if( bDrawInBrush )
			_BrushRectangleDraw( count , m_fBrushInSize , inColor );

		_BrushRectangleDraw( count , m_fBrushOutSize , outColor );
	}

	m_pDevice->SetRenderState( D3DRS_LIGHTING , TRUE );
}

void Terrain::EditHeightInCircle()
{
	// 셀 간격을 1로 만든다. (편집하기 편리하기 위해)
	float fx =  ( m_vPickPos.x / m_MapInfo.m_nCellSpacing );
	float fz = -( m_vPickPos.z / m_MapInfo.m_nCellSpacing );

	int startX	= static_cast<int>( fx - m_fBrushOutSize );
	int startZ	= static_cast<int>( fz - m_fBrushOutSize );
	int endX	= static_cast<int>( fx + m_fBrushOutSize );
	int endZ	= static_cast<int>( fz + m_fBrushOutSize );

	if( startX < 0 ) { startX = 0; }
	if( startZ < 0 ) { startZ = 0; }
	if( endX >= m_MapInfo.m_nTotalVertices ) { endX = m_MapInfo.m_nTotalVertices - 1; }		// 0부터 시작하기 때문에 끝은 -1
	if( endZ >= m_MapInfo.m_nTotalVertices ) { endZ = m_MapInfo.m_nTotalVertices - 1; }		// 0부터 시작하기 때문에 끝은 -1

	D3DXVECTOR3 p( 0.0f , 0.0f , 0.0f );
	for( int z = startZ ; z < endZ ; ++z )
	{
		for( int x = startX; x < endX ; ++x )
		{
			p = m_pHeightMap[z * m_MapInfo.m_nTotalVertices + x].p / (float)m_MapInfo.m_nCellSpacing;
			p.y = 0.0f;

			// 브러쉬 크기 만큼의 위치 부터 마우스 위치까지의 거리르 구한다.
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

	// 브러쉬 영역보다 1씩 위치를 더 크게 잡은 타일을 검사
	POINT pos[3];
	m_vecBrushAreaTile.clear();
	_SearchTileInPickArea( &pos[0] , &m_vecBrushAreaTile );

	// 검사한 타일로 노말 벡터를 계산한다.
	ComputeNormal( &pos[0] );

	// 검사한 타일로 저장한다.
	for( unsigned int i = 0 ; i < m_vecBrushAreaTile.size(); ++i )
		CreateVB( m_vecBrushAreaTile[i] , &m_pTileInfo[m_vecBrushAreaTile[i]] );
}

void Terrain::EditHeightInRectangle()
{
	// 셀 간격을 1로 만든다. (편집하기 편리하기 위해)
	float fx = ( m_vPickPos.x / m_MapInfo.m_nCellSpacing );
	float fz = -( m_vPickPos.z / m_MapInfo.m_nCellSpacing );

	int startX = static_cast<int>( fx - m_fBrushOutSize );
	int startZ = static_cast<int>( fz - m_fBrushOutSize );
	int endX = static_cast<int>( fx + m_fBrushOutSize );
	int endZ = static_cast<int>( fz + m_fBrushOutSize );

	if( startX < 0 ) { startX = 0; }
	if( startZ < 0 ) { startZ = 0; }
	if( endX >= m_MapInfo.m_nTotalVertices ) { endX = m_MapInfo.m_nTotalVertices - 1; }		// 0부터 시작하기 때문에 끝은 -1
	if( endZ >= m_MapInfo.m_nTotalVertices ) { endZ = m_MapInfo.m_nTotalVertices - 1; }		// 0부터 시작하기 때문에 끝은 -1

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

	// 브러쉬 영역보다 1씩 위치를 더 크게 잡은 타일을 검사
	POINT pos[3];
	m_vecBrushAreaTile.clear();
	_SearchTileInPickArea( &pos[0] , &m_vecBrushAreaTile );

	// 검사한 타일로 노말 벡터를 계산한다.
	ComputeNormal( &pos[0] );

	// 검사한 타일로 저장한다.
	for( unsigned int i = 0 ; i < m_vecBrushAreaTile.size(); ++i )
		CreateVB( m_vecBrushAreaTile[i] , &m_pTileInfo[m_vecBrushAreaTile[i]] );
}

void Terrain::MakeAlphaTexture( int index , int brushType )
{
	// 브러쉬로 칠한 곳만 스플래팅 렌더링 하기
	_TexturePaintInBrushArea();

	int mapSize = m_MapInfo.m_nTiles * m_MapInfo.m_nCells * m_MapInfo.m_nCellSpacing;

	// 지형 가로 세로 비율에 대한 텍스처 비율을 구한다.( 1텍셀의 크기를 구한다.)
	float texelSize = (float)mapSize / (float)m_MapInfo.m_nAlphaTexSize;

	// 텍스처에 대한 브러쉬 크기를 구한다.
	int brushSize = (int)( m_fBrushOutSize*m_MapInfo.m_nCellSpacing / texelSize );

	// 현재 마우스 위치에 대한 알파 텍스처의 tu , tv 값을 구한다.(0.0f ~ 1.0f)
	float tu =  ( m_vPickPos.x / mapSize );
	float tv = -( m_vPickPos.z / mapSize );

	// 위에서 구한 tu , tv를 이용해서 알파 텍스처의 좌표를 구한다.
	// 알파 텍스처가 256이면 0 ~ 255 값이다.
	int texPosX = static_cast<int>( m_MapInfo.m_nAlphaTexSize * tu );
	int texPosY = static_cast<int>( m_MapInfo.m_nAlphaTexSize * tv );

	// 마우스 위치를 중심으로 -x ~ +x, -y ~ +y의 텍스처 좌표를 구한다.
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
	if( brushType == 0 )			// 브러쉬 타입이 원일때
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

	// 마우스를 중심으로 구한 픽셀을 돌면서 값을 셋팅한다.
	for( int y = startY ; y < endY ; ++y )
	{
		for( int x = startX ; x < endX ; ++x )
		{
			// in 은 x * t로 알파값 위치만 읽는다. (BGRA이기 때문)
			int in = ( alphaTex_Locked.Pitch * y ) + ( x * 4 );

			// 해당 픽셀의 알파값을 읽어온다.
			BYTE read = defBits[in];

			// 알파 텍스처의 마우스 픽킹한 위치를 3D 좌표로 변환하고,
			// 그 위치를 중심으로 안쪽워, 바깥쪽원과의 거리를 계산한다.
			D3DXVECTOR3 distance = D3DXVECTOR3( x * texelSize , 0.0f , ( ( m_MapInfo.m_nAlphaTexSize ) - y ) * texelSize ) - D3DXVECTOR3( texPosX * texelSize , 0.0f , ( ( m_MapInfo.m_nAlphaTexSize ) - texPosY )*texelSize );
			float length = D3DXVec3Length( &distance );

			if( length <= inSize )									// 작은 원안에 있을때
				data = 0xff;										// 작은 원보다 작으면 모두 흰색(0xff)
			else if( length <= outSize )							// 큰 원안에 있을 때
			{
				length -= inSize;									// 작은 원과의 거리를 구한다.
				int smooth = static_cast<int>( outSize - inSize );	// 큰원과 작은원 사이의 거리를 구한다.

				// 마우스 위치에서 거리가 멀수록 흰색이 옅어진다.
				data = (BYTE)( ( smooth - length ) / (float)smooth * 0xff );
			}
			else
			{
				// 두 원안에 존재하지 않으면 적용하지 않는다.
				continue;
			}

			// 읽어온 픽셀에 새로 생성된 알파값을
			// 더하기 또는 지우기에 따라 새로 셋팅한다.
			if( m_nTextureSplatting == 0 )
				read = ( read < data ) ? data : read;
			else if( m_nTextureSplatting == 1 )
				read = ( ( read - data ) < 0x00 ) ? 0x00 : ( read - data );

			// 알파 텍스처에 위에서 구한 알파값을 갱신한다.
			// 중요한 것은 A만 바꾸면 되며 , RGB값은 굳이 바꾸지 않아도 된다.
			// 알파값은 텍스처로 저장해도 안나오므로 RGB로 같은 값을 넣고 저장하면
			// 텍스처에 나오게 된다.
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
	// LOD는 전체 타일에 대해 적용한다.
	// 보이는 타일만 적용하면 좌요에 보이지 않는 타일은 LOD가 그냥 0이 되므로 세부레벨(크랙)을 설정할 수가 없다.
	for( int lod = 0 ; lod < m_MapInfo.m_nTotalTiles; ++lod )
	{
		// LOD 레벨 설정
		if( m_bApplyLOD )
			_GetLODLevel( look , lod );
		else
			m_pTileInfo[lod].m_nLODLevel = 0;
	}

	for( unsigned int i = 0 ; i < m_vecVisibleTile.size() ; ++i )
	{
		// LOD 레벨에 맞게 크랙 인덱스 설정
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
	// 현재 카메라의 위치가 어느 타일인지 구한다.
	int cameraInTiles = _SearchPositionInTiles( pPick->m_vPickRayOrig );

	// 현재 카메라의 위치가 타일안의 어느 셀인지 구한다.
	EdgeInfo currentMapPosition;
	int cameraInMap = _SearchPositionInTiles( pPick->m_vPickRayOrig , &currentMapPosition );

	// 현재 카메라의 셀 위치를 중심으로 가까운 곳으로 이동후
	// 이웃한 4개의 사각형(4점)을 저장한다.
	QuadEdgeInfo currentPart;
	int currentCell = _NearPositionInCells( &currentPart , pPick->m_vPickRayOrig , &currentMapPosition , cameraInMap );

	// 현재 카메라의 위치와 마우스 좌표를 이용해서 방향벡터를 만든다.
	D3DXVECTOR3 cameraPos = pPick->m_vPickRayOrig;
	cameraPos.y = 0.0f;
	D3DXVECTOR3 cameraDir = pPick->m_vPickRayOrig + pPick->m_vPickRayDir * 1000;
	cameraDir.y = 0.0f;

	m_bPickSuccess = FALSE;
	for( int i = 0; i < 100; ++i )
	{
		// 현재 카메라 위치를 구한후 어느정도 간격으로 다음 조사할 위치를 구한다.
		D3DXVECTOR3 nextPos = cameraDir - cameraPos;
		D3DXVec3Normalize( &nextPos , &nextPos );
		nextPos *= (float)( m_MapInfo.m_nCellSpacing * 2.0f * i );
		nextPos += cameraPos;

		// 다음 조사할 위치의 타일번호를 구한다.
		int nextTileIndex = _SearchPositionInTiles( nextPos );

		// 다음 조사할 위치의 맵번호를 구한다.
		EdgeInfo nextPosition;
		int nextMapIndex = _SearchPositionInMap( nextPos , &nextPosition );

		// 맵번호를 중심으로 가까운 점으로 이동후 4개의 사각형(4점)을 구한다.
		QuadEdgeInfo nextPart;
		int nextCellIndex = _NearPositionInCells( &nextPart , nextPos , &nextPosition , nextMapIndex );

		int j = 0;
		for( j = 0; j < 4; ++j )
		{
			// 0 = nextPart.part[j].TL
			// 1 = nextPart.part[j].TR
			// 2 = nextPart.part[j].BL
			// 3 = nextPart.part[j].BR
			// 사각형의 윗 삼각형을 대입한다.(0, 1, 2)
			float dist = 0.0f;
			if( pPick->IntersectTriangle( nextPart.part[j].TL , nextPart.part[j].TR , nextPart.part[j].BL , dist ) )
			{
				m_vPickPos = pPick->m_vPickRayOrig + pPick->m_vPickRayDir * dist;
				m_bPickSuccess = TRUE;
				break;
			}
			// 사각형의 아래 삼각형을 대입한다.(2, 1 ,3)
			if( pPick->IntersectTriangle( nextPart.part[j].BL , nextPart.part[j].TR , nextPart.part[j].BR , dist ) )
			{
				m_vPickPos = pPick->m_vPickRayOrig + pPick->m_vPickRayDir * dist;
				m_bPickSuccess = TRUE;
				break;
			}
		}

		// 만약 위에서 피킹이 되면 빠져나간다.(남은것들 관계없이 속도향상 ^^;)
		if( m_bPickSuccess )
			break;
	}
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



void Terrain::_DrawBaseTile( int index )
{
	// 텍스쳐 매트릭스를 사용할 경우
	D3DXMATRIXA16 matTemp;
	D3DXMatrixScaling( &matTemp , 2.0f , 2.0f , 1.0f );
	m_pDevice->SetTextureStageState( 0 , D3DTSS_TEXTURETRANSFORMFLAGS , D3DTTFF_COUNT2 );
	m_pDevice->SetSamplerState( 0 , D3DSAMP_ADDRESSU , D3DTADDRESS_WRAP );
	m_pDevice->SetSamplerState( 0 , D3DSAMP_ADDRESSV , D3DTADDRESS_WRAP );
	m_pDevice->SetTransform( D3DTS_TEXTURE0 , &matTemp );
	m_pDevice->SetTransform( D3DTS_TEXTURE1 , &matTemp );

	// 베이스 텍스처는 t1.u , t1.v 적용( 0 번째 텍스처 인덱스)
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
		Texture Coordinate Index는 정점 마다 여러개의 좌표를 (최대 8개의 좌표) 가질 수 있는데 그 중 하나를 선택하는것 
		정점 선언시 (ex : TERRAIN_VERTEX) 텍스쳐 좌표를 어떻게 쓸지를 선언하는데 여기서 선언된 순서대로 인덱스가 부여된다.
	*/
	for( int i = 0; i < m_nSelectTextureNum; ++i )
	{
		// 텍스처 매트릭스를 사용할 경우
		m_pDevice->SetTextureStageState( 
			0 , 
			D3DTSS_TEXTURETRANSFORMFLAGS ,		// 텍스처 스테이지 스테이트의 값을 정의
			D3DTTFF_DISABLE						// D3DTTFF_DISABLE : 텍스처 좌표는 래스터라이즈에 직접 건내 받는다.
			);	
		m_pDevice->SetTextureStageState( 
			1 , 
			D3DTSS_TEXTURETRANSFORMFLAGS , 
			D3DTTFF_COUNT2						// D3DTTFF_COUNT2 : 래스터 라이즈는 2D 의 텍스처 좌표를 상정한다.
			);

		/*
			텍스쳐를 제어하기 위한 u,v 좌표는 항상 0.0 ~ 1.0 사이에 있다고 하였다. 그러나 정점의 u,v 성분은
			그 이상 또는 그 이하의 값을 가질 수 있다. 어드레스 모드는 u,v 성분이 0.0 ~ 1.0 사이를 벗어날 때 텍스처를 어떻게 
			처리할 지를 결정하는 것
		*/
		m_pDevice->SetSamplerState( 0 , D3DSAMP_ADDRESSU , D3DTADDRESS_CLAMP );
		m_pDevice->SetSamplerState( 0 , D3DSAMP_ADDRESSV , D3DTADDRESS_CLAMP );
		m_pDevice->SetSamplerState( 1 , D3DSAMP_ADDRESSU , D3DTADDRESS_WRAP );
		m_pDevice->SetSamplerState( 1 , D3DSAMP_ADDRESSV , D3DTADDRESS_WRAP );

		// texture splatting에서 기본 텍스처 이후에 올라오는 텍스쳐는 
		// 멀티 텍스쳐를 이용해서 0번 인덱스에 알파를 1번 인덱스에 이미지를 셋팅한다.
		// 1번 인덱스에서는 칼라 OP는 버텍스 칼라와 자신의 텍스쳐를 이용하며 
		// 알파 OP는 0번 인덱스의 알파를 사용해서 출력한다.
		// 알파 텍스쳐는 t2.u , t2.v 적용 ( 1번째 텍스쳐 인덱스)
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

		// 기본 알파 옵션
		/*
			이미 그려진 색과 지금부터 랜더링 하려고 하는 폴리곤의 색을 다음과 같은 비율로 합성
			선형 합성 방법 : 최종색 = ( 1 - a ) * Dest + a * src
		*/
		// D3DRS_SRCBLEND는 합성 강도 를 설정
		m_pDevice->SetRenderState( D3DRS_SRCBLEND			, D3DBLEND_SRCALPHA );
		// 이미 랜더링 된 랜더일 타겟 색의 합성 강도
		// D3DBLEND_INVSRCALPHA 가 바로 ( 1 - a ) 임
		m_pDevice->SetRenderState( D3DRS_DESTBLEND			, D3DBLEND_INVSRCALPHA );
		m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE	, TRUE );

		m_pDevice->SetTexture( 0 , m_pAlphaTexture[i] );
		m_pDevice->SetTexture( 1 , m_ppTexture[m_nTextureIndex[i + 1]] );
		m_pDevice->SetStreamSource( 0 , m_pTileInfo[index].m_pVB , 0 , sizeof( TERRAIN_VERTEX ) );
		m_pDevice->SetIndices( m_ppIB[m_pTileInfo[index].m_nLODLevel][m_pTileInfo[index].m_nLODCrackIndex] );
		m_pDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST , 0 , 0 , m_MapInfo.m_nVertices , 0 , m_pTileInfo[index].m_nTriangleNum );
	}

	// 멀티 텍스쳐 사용을 중지한다.
	m_pDevice->SetTextureStageState( 0 , D3DTSS_COLOROP , D3DTOP_DISABLE );
	m_pDevice->SetTextureStageState( 0 , D3DTSS_ALPHAOP , D3DTOP_DISABLE );
	m_pDevice->SetTextureStageState( 1 , D3DTSS_COLOROP , D3DTOP_DISABLE );
	m_pDevice->SetTextureStageState( 1 , D3DTSS_ALPHAOP , D3DTOP_DISABLE );

	// 알파 사용을 중지한다.
	m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE , FALSE );
}

void Terrain::_DrawBoundaryLine( int index )
{

}













