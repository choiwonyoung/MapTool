#pragma once


class Camera
{
public:
	Camera();
	Camera( D3DXVECTOR3& eye , D3DXVECTOR3& look , D3DXVECTOR3& up , int width , int height );
	~Camera();

	// Get �Լ�
	const D3DXVECTOR3 GetEyePt()		{ return m_vEyePt; }
	const D3DXVECTOR3 GetLookatPt()		{ return m_vLookPt; }
	const D3DXVECTOR3 GetUpVec()		{ return m_vUpVec; }
	const D3DXVECTOR3 GetViewDir()		{ return m_vView; }
	const D3DXVECTOR3 GetCross()		{ return m_vCross; }
	const float GetDistance()			{ return D3DXVec3Length( &( m_vLookPt - m_vEyePt ) ); }

	D3DXMATRIX GetViewMatrix()			{ return m_matView; }
	D3DXMATRIX GetBillboardMatrix() { return m_matBillBoard; }

public:
	float		m_fMaxDistance;		// ī�޶� �ְ� �Ÿ�
	float		m_fFrustumDistance;	// Frustum �ְ� �Ÿ�

private:
	D3DXVECTOR3 m_vEyePt;			// ���� ī�޶��� ��ġ
	D3DXVECTOR3 m_vLookPt;			// ī�޶� �ٶ󺸴� ��(��ġ)
	D3DXVECTOR3 m_vUpVec;			// ���� ī�޶��� up ����

	D3DXVECTOR3 m_vView;			// ���� ī�޶� ��ġ�� look ����
	D3DXVECTOR3 m_vCross;			// ���� ī�޶� ��ġ�� right ����

	D3DXMATRIX	m_matView;			// �� ���
	D3DXMATRIX	m_matBillBoard;		// ������ ���

	float		m_fFOV;				// FOV (Field of View) �þ߰�
	float		m_fAspect;			// ��ȸ��( ���� / ���� )
	float		m_fNearPlane;		// ����� Ŭ���� ���
	float		m_fFarPlane;		// �� Ŭ���� ���
	D3DXMATRIX	m_matProj;			// �������� ���

	// ȸ�� ����
	float		m_fAngleHorizon;	// ����(����)
	float		m_fAngleVertical;	// ������(����)

};

