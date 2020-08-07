
// ServerView_ServerPresenter.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "ServerView_ServerPresenter.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ServerView 对话框


ServerView::ServerView(CWnd* pParent /*=NULL*/)
	: CDialogEx(ServerView::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    p_presenter = new ServerPresenter();
    p_presenter->p_view = this;
}

void ServerView::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_recvEdit);
    DDX_Control(pDX, IDC_EDIT2, m_sendEdit);
    DDX_Control(pDX, IDC_EDIT3, m_sendFileEdit);
}

BEGIN_MESSAGE_MAP(ServerView, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &ServerView::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &ServerView::OnBnClickedButton2)
    ON_BN_CLICKED(IDC_BUTTON3, &ServerView::OnBnClickedButton3)
    ON_BN_CLICKED(IDC_BUTTON4, &ServerView::OnBnClickedButton4)
END_MESSAGE_MAP()

// ServerView 消息处理程序

BOOL ServerView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
    m_recvEdit.SetWindowText(_T("----------------服务器未打开----------------"));
    m_sendFileEdit.SetWindowText(_T("（拖拽文件到这里以上传）"));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void ServerView::OnPaint()
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

HCURSOR ServerView::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL ServerView::PreTranslateMessage(MSG* pMsg){
    //当回车键被按下时，发送消息
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN){
        p_presenter->sendMessage();
        return TRUE;
    }
    else
        return CDialog::PreTranslateMessage(pMsg);
}

void ServerView::OnBnClickedButton1()
{
    //发送消息
    p_presenter->sendMessage();
}

void ServerView::OnBnClickedButton2()
{
    // 运行服务器
    p_presenter->startServer();
}

void ServerView::OnBnClickedButton3()
{
    // 发送文件
    p_presenter->sendFile();
}

void ServerView::OnBnClickedButton4()
{
    //查看聊天记录
    p_presenter->showMessageLog();
}


//ServerPresenter 

ServerPresenter::ServerPresenter(){
    p_model = new ServerModel();
}

void ServerPresenter::startServer(){
    if (!m_bListening){
        m_bListening = TRUE;
        m_strRecvText += _T("------------------正在监听--------------------");
        p_view->m_recvEdit.SetWindowText(m_strRecvText);
        p_model->startServer(12000, *this);
    }
    else {
        AfxMessageBox(_T("已经打开服务器！"));
    }
}

void ServerPresenter::OnConnectSuccess(){
    m_strRecvText += _T("\r\n---------------客户端成功连接----------------");
    p_view->m_recvEdit.SetWindowText(m_strRecvText);
    m_bConnected = TRUE;
}

void ServerPresenter::sendMessage(){
    if (m_bConnected){
        CString strText;
        p_view->m_sendEdit.GetWindowText(strText);
        if (!strText.GetLength()){
            AfxMessageBox(_T("发送内容不能为空！"));
            return;
        }

        m_strRecvText += _T("\r\nServer:") + strText;
        p_view->m_recvEdit.SetWindowText(m_strRecvText);
        p_view->m_sendEdit.SetWindowText(_T(""));

        p_model->sendMessage(strText);
        
    }
    else{
        AfxMessageBox(_T("发送失败，没有客户端连接！"));
    }
}

void ServerPresenter::OnRecvMessage(CString strMessage){
    m_strRecvText += _T("\r\nClient:") + strMessage;
    p_view->m_recvEdit.SetWindowTextW(m_strRecvText);
}

void ServerPresenter::sendFile(){
    if (m_bConnected){
        CString strFilePath;
        p_view->m_sendFileEdit.GetWindowText(strFilePath);
        p_model->sendFile(strFilePath);

        m_strRecvText += _T("\r\n已向客户端发送文件：") + strFilePath;
        p_view->m_recvEdit.SetWindowText(m_strRecvText);
    }
    else{
        AfxMessageBox(_T("发送失败，没有客户端连接！"));
    }
}

void ServerPresenter::OnRecvFile(CString strFileName){
    m_strRecvText += _T("\r\n接收到客户端发来的文件：") + strFileName;
    p_view->m_recvEdit.SetWindowTextW(m_strRecvText);
}

void ServerPresenter::OnConnectClose(){
    m_strRecvText += _T("\r\n--------------客户端已断开连接--------------");
    p_view->m_recvEdit.SetWindowTextW(m_strRecvText);
    m_bConnected = false;
}

void ServerPresenter::showMessageLog(){
    if (m_bConnected){
        p_view->m_messageLogDlg = new CMessageLogDlg();
        p_view->m_messageLogDlg->Create(IDD_DIALOG1);
        p_view->m_messageLogDlg->ShowWindow(SW_SHOWNORMAL);
        CString strText = p_model->showMessageLog();
        p_view->m_messageLogDlg->m_mainEdit.SetWindowTextW(strText);
    }
    else{
        AfxMessageBox(_T("没有客户端连接！"));
    }
    
}