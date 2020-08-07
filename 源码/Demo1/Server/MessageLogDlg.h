#pragma once
#include "afxwin.h"


// CMessageLogDlg 对话框

class CMessageLogDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMessageLogDlg)

public:
	CMessageLogDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMessageLogDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    CEdit m_mainEdit;
};
