#include "stdafx.h"
#include "QuadTree.h"
#include "Terrain.h"

QuadTree::QuadTree( int nTile )
	: m_bCulled(false)
	, m_fRadius(0.0f)
{
	// 루트노드의 4개 코너의 인덱스와 버텍스 좌표 위치 설정
	m_nEdge[TL] = 0;
	m_nEdge[TR] = nTile - 1;
	m_nEdge[BL] = nTile * ( nTile - 1 );
	m_nEdge[BR] = nTile * nTile - 1;
	m_nCenter = _CalcCenter();

	for( int i = 0 ; i < 4 ; ++i )
		m_pChild[i] = NULL;
}

QuadTree::QuadTree( QuadTree* pParent )
	: m_nCenter(0)
	, m_bCulled(false)
	, m_fRadius(0.0f)
{
	for( int i = 0 ; i < 4 ; ++i )
		m_nEdge[i] = 0;

	for( int i = 0 ; i < MAX_CHILD ; ++i )
		m_pChild[i] = NULL;
}


QuadTree::~QuadTree()
{
	for( int i = 0 ; i < MAX_CHILD ; ++i )
		SAFE_DELETE( m_pChild[i] );
}

void QuadTree::SetupQuadTree( Terrain* pTerrain )
{
	if( _SetDivide() )
	{
		// 셀이 2개 이상일 경우 HeightMap은 증가하므로 타일별로 쿼드트리를 구성한 경우에 맞지가 않다.
		// 예> 타일 8*8 과 셀 1*1 일땐 아무 이상이 없다.
		// 왜냐하면 타일만 가지고 쿼드트리를 구성하기 때문에 셀 1을 곱해도 이상이 없다.
		// 문제는 셀 2*2를 하게되면 HeightMap의 버텍스가 2배로 증가하는데
		// 타일은 증가하지 않으므로 HeightMap으로 좌표를 찾는게 쉽지가 않다.
		// 그러므로 타일에 맞게 쿼드트리를 구헝하고 셀을 적용시켜 줘야 한다.

		const MapInfo& info = pTerrain->m_MapInfo;
		int cells	= info.m_nCells;			// 현재 셀 개수
		int tiles	= info.m_nTiles + 1;		// 현재 타일 개수 + 1 = 쿼드트리 구성을 위해( 2 ^ n + 1 이므로)
		int vertex	= info.m_nTotalVertices;	// 현재 셀 * 타일 + 1 ( 한쪽 버텍스 개수 )

		// 새로 적용된 값 ( 셀이 2개 이상일 때를 위해서 , 1개 일때도 적용됨 )
		int ADD_TL = ( ( m_nEdge[TL] / tiles ) * cells * vertex ) + ( ( m_nEdge[TL] % tiles ) * cells );
		int ADD_BR = ( ( m_nEdge[BR] / tiles ) * cells * vertex ) + ( ( m_nEdge[BR] % tiles ) * cells );

		// 좌측 상단과, 우측 하단의 거리를 구한다.
		TERRAIN_VERTEX* pTerrainVertex = pTerrain->m_pHeightMap;
		D3DXVECTOR3 _TL = static_cast<D3DXVECTOR3>( pTerrainVertex[ADD_TL].p ); _TL.y = 0.0f; // y는 0.0f
		D3DXVECTOR3 _BR = static_cast<D3DXVECTOR3>( pTerrainVertex[ADD_BR].p ); _BR.y = 0.0f; // y는 0.0f
		D3DXVECTOR3 v = _TL - _BR;

		// v의 거리값이 이 노드를 감싸는 경계구의 지름이므로, 2로 나누어 반지름을 구한다.
		m_fRadius = D3DXVec3Length( &v ) / 2.0f;

		m_pChild[TL]->SetupQuadTree( pTerrain );
		m_pChild[TR]->SetupQuadTree( pTerrain );
		m_pChild[BL]->SetupQuadTree( pTerrain );
		m_pChild[BR]->SetupQuadTree( pTerrain );
	}
}

