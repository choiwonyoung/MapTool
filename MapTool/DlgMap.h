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
	

	int			m_nUpDown;					// 올리고 내리는 수치값
	int			m_nHeightSize;				// 높이 증가값
	int			m_nFrustumDistance;			// Frustum 거리
	int			m_nLodLevelSize;			// Lod 레벨 값

	BOOL		m_bLODCheck;				// LOD 적용 상태
	CSliderCtrl m_sliderHeightSize;			// 높이 증가 슬라이드
	CSliderCtrl m_sliderFrustumDistance;	// 프러스텀 거리 슬라이드
	CSliderCtrl m_sliderLodLevelSize;		// LOD 레벨 적용 슬라이드
	afx_msg void OnNMCustomdrawSliderHeightsize( NMHDR *pNMHDR , LRESULT *pResult );
	afx_msg void OnNMCustomdrawSliderFrustumdistance( NMHDR *pNMHDR , LRESULT *pResult );
	afx_msg void OnBnClickedCheckLod();
	afx_msg void OnNMCustomdrawSliderLodlevelsize( NMHDR *pNMHDR , LRESULT *pResult );
};
