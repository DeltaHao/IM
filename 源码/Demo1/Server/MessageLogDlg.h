#pragma once
#include "afxwin.h"


// CMessageLogDlg �Ի���

class CMessageLogDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMessageLogDlg)

public:
	CMessageLogDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CMessageLogDlg();

// �Ի�������
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
    CEdit m_mainEdit;
};