void QuadTree::RevealDrawTile( Terrain* pTerrain )
{
	// frustum 컬링을 하여 그려질 타일을 검사한다.
	_FrustumCull( pTerrain );

	// 가려진 타일로 몇번째 타일이 그려질지 타일 번호를 알아내어서
	// pTerrain->m_vecVisibleTile 에 타일 인덱스를 저장해 그린다.
	pTerrain->m_vecVisibleTile.clear();
	_SearchDrawTitle( 0 , pTerrain );
}

bool QuadTree::_SetDivide()
{
	int north	= ( m_nEdge[TL] + m_nEdge[TR] ) / 2;
	int south	= ( m_nEdge[BL] + m_nEdge[TR] ) / 2;
	int west	= ( m_nEdge[TL] + m_nEdge[BL] ) / 2;
	int east	= ( m_nEdge[TR] + m_nEdge[BR] ) / 2;
	int center	= _CalcCenter();

	// 더 이상 분할이 불가능 한가? 그렇다면 _SetDivide 종료
	if( _IsVisible() )
		return false;

	// 4개의 자식노드 추가
	m_pChild[TL] = _AddChild( m_nEdge[TL] , north , west , center );
	m_pChild[TR] = _AddChild( north , m_nEdge[TR] , center , east );
	m_pChild[BL] = _AddChild( west , center , m_nEdge[BL] , south );
	m_pChild[BR] = _AddChild( center , east , south , m_nEdge[BR] );

	return true;
}

QuadTree* QuadTree::_AddChild( int _TL , int _TR , int _BL , int _BR )
{
	QuadTree* pChild = new QuadTree( this );
	pChild->_SetCorners( _TL , _TR , _BL , _BR );
	return pChild;
}

void QuadTree::_FrustumCull( Terrain* pTerrain )
{
	int ret = _IsInFrustum( pTerrain );
	switch( ret )
	{
	case ALL_IN:			// frustum에 완전 포함되면 자식들도 모두 포함이므로 더 검사안함.
		m_bCulled = false;
		return;

	case PART_IN:			// frustum에 일부 포함되면 자식들도 정밀 검사 필요
		m_bCulled = false;
		break;

	case ALL_OUT:			// frustum에 완전 벗어나면 자식들도 벗어나므로 더 검사 안함
		m_bCulled = true;
		return;
	
	default:
		break;
	}

	// 자식들의 frustum 컬링
	if( m_pChild[0] ) m_pChild[0]->_FrustumCull( pTerrain );
	if( m_pChild[1] ) m_pChild[1]->_FrustumCull( pTerrain );
	if( m_pChild[2] ) m_pChild[2]->_FrustumCull( pTerrain );
	if( m_pChild[3] ) m_pChild[3]->_FrustumCull( pTerrain );
}

int QuadTree::_IsInFrustum( Terrain* pTerrain )
{
	bool b[4];
	bool bInSphere;

	const MapInfo& info = pTerrain->m_MapInfo;
	int cells = info.m_nCells;			// 현재 셀 개수
	int tiles = info.m_nTiles + 1;		// 현재 타일 개수 + 1 = 쿼드트리 구성을 위해( 2 ^ n + 1 이므로)
	int vertex = info.m_nTotalVertices;	// 현재 셀 * 타일 + 1 ( 한쪽 버텍스 개수 )

	// 최소 타일(타일크기 1) 이 아닌경우만 frustum과 구를 검사한다.
	if( !_IsVisible() )
	{
		// 새로 적용된 값(셀이 2이상일 때를 위해서, 1일때도 적용됨)
		int ADD_CEN = ( ( m_nCenter / tiles ) * cells * vertex ) + ( ( m_nCenter % tiles ) * cells );

		// 경계구안에 있는가?
		bInSphere = pTerrain->GetFrustum()->SphereIsInFrustum( &pTerrain->m_pHeightMap[ADD_CEN].p , m_fRadius );
		if( !bInSphere )
			return ALL_OUT;		// 경계구 안에 없으면 점 단위의 프로스텀 테스트 생략
	}

	// 새로 적용된 값( 셀이 2 이상일 때를 위해서 , 1일때도 적용됨 )
	int ADD_TL = ( ( m_nEdge[TL] / tiles ) * cells * vertex ) + ( ( m_nEdge[TL] % tiles ) * cells );
	int ADD_TR = ( ( m_nEdge[TR] / tiles ) * cells * vertex ) + ( ( m_nEdge[TR] % tiles ) * cells );
	int ADD_BL = ( ( m_nEdge[BL] / tiles ) * cells * vertex ) + ( ( m_nEdge[BL] % tiles ) * cells );
	int ADD_BR = ( ( m_nEdge[BR] / tiles ) * cells * vertex ) + ( ( m_nEdge[BR] % tiles ) * cells );

	// 쿼드트리의 4군데 경계 프러스텀 테스트
	b[0] = pTerrain->GetFrustum()->VertexIsInFrustum( &pTerrain->m_pHeightMap[ADD_TL].p );
	b[1] = pTerrain->GetFrustum()->VertexIsInFrustum( &pTerrain->m_pHeightMap[ADD_TR].p );
	b[2] = pTerrain->GetFrustum()->VertexIsInFrustum( &pTerrain->m_pHeightMap[ADD_BL].p );
	b[3] = pTerrain->GetFrustum()->VertexIsInFrustum( &pTerrain->m_pHeightMap[ADD_BR].p );

	// 완전 포함을 판단하기 위해 2가지를 생각한다.
	// 1. 4개 모두 프러스텀 안에 있을 때
	// 2. leaf 타일 일 경우 버텍스가 1개라도 보이면 완전 포함이라고 간주
	// if( b[0] & b[1] & b[2] & b[3] ||						// 4개 모두 프러스텀 안에 있음
	//   ( _IsVisible() && (b[0] | b[1] | b[2] | b[3]) ) )	// leaf 타일 일 경우 버텍스가 1개라도 보이면 완전 포함이라고 간주
	if( b[0] + b[1] + b[2] + b[3] == 4 )
		return ALL_IN;

	return PART_IN;
}

int QuadTree::_SearchDrawTitle( int tileNum , Terrain* pTerrain )
{
	// 컬링된 노드라면 그냥 리턴
	if( m_bCulled )
	{
		m_bCulled = false;
		return tileNum;
	}

	// 현재 노드가 출력되어야 하는가?
	// 타일 간격이 1 이면 leaf 타일이므로 출력
	if( m_nEdge[TR] - m_nEdge[TL] <= 1 )
	{
		// 왼쪽 상단으로 현재 타일의 위치를 알 수 있는 공식
		// 버텍스의 개수는 타일보다 +1 많다.
		// 그래서 타일 번호로 저장하기 위해선 아래의 공식 사용
		int tileIdx = m_nEdge[TL] - ( m_nEdge[TL] / ( pTerrain->m_MapInfo.m_nTiles + 1 ) );

		// 위에서 구한 타일 번호를 저장한다. (이것만 그리면 됨)
		pTerrain->m_vecVisibleTile.push_back( tileIdx );
		tileNum++;

		return tileNum;
	}

	// 자식 노드들 검사
	if( m_pChild[TL] ) tileNum = m_pChild[TL]->_SearchDrawTitle( tileNum , pTerrain );
	if( m_pChild[TR] ) tileNum = m_pChild[TR]->_SearchDrawTitle( tileNum , pTerrain );
	if( m_pChild[BL] ) tileNum = m_pChild[BL]->_SearchDrawTitle( tileNum , pTerrain );
	if( m_pChild[BR] ) tileNum = m_pChild[BR]->_SearchDrawTitle( tileNum , pTerrain );

	return tileNum;
}

