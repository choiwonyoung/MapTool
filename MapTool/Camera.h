#pragma once


class Camera
{
public:
	Camera();
	Camera( D3DXVECTOR3& eye , D3DXVECTOR3& look , D3DXVECTOR3& up , int width , int height );
	~Camera();

	// Get 함수
	const D3DXVECTOR3 GetEyePt()		{ return m_vEyePt; }
	const D3DXVECTOR3 GetLookatPt()		{ return m_vLookPt; }
	const D3DXVECTOR3 GetUpVec()		{ return m_vUpVec; }
	const D3DXVECTOR3 GetViewDir()		{ return m_vView; }
	const D3DXVECTOR3 GetCross()		{ return m_vCross; }
	const float GetDistance()			{ return D3DXVec3Length( &( m_vLookPt - m_vEyePt ) ); }

	D3DXMATRIX GetViewMatrix()			{ return m_matView; }
	D3DXMATRIX GetBillboardMatrix() { return m_matBillBoard; }

public:
	float		m_fMaxDistance;		// 카메라 최고 거리
	float		m_fFrustumDistance;	// Frustum 최고 거리

private:
	D3DXVECTOR3 m_vEyePt;			// 현재 카메라의 위치
	D3DXVECTOR3 m_vLookPt;			// 카메라가 바라보는 곳(위치)
	D3DXVECTOR3 m_vUpVec;			// 현재 카메라의 up 벡터

	D3DXVECTOR3 m_vView;			// 현재 카메라 위치의 look 벡터
	D3DXVECTOR3 m_vCross;			// 현재 카메라 위치의 right 벡터

	D3DXMATRIX	m_matView;			// 뷰 행렬
	D3DXMATRIX	m_matBillBoard;		// 빌보드 행렬

	float		m_fFOV;				// FOV (Field of View) 시야각
	float		m_fAspect;			// 종회비( 가로 / 세로 )
	float		m_fNearPlane;		// 가까운 클리핑 평면
	float		m_fFarPlane;		// 먼 클리핑 평면
	D3DXMATRIX	m_matProj;			// 프로젝션 행렬

	// 회전 관련
	float		m_fAngleHorizon;	// 수평선(가로)
	float		m_fAngleVertical;	// 수직선(세로)

};

