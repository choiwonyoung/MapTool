#pragma once


// CDlgTab1 dialog

class CDlgTab1 : public CDialog
{
	DECLARE_DYNAMIC(CDlgTab1)

public:
	CDlgTab1(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgTab1();

// Dialog Data
	enum { IDD = IDD_DLG_Map };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
