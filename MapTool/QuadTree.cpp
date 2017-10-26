#include "stdafx.h"
#include "QuadTree.h"
#include "Terrain.h"

QuadTree::QuadTree( int nTile )
	: m_bCulled(false)
	, m_fRadius(0.0f)
{
	// ��Ʈ����� 4�� �ڳ��� �ε����� ���ؽ� ��ǥ ��ġ ����
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
		// ���� 2�� �̻��� ��� HeightMap�� �����ϹǷ� Ÿ�Ϻ��� ����Ʈ���� ������ ��쿡 ������ �ʴ�.
		// ��> Ÿ�� 8*8 �� �� 1*1 �϶� �ƹ� �̻��� ����.
		// �ֳ��ϸ� Ÿ�ϸ� ������ ����Ʈ���� �����ϱ� ������ �� 1�� ���ص� �̻��� ����.
		// ������ �� 2*2�� �ϰԵǸ� HeightMap�� ���ؽ��� 2��� �����ϴµ�
		// Ÿ���� �������� �����Ƿ� HeightMap���� ��ǥ�� ã�°� ������ �ʴ�.
		// �׷��Ƿ� Ÿ�Ͽ� �°� ����Ʈ���� �����ϰ� ���� ������� ��� �Ѵ�.

		const MapInfo& info = pTerrain->m_MapInfo;
		int cells	= info.m_nCells;			// ���� �� ����
		int tiles	= info.m_nTiles + 1;		// ���� Ÿ�� ���� + 1 = ����Ʈ�� ������ ����( 2 ^ n + 1 �̹Ƿ�)
		int vertex	= info.m_nTotalVertices;	// ���� �� * Ÿ�� + 1 ( ���� ���ؽ� ���� )

		// ���� ����� �� ( ���� 2�� �̻��� ���� ���ؼ� , 1�� �϶��� ����� )
		int ADD_TL = ( ( m_nEdge[TL] / tiles ) * cells * vertex ) + ( ( m_nEdge[TL] % tiles ) * cells );
		int ADD_BR = ( ( m_nEdge[BR] / tiles ) * cells * vertex ) + ( ( m_nEdge[BR] % tiles ) * cells );

		// ���� ��ܰ�, ���� �ϴ��� �Ÿ��� ���Ѵ�.
		TERRAIN_VERTEX* pTerrainVertex = pTerrain->m_pHeightMap;
		D3DXVECTOR3 _TL = static_cast<D3DXVECTOR3>( pTerrainVertex[ADD_TL].p ); _TL.y = 0.0f; // y�� 0.0f
		D3DXVECTOR3 _BR = static_cast<D3DXVECTOR3>( pTerrainVertex[ADD_BR].p ); _BR.y = 0.0f; // y�� 0.0f
		D3DXVECTOR3 v = _TL - _BR;

		// v�� �Ÿ����� �� ��带 ���δ� ��豸�� �����̹Ƿ�, 2�� ������ �������� ���Ѵ�.
		m_fRadius = D3DXVec3Length( &v ) / 2.0f;

		m_pChild[TL]->SetupQuadTree( pTerrain );
		m_pChild[TR]->SetupQuadTree( pTerrain );
		m_pChild[BL]->SetupQuadTree( pTerrain );
		m_pChild[BR]->SetupQuadTree( pTerrain );
	}
}

