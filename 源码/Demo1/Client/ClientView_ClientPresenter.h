#ifndef VIEW_H
#define VIEW_H
// view.h : 头文件
//

#include "ClientModel.h"
#include "DragEdit.h"
#include "afxwin.h"
#include "afxcmn.h"

class ClientPresenter;

// ClientView 对话框
class ClientView : public CDialogEx
{
public:
	enum { IDD = IDD_CLIENT_DIALOG };
    ClientView(CWnd* pParent = NULL);	// 标准构造函数
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
    afx_msg void OnBnClickedButton1();//“连接服务器” 按钮
    afx_msg void OnBnClickedButton2();//“发送” 按钮
    afx_msg void OnBnClickedButton3();// “发送文件” 按钮
    afx_msg void OnBnClickedButton4();// “断开连接” 按钮
    
    
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