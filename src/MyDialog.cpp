///////////////////////////////////////
// CViewDialog.cpp

#include "stdafx.h"
#include "MyDialog.h"
#include "resource.h"

#include "Output.h"

#include "TabbedMDIApp.h"
extern CTabbedMDIApp *theApp_ptr;

// Definitions for the CViewDialog class
CViewDialog::CViewDialog(UINT nResID) : CDialog(nResID)
{
	temp_str=NULL;
	auto_send_str=NULL;
}

CViewDialog::~CViewDialog()
{
}

void CViewDialog::AppendText(int nID, LPCTSTR szText)
{
    // This function appends text to an edit control

    // Append Line Feed
    LRESULT ndx = SendDlgItemMessage(nID, WM_GETTEXTLENGTH, 0, 0);
    if (ndx)
    {
        SendDlgItemMessage(nID, EM_SETSEL, ndx, ndx);
        SendDlgItemMessage(nID, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(_T("\r\n")));
    }

    // Append text
    ndx = SendDlgItemMessage(nID, WM_GETTEXTLENGTH, 0, 0);
    SendDlgItemMessage(nID, EM_SETSEL, ndx, ndx);
    SendDlgItemMessage(nID, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(szText));
}

INT_PTR CViewDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Pass resizing messages on to the resizer
    m_Resizer.HandleMessage(uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_MOUSEACTIVATE:      return OnMouseActivate(uMsg, wParam, lParam);
		case WM_TIMER:		 
			if(LOWORD(wParam)==1000)
			{
				OnAutoSendTimer();
			}
		break;
    }

    // Pass unhandled messages on to parent DialogProc
    return DialogProcDefault(uMsg, wParam, lParam);
}

void CViewDialog::OnCancel()
{
}

void CViewDialog::OnClose()
{
    // Suppress the WM_CLOSE message that is sent by pressing an Esc key in the RichEdit controls.
}

BOOL CViewDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    UINT nID = LOWORD(wParam);
    switch (nID)
    {
    case IDC_BUTTON1:   return OnButton();

    case IDC_RADIO1:    // intentionally blank
    case IDC_RADIO2:
    case IDC_RADIO3:    return OnRangeOfRadioIDs(IDC_RADIO1, IDC_RADIO3, nID);
    }

    //return FALSE;
    return m_RichEdit2.OnCommand(wParam, lParam);
}

BOOL CViewDialog::OnInitDialog()
{
    // Attach CWnd objects to the dialog items
    AttachItem(IDC_BUTTON1, m_Button);
    AttachItem(IDC_RADIO1,  m_RadioA);
    AttachItem(IDC_RADIO2,  m_RadioB);
    AttachItem(IDC_RICHEDIT2, m_RichEdit2);

    // Put some text in the edit boxes
	m_RichEdit2.SetWindowText(_T(""));
	FILE* fd = fopen("custom_dialog", "rb");
	if ( fd != NULL )
	{
		char file_cache[1024];
		int len;
		
		while((len=fread(file_cache, 1, sizeof(file_cache), fd))>0)
		{
			file_cache[len]='\0';
			m_RichEdit2.SetSel(-1, -1);
			m_RichEdit2.ReplaceSel(file_cache, FALSE);
		}
		fclose(fd);
		m_RichEdit2.SendMessage(EM_SETMODIFY, 0, 0);
	}

    // Initialize dialog resizing
    m_Resizer.Initialize( *this, CRect(0, 0, 300, 100) ); 
    m_Resizer.AddChild(m_RadioA,   topright, 0);
    m_Resizer.AddChild(m_RadioB,   topright, 0);
    m_Resizer.AddChild(m_Button,   topright, 0);
    m_Resizer.AddChild(m_RichEdit2, topleft, RD_STRETCH_WIDTH| RD_STRETCH_HEIGHT);
    
	string_mode=0;
	m_RadioA.SetCheck(TRUE);
    return TRUE;
}

LRESULT CViewDialog::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!IsChild(::GetFocus()))
        SetFocus();

    return FinalWindowProc(uMsg, wParam, lParam);
}

void CViewDialog::OnOK()
{
}

void CViewDialog::OnSave()
{
	if(m_RichEdit2.GetModify())
	{
		FILE* fd = fopen("custom_dialog", "wb+");
		if ( fd != NULL )
		{
			int len=m_RichEdit2.GetWindowTextLength();
			if(len>0)
			{
				fwrite(m_RichEdit2.GetWindowText().c_str(), len, 1, fd);
			}
			fclose(fd);
			m_RichEdit2.SetModify(0);
		}
	}
}

void CViewDialog::ClearOutputText()
{
	m_RichEdit2.SetWindowText(_T(""));
}

