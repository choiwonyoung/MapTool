#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CDlgTab1 dialog

class CDlgMap : public CDialog
{
	DECLARE_DYNAMIC( CDlgMap )

public:
	CDlgMap( CWnd* pParent = NULL );   // standard constructor
	virtual ~CDlgMap();

// Dialog Data
	enum { IDD = IDD_DLG_Map };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog() override;
	virtual BOOL PreTranslateMessage( MSG* pMsg ) override;

	afx_msg void OnBnClickedDown();
	afx_msg void OnBnClickedUp();
	afx_msg void OnBnClickedFlat();
	afx_msg void OnBnClickedOrigin();
	

	int			m_nUpDown;					// �ø��� ������ ��ġ��
	int			m_nHeightSize;				// ���� ������
	int			m_nFrustumDistance;			// Frustum �Ÿ�
	int			m_nLodLevelSize;			// Lod ���� ��

	BOOL		m_bLODCheck;				// LOD ���� ����
	CSliderCtrl m_sliderHeightSize;			// ���� ���� �����̵�
	CSliderCtrl m_sliderFrustumDistance;	// �������� �Ÿ� �����̵�
	CSliderCtrl m_sliderLodLevelSize;		// LOD ���� ���� �����̵�
};
