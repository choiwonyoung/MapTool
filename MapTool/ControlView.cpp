// ControlView.cpp : implementation file
//

#include "stdafx.h"
#include "MapTool.h"
#include "ControlView.h"
#include "DlgMap.h"

#define SAFE_DELETE(p) {if(p){delete p; p=NULL;}}

// CControlView

IMPLEMENT_DYNCREATE(CControlView, CFormView)

CControlView::CControlView()
	: CFormView(CControlView::IDD),
	m_pDlgMap(NULL)
{

}

CControlView::~CControlView()
{
	SAFE_DELETE( m_pDlgMap )
}

void CControlView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange( pDX );
	DDX_Control( pDX , IDC_TAP_EDITMAP , m_ctrlTab );
}

BEGIN_MESSAGE_MAP(CControlView, CFormView)
END_MESSAGE_MAP()


// CControlView diagnostics

#ifdef _DEBUG
void CControlView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CControlView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CControlView message handlers




void CControlView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	m_ctrlTab.DeleteAllItems();
	m_ctrlTab.InsertItem( 0 , _T("Setting") );
	m_ctrlTab.InsertItem( 1 , L"ÅØ½ºÃÄ");

	CRect rect;
	m_pDlgMap = new CDlgMap;
	m_pDlgMap->Create( IDD_DLG_Map , &m_ctrlTab );
	m_pDlgMap->GetWindowRect( &rect );
	m_pDlgMap->MoveWindow( 5 , 25 , rect.Width() , rect.Height() );
	m_pDlgMap->ShowWindow( SW_SHOW );

	UpdateData( FALSE );
}


void CControlView::OnTcnSelchangeTab1( NMHDR *pNMHDR , LRESULT *pResult )
{
	// TODO: Add your control notification handler code here

	int select = m_ctrlTab.GetCurSel();
	switch( select )
	{
	case 0:
		m_pDlgMap->ShowWindow( SW_SHOW );
		break;
	case 1:
		m_pDlgMap->ShowWindow( SW_HIDE );
		break;
	default:
		break;
	}
	*pResult = 0;
}
