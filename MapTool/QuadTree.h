#pragma once

class Terrain;

class QuadTree
{
	enum  { MAX_CHILD = 4 };
	enum EDGE_TYPE { TL, TR, BL, BR };
	enum INTER_SECT { ALL_OUT = 0 , PART_IN , ALL_IN };

	
public:
	QuadTree( int nTile );
	QuadTree( QuadTree* pParent );
	~QuadTree();

	void		SetupQuadTree( Terrain* pTerrain );
	void		RevealDrawTile( Terrain* pTerrain );

private:
	void		_SetCorners( int _TL , int _TR , int _BL , int _BR );
	QuadTree*	_AddChild( int _TL , int _TR , int _BL , int _BR );
	bool		_SetDivide();

	int			_IsInFrustum( Terrain* pTerrain );
	void		_FrustumCull( Terrain* pTerrain );
	int			_SearchDrawTitle( int tileNum , Terrain* pTerrain );
	
	bool		_IsVisible()		{ return ( m_nEdge[TR] - m_nEdge[TL] <= 1 ); }
	int			_CalcCenter()		{ return ( ( m_nEdge[TL] + m_nEdge[TR] + m_nEdge[BL] + m_nEdge[BR] ) / 4 ); }

private:
	int			m_nCenter;
	int			m_nEdge[4];
	bool		m_bCulled;				// 자식 노드마다 있는 정보이기 때문에 이 정보로 그려져야 할지 검사할 수 있다...
	float		m_fRadius;
	QuadTree*	m_pChild[ MAX_CHILD ];
};

