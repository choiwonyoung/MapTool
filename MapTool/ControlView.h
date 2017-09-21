#pragma once
#include "afxcmn.h"



// CControlView form view
class CDlgTab1;
class CControlView : public CFormView
{
	DECLARE_DYNCREATE(CControlView)

protected:
	CControlView();           // protected constructor used by dynamic creation
	virtual ~CControlView();

public:
	enum { IDD = IDD_CONTROLVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_ctrlTab;

	CDlgTab1* m_pDlgMap;
	
	virtual void OnInitialUpdate();
	afx_msg void OnTcnSelchangeTab1( NMHDR *pNMHDR , LRESULT *pResult );
};


