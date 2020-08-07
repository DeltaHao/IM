
// view.cpp : ʵ���ļ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
    m_recvEdit.SetWindowText(_T("----------------δ���ӷ�����----------------"));
    m_sendFileEdit.SetWindowText(_T("����ק�ļ����������ϴ���"));
    m_IPAdress.SetAddress(127, 0, 0, 1);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void ClientView::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
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
    //���س���������ʱ��������Ϣ
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN){
        p_presenter->sendMessage();
        return TRUE;
    }
    else
        return CDialog::PreTranslateMessage(pMsg);
}

void ClientView::OnBnClickedButton1()
{
    //���ӷ�����
    p_presenter->connectToServer();
}

void ClientView::OnBnClickedButton2()
{
    //������Ϣ
    p_presenter->sendMessage();
}

void ClientView::OnBnClickedButton3()
{
    // �����ļ�
    p_presenter->sendFile();
}

void ClientView::OnBnClickedButton4()
{
    // �Ͽ�����
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
        AfxMessageBox(_T("�Ѿ���������������"));
    }
}

void ClientPresenter::OnConnectSuccess(){
    m_strRecvText += _T("\r\n---------------�ɹ����ӷ�����---------------");
    p_view->m_recvEdit.SetWindowTextW(m_strRecvText);
    m_bConnected = TRUE;
}

void ClientPresenter::sendMessage(){
    if (m_bConnected){
        CString strText;
        p_view->m_sendEdit.GetWindowText(strText);
        if (!strText.GetLength()){
            AfxMessageBox(_T("�������ݲ���Ϊ�գ�"));
            return;
        }

        m_strRecvText += _T("\r\nClient:") + strText;
        p_view->m_recvEdit.SetWindowText(m_strRecvText);
        p_view->m_sendEdit.SetWindowText(_T(""));

        p_model->sendMessage(strText);
        
    }
    else{
        AfxMessageBox(_T("����ʧ�ܣ�û�����ӵ���������"));
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

        m_strRecvText += _T("\r\n��������������ļ���") + strFilePath;
        p_view->m_recvEdit.SetWindowText(m_strRecvText);
    }
    else{
        AfxMessageBox(_T("����ʧ�ܣ�û�����ӵ���������"));
    }
}

void ClientPresenter::OnRecvFile(CString strFileName){
    m_strRecvText += _T("\r\n���յ��������������ļ���") + strFileName;
    p_view->m_recvEdit.SetWindowTextW(m_strRecvText);
}

void ClientPresenter::closeConnect(){
    if (m_bConnected){
        p_model->sendMessage(_T("//�Ͽ�����"));
        m_bConnected = FALSE;
        m_strRecvText += _T("\r\n---------------�ɹ��Ͽ�������---------------");
        p_view->m_recvEdit.SetWindowText(m_strRecvText);   
    }
    else{
        AfxMessageBox(_T("��û�����ӵ���������"));
    }
}