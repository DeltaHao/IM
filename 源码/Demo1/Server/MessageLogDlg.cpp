// MessageLogDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "MessageLogDlg.h"
#include "afxdialogex.h"


// CMessageLogDlg �Ի���

IMPLEMENT_DYNAMIC(CMessageLogDlg, CDialogEx)

CMessageLogDlg::CMessageLogDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMessageLogDlg::IDD, pParent)
{

}

CMessageLogDlg::~CMessageLogDlg()
{
}

void CMessageLogDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_mainEdit);
}


BEGIN_MESSAGE_MAP(CMessageLogDlg, CDialogEx)
END_MESSAGE_MAP()


// CMessageLogDlg ��Ϣ�������
