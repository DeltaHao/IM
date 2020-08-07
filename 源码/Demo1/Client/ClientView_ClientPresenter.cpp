
// view.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "afxdialogex.h"
#include "ClientView_ClientPresenter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ClientView 

ClientView::ClientView(CWnd* pParent /*=NULL*/)
	: CDialogEx(ClientView::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    p_presenter = new ClientPresenter();
    p_presenter->p_view = this;
}

void ClientView::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_recvEdit);
    DDX_Control(pDX, IDC_EDIT2, m_sendEdit);
    DDX_Control(pDX, IDC_EDIT3, m_sendFileEdit);
    DDX_Control(pDX, IDC_IPADDRESS1, m_IPAdress);
}

BEGIN_MESSAGE_MAP(ClientView, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &ClientView::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &ClientView::OnBnClickedButton2)
    ON_BN_CLICKED(IDC_BUTTON3, &ClientView::OnBnClickedButton3)
    ON_BN_CLICKED(IDC_BUTTON4, &ClientView::OnBnClickedButton4)
END_MESSAGE_MAP()

BOOL ClientView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
    m_recvEdit.SetWindowText(_T("----------------未连接服务器----------------"));
    m_sendFileEdit.SetWindowText(_T("（拖拽文件到这里以上传）"));
    m_IPAdress.SetAddress(127, 0, 0, 1);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void ClientView::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR ClientView::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL ClientView::PreTranslateMessage(MSG* pMsg){
    //当回车键被按下时，发送消息
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN){
        p_presenter->sendMessage();
        return TRUE;
    }
    else
        return CDialog::PreTranslateMessage(pMsg);
}

void ClientView::OnBnClickedButton1()
{
    //连接服务器
    p_presenter->connectToServer();
}

void ClientView::OnBnClickedButton2()
{
    //发送消息
    p_presenter->sendMessage();
}

void ClientView::OnBnClickedButton3()
{
    // 发送文件
    p_presenter->sendFile();
}

void ClientView::OnBnClickedButton4()
{
    // 断开连接
    p_presenter->closeConnect();
}


//ClientPresenter 

ClientPresenter::ClientPresenter(){
    p_model = new ClientModel();
}

void ClientPresenter::connectToServer(){
    if (!m_bConnected){
        BYTE nf1, nf2, nf3, nf4;
        p_view->m_IPAdress.GetAddress(nf1, nf2, nf3, nf4);
        CString s;
        s.Format(_T("%d.%d.%d.%d"), nf1, nf2, nf3, nf4);
        p_model->connectToServer(s, 12000, *this);
    }
    else{
        AfxMessageBox(_T("已经连接至服务器！"));
    }
}

void ClientPresenter::OnConnectSuccess(){
    m_strRecvText += _T("\r\n---------------成功连接服务器---------------");
    p_view->m_recvEdit.SetWindowTextW(m_strRecvText);
    m_bConnected = TRUE;
}

void ClientPresenter::sendMessage(){
    if (m_bConnected){
        CString strText;
        p_view->m_sendEdit.GetWindowText(strText);
        if (!strText.GetLength()){
            AfxMessageBox(_T("发送内容不能为空！"));
            return;
        }

        m_strRecvText += _T("\r\nClient:") + strText;
        p_view->m_recvEdit.SetWindowText(m_strRecvText);
        p_view->m_sendEdit.SetWindowText(_T(""));

        p_model->sendMessage(strText);
        
    }
    else{
        AfxMessageBox(_T("发送失败，没有连接到服务器！"));
    }
}

void ClientPresenter::OnRecvMessage(CString strMessage){
    m_strRecvText += _T("\r\nServer:") + strMessage;
    p_view->m_recvEdit.SetWindowTextW(m_strRecvText);
}

void ClientPresenter::sendFile(){
    if (m_bConnected){
        CString strFilePath;
        p_view->m_sendFileEdit.GetWindowText(strFilePath);
        p_model->sendFile(strFilePath);

        m_strRecvText += _T("\r\n已向服务器发送文件：") + strFilePath;
        p_view->m_recvEdit.SetWindowText(m_strRecvText);
    }
    else{
        AfxMessageBox(_T("发送失败，没有连接到服务器！"));
    }
}

void ClientPresenter::OnRecvFile(CString strFileName){
    m_strRecvText += _T("\r\n接收到服务器发来的文件：") + strFileName;
    p_view->m_recvEdit.SetWindowTextW(m_strRecvText);
}

void ClientPresenter::closeConnect(){
    if (m_bConnected){
        p_model->sendMessage(_T("//断开连接"));
        m_bConnected = FALSE;
        m_strRecvText += _T("\r\n---------------成功断开服务器---------------");
        p_view->m_recvEdit.SetWindowText(m_strRecvText);   
    }
    else{
        AfxMessageBox(_T("还没有连接到服务器！"));
    }
}