#ifndef VIEW_H
#define VIEW_H
// view.h : ͷ�ļ�
//

#include "ClientModel.h"
#include "DragEdit.h"
#include "afxwin.h"
#include "afxcmn.h"

class ClientPresenter;

// ClientView �Ի���
class ClientView : public CDialogEx
{
public:
	enum { IDD = IDD_CLIENT_DIALOG };
    ClientView(CWnd* pParent = NULL);	// ��׼���캯��
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
    
public:
    ClientPresenter* p_presenter;
    CEdit m_recvEdit;
    CEdit m_sendEdit;
    CDragEdit m_sendFileEdit;
    CIPAddressCtrl m_IPAdress;

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnBnClickedButton1();//�����ӷ������� ��ť
    afx_msg void OnBnClickedButton2();//�����͡� ��ť
    afx_msg void OnBnClickedButton3();// �������ļ��� ��ť
    afx_msg void OnBnClickedButton4();// ���Ͽ����ӡ� ��ť
    
    
};




class ClientPresenter:public CallBack{
private:
    ClientModel* p_model;
    CString m_strRecvText;
    bool m_bConnected = FALSE;
public:   
    ClientView* p_view;

    ClientPresenter();   
    void connectToServer();
    virtual void OnConnectSuccess();
    void sendMessage(); 
    virtual void OnRecvMessage(CString);
    void sendFile();
    virtual void OnRecvFile(CString);
    void closeConnect();
};


#endif 