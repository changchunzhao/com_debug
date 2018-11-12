///////////////////////////////////////////////////////
// Text.h - Declaration of the CViewText, CContainText, 
//          and CDockText classes

#ifndef TEXT_H
#define TEXT_H


#include <richedit.h>


// Declaration of the CViewText class
class CViewText : public CRichEdit
{
public:
    CViewText();
    virtual ~CViewText();
    void OnEditCopy();
    void OnEditPaste();
    void OnEditCut();
    void OnEditDelete();
    void OnEditRedo();
    void OnEditUndo();
	void OnBUTTONDBLCLK();
	void OnSave();
	void SetFileName(char *file_name_string);
	char file_name[128];

protected:
    virtual void OnAttach();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    void PreCreate(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void set_send_flag(int line_first_char);
	void clear_send_flag(void);
	void FlagOnPain(void);

private:
    CFont m_Font;
	int pre_flag_y;
	int pre_flag_color;
	int readonly;
};

// Declaration of the CContainText class
class CContainText : public CDockContainer
{
public:
    CContainText();
    ~CContainText() {}
	void set_CViewText_file_name(char *file_name);
	char *get_CViewText_file_name();
	
protected:
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CViewText m_ViewText;
};

// Declaration of the CDockText class
class CDockText : public CDocker
{
public:
    CDockText();
    virtual ~CDockText() {}
	void set_CContainText_file_name(char *file_name);

private:
    CContainText m_View;

};


#endif // TEXT_H

