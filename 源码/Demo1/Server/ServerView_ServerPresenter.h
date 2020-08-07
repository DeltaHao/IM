
// ServerView_ServerPresenter.h : ͷ�ļ�
//

#pragma once
#include "ServerModel.h"
#include "DragEdit.h"
#include "MessageLogDlg.h"
class ServerPresenter;

// ServerView �Ի���
class ServerView : public CDialogEx
{
public:
	enum { IDD = IDD_SERVER_DIALOG };
    ServerView(CWnd* pParent = NULL);	// ��׼���캯��
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();
    afx_msg void OnBnClickedButton3();
    afx_msg void OnBnClickedButton4();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
    ServerPresenter* p_presenter;
    CEdit m_recvEdit;
    CEdit m_sendEdit;
    CDragEdit m_sendFileEdit;
    CMessageLogDlg* m_messageLogDlg;     
};


class ServerPresenter:public CallBack{
private:
    ServerModel* p_model;   
    CString m_strRecvText;
    bool m_bConnected = FALSE;
    bool m_bListening = FALSE;
public:
    ServerView* p_view;
    ServerPresenter();  
    void startServer();
    virtual void OnConnectSuccess();
    void sendMessage();
    virtual void OnRecvMessage(CString);
    void sendFile();
    virtual void OnRecvFile(CString);
    virtual void OnConnectClose();
    void showMessageLog();
};