void QuadTree::RevealDrawTile( Terrain* pTerrain )
{
	// frustum �ø��� �Ͽ� �׷��� Ÿ���� �˻��Ѵ�.
	_FrustumCull( pTerrain );

	// ������ Ÿ�Ϸ� ���° Ÿ���� �׷����� Ÿ�� ��ȣ�� �˾Ƴ��
	// pTerrain->m_vecVisibleTile �� Ÿ�� �ε����� ������ �׸���.
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

	// �� �̻� ������ �Ұ��� �Ѱ�? �׷��ٸ� _SetDivide ����
	if( _IsVisible() )
		return false;

	// 4���� �ڽĳ�� �߰�
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
	case ALL_IN:			// frustum�� ���� ���ԵǸ� �ڽĵ鵵 ��� �����̹Ƿ� �� �˻����.
		m_bCulled = false;
		return;

	case PART_IN:			// frustum�� �Ϻ� ���ԵǸ� �ڽĵ鵵 ���� �˻� �ʿ�
		m_bCulled = false;
		break;

	case ALL_OUT:			// frustum�� ���� ����� �ڽĵ鵵 ����Ƿ� �� �˻� ����
		m_bCulled = true;
		return;
	
	default:
		break;
	}

	// �ڽĵ��� frustum �ø�
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
	int cells = info.m_nCells;			// ���� �� ����
	int tiles = info.m_nTiles + 1;		// ���� Ÿ�� ���� + 1 = ����Ʈ�� ������ ����( 2 ^ n + 1 �̹Ƿ�)
	int vertex = info.m_nTotalVertices;	// ���� �� * Ÿ�� + 1 ( ���� ���ؽ� ���� )

	// �ּ� Ÿ��(Ÿ��ũ�� 1) �� �ƴѰ�츸 frustum�� ���� �˻��Ѵ�.
	if( !_IsVisible() )
	{
		// ���� ����� ��(���� 2�̻��� ���� ���ؼ�, 1�϶��� �����)
		int ADD_CEN = ( ( m_nCenter / tiles ) * cells * vertex ) + ( ( m_nCenter % tiles ) * cells );

		// ��豸�ȿ� �ִ°�?
		bInSphere = pTerrain->GetFrustum()->SphereIsInFrustum( &pTerrain->m_pHeightMap[ADD_CEN].p , m_fRadius );
		if( !bInSphere )
			return ALL_OUT;		// ��豸 �ȿ� ������ �� ������ ���ν��� �׽�Ʈ ����
	}

	// ���� ����� ��( ���� 2 �̻��� ���� ���ؼ� , 1�϶��� ����� )
	int ADD_TL = ( ( m_nEdge[TL] / tiles ) * cells * vertex ) + ( ( m_nEdge[TL] % tiles ) * cells );
	int ADD_TR = ( ( m_nEdge[TR] / tiles ) * cells * vertex ) + ( ( m_nEdge[TR] % tiles ) * cells );
	int ADD_BL = ( ( m_nEdge[BL] / tiles ) * cells * vertex ) + ( ( m_nEdge[BL] % tiles ) * cells );
	int ADD_BR = ( ( m_nEdge[BR] / tiles ) * cells * vertex ) + ( ( m_nEdge[BR] % tiles ) * cells );

	// ����Ʈ���� 4���� ��� �������� �׽�Ʈ
	b[0] = pTerrain->GetFrustum()->VertexIsInFrustum( &pTerrain->m_pHeightMap[ADD_TL].p );
	b[1] = pTerrain->GetFrustum()->VertexIsInFrustum( &pTerrain->m_pHeightMap[ADD_TR].p );
	b[2] = pTerrain->GetFrustum()->VertexIsInFrustum( &pTerrain->m_pHeightMap[ADD_BL].p );
	b[3] = pTerrain->GetFrustum()->VertexIsInFrustum( &pTerrain->m_pHeightMap[ADD_BR].p );

	// ���� ������ �Ǵ��ϱ� ���� 2������ �����Ѵ�.
	// 1. 4�� ��� �������� �ȿ� ���� ��
	// 2. leaf Ÿ�� �� ��� ���ؽ��� 1���� ���̸� ���� �����̶�� ����
	// if( b[0] & b[1] & b[2] & b[3] ||						// 4�� ��� �������� �ȿ� ����
	//   ( _IsVisible() && (b[0] | b[1] | b[2] | b[3]) ) )	// leaf Ÿ�� �� ��� ���ؽ��� 1���� ���̸� ���� �����̶�� ����
	if( b[0] + b[1] + b[2] + b[3] == 4 )
		return ALL_IN;

	return PART_IN;
}

int QuadTree::_SearchDrawTitle( int tileNum , Terrain* pTerrain )
{
	// �ø��� ����� �׳� ����
	if( m_bCulled )
	{
		m_bCulled = false;
		return tileNum;
	}

	// ���� ��尡 ��µǾ�� �ϴ°�?
	// Ÿ�� ������ 1 �̸� leaf Ÿ���̹Ƿ� ���
	if( m_nEdge[TR] - m_nEdge[TL] <= 1 )
	{
		// ���� ������� ���� Ÿ���� ��ġ�� �� �� �ִ� ����
		// ���ؽ��� ������ Ÿ�Ϻ��� +1 ����.
		// �׷��� Ÿ�� ��ȣ�� �����ϱ� ���ؼ� �Ʒ��� ���� ���
		int tileIdx = m_nEdge[TL] - ( m_nEdge[TL] / ( pTerrain->m_MapInfo.m_nTiles + 1 ) );

		// ������ ���� Ÿ�� ��ȣ�� �����Ѵ�. (�̰͸� �׸��� ��)
		pTerrain->m_vecVisibleTile.push_back( tileIdx );
		tileNum++;

		return tileNum;
	}

	// �ڽ� ���� �˻�
	if( m_pChild[TL] ) tileNum = m_pChild[TL]->_SearchDrawTitle( tileNum , pTerrain );
	if( m_pChild[TR] ) tileNum = m_pChild[TR]->_SearchDrawTitle( tileNum , pTerrain );
	if( m_pChild[BL] ) tileNum = m_pChild[BL]->_SearchDrawTitle( tileNum , pTerrain );
	if( m_pChild[BR] ) tileNum = m_pChild[BR]->_SearchDrawTitle( tileNum , pTerrain );

	return tileNum;
}

