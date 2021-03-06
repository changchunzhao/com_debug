/////////////////////////////////////////////
// Mainfrm.h

#ifndef MAINFRM_H
#define MAINFRM_H


#include "MyTabbedMDI.h"

enum DockIDs
{
    ID_DOCK_CLASSES1 = 1,
    ID_DOCK_CLASSES2 = 2,
    ID_DOCK_FILES1 = 3,
    ID_DOCK_FILES2 = 4,
    ID_DOCK_OUTPUT1 = 5,
    ID_DOCK_OUTPUT2 = 6,
    ID_DOCK_TEXT1 = 7,
    ID_DOCK_TEXT2 = 8,
    ID_DOCK_BROWSER = 9,
    ID_DOCK_RECT = 10,
    ID_DOCK_DIALOG = 11
};

// Declaration of the CMainFrame class
class CMainFrame : public CDockFrame
{
public:
    CMainFrame();
    virtual ~CMainFrame();

    void HideSingleContainerTab(BOOL HideSingle);
    BOOL OnCloseMDIs();
    BOOL OnCloseDockers();
    BOOL OnContainerTabsAtTop();
    BOOL OnDefaultLayout();
    BOOL OnFileNew();
    BOOL OnFileExit();
    BOOL OnFileNewSimple();
    BOOL OnFileNewRect();
    BOOL OnFileNewList();
    BOOL OnFileNewText();
    BOOL OnFileNewTree();
    BOOL OnHideSingleTab();
    BOOL OnMDITabsAtTop();
    void LoadDefaultDockers();
    void LoadDefaultMDIs();
    void SetContainerTabsAtTop(BOOL bTop);
    void SetMDITabsAtTop(BOOL bTop);
	void OnFileOpen();
	void OnFileSave();
	CTabbedMDI* GetTabbedMDI(){return &m_MyTabbedMDI;}

    
protected:
    virtual CDocker* NewDockerFromID(int idDock);
    virtual BOOL    OnCommand(WPARAM wParam, LPARAM lParam);
    virtual int     OnCreate(CREATESTRUCT& cs);
    virtual LRESULT OnDockActivated(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void    OnInitialUpdate();
    virtual LRESULT OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void    OnMenuUpdate(UINT nID);
    virtual void    PreCreate(CREATESTRUCT& cs);
    virtual BOOL    SaveRegistrySettings();
    virtual void    SetupMenuIcons();
    virtual void    SetupToolBar();
    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnFileNewOutput(char *connect_str);
	void OnFileNewPSC(char *file_name);
	void OnStatusChange();
	void OnClearCommand(WPARAM wParam);

private:
    CMyTabbedMDI m_MyTabbedMDI;
    BOOL m_IsContainerTabsAtTop;
    BOOL m_IsHideSingleTab;
    BOOL m_IsMDITabsAtTop;
    CDocker* m_pActiveDocker;
};

#endif //MAINFRM_H