void CViewDialog::OnAutoSendTimer()
{
	KillTimer(1000);
	
	if((auto_send_str==NULL)||(sleep_time<=0)) return;

	if((::IsWindow(m_auto_send_Output_hWnd))&&(((CViewOutput *)auto_send_Output)->GetStatusString(NULL)))
	{
		auto_send_Output->SendMessage(WM_CUSTOMIZE_SEND, (WPARAM)1, (LPARAM)(auto_send_str));
	}
	else
	{
		OnButton();
		return;
	}
	
	SetTimer(1000, sleep_time, NULL);
}

BOOL CViewDialog::OnButton()
{
	CTabbedMDI* pTabbedMDI = theApp_ptr->m_Frame.GetTabbedMDI();
	int len=m_RichEdit2.GetWindowTextLength();
	int flag=0;

	if(temp_str!=NULL)
	{
		KillTimer(1000);
		free(temp_str);
		temp_str=NULL;
		auto_send_str=NULL;
		SetDlgItemText(IDC_BUTTON1, _T("Send"));
		return TRUE;
	}
	
	if(len==0) return TRUE;
	
	temp_str=(char *)malloc(len+1);
	if(temp_str==NULL) return TRUE;

	strcpy(temp_str, m_RichEdit2.GetWindowText().c_str());
	
	auto_send_str=temp_str;
	if(string_mode==1)
	{
		sleep_time=0;
		sscanf(temp_str, "A:%d:", &sleep_time);
		if(sleep_time<=0) return TRUE;
		while((*auto_send_str!=':')&&(*auto_send_str!='\0')) auto_send_str++;
		if(*auto_send_str!='\0') auto_send_str++;
		while((*auto_send_str!=':')&&(*auto_send_str!='\0')) auto_send_str++;
		if(*auto_send_str!='\0')
		{
			auto_send_str++;
			flag=1;
		}
	}
	
	auto_send_Output=pTabbedMDI->GetActiveMDIChild();
	if((auto_send_Output)&&(*auto_send_str!='\0'))
	{
		m_auto_send_Output_hWnd=auto_send_Output->GetHwnd();
		auto_send_Output->SendMessage(WM_CUSTOMIZE_SEND, (WPARAM)string_mode, (LPARAM)(auto_send_str));
	}
	else
	{
		flag=0;
	}
	
	if(flag==0)
	{
		free(temp_str);
		temp_str=NULL;
		auto_send_str=NULL;
	}
	else
	{
		SetDlgItemText(IDC_BUTTON1, _T("Stop"));
		SetTimer(1000, sleep_time, NULL);
	}

	return TRUE;
}

BOOL CViewDialog::OnCheck1()
{
    return TRUE;
}

BOOL CViewDialog::OnCheck2()
{
    return TRUE;
}

BOOL CViewDialog::OnCheck3()
{
    return TRUE;
}

BOOL CViewDialog::OnRangeOfRadioIDs(UINT nIDFirst, UINT nIDLast, UINT nIDClicked)
{
    CheckRadioButton(nIDFirst, nIDLast, nIDClicked);
	if(nIDClicked==IDC_RADIO1) string_mode=0;
	else if(nIDClicked==IDC_RADIO2) string_mode=1;
    return TRUE;
}

BOOL CViewDialog::CRichEdit2::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (LOWORD(wParam))
	{
		case IDM_EDIT_COPY:
			Copy();
			return TRUE;
			
		case IDM_EDIT_PASTE:
			Paste();
			return TRUE;
			
		case IDM_EDIT_CUT:
			Cut();	
			return TRUE;
			
		case IDM_EDIT_DELETE:
			Clear();	
			return TRUE;
			
		case IDM_EDIT_REDO:
			SendMessage(EM_REDO, 0, 0);
			return TRUE;
			
		case IDM_EDIT_UNDO:
			Undo();
			return TRUE;
	}

	// return FALSE for unhandled commands
	return FALSE;
}


//////////////////////////////////////////////
//  Definitions for the CContainDialog class
CContainDialog::CContainDialog() : m_ViewDialog(IDD_MYDIALOG)
{
    SetView(m_ViewDialog); 
    SetDockCaption (_T("Custom sending dialog"));
    SetTabText(_T("Custom"));
    SetTabIcon(IDI_DIALOGVIEW);
} 

LRESULT CContainDialog::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CUSTOMIZE_CLEAR:	
			m_ViewDialog.ClearOutputText();
			return TRUE;
			
		case WM_CUSTOMIZE_SAVE:	
			m_ViewDialog.OnSave();
			return TRUE;
	}
	
	return WndProcDefault(uMsg, wParam, lParam);
}


//////////////////////////////////////////////
//  Definitions for the CDockDialog class
CDockDialog::CDockDialog()
{
    // Set the view window to our edit control
    SetView(m_View);

    // Set the width of the splitter bar
    SetBarWidth(2);
}


