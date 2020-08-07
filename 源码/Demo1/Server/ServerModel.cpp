#include "stdafx.h"
#include "Server.h"
#include "ServerModel.h"
#include "ServerView_ServerPresenter.h"
#include <string>
void CString2Char(CString str, char ch[]){
    int i;
    char *tmpch;
    int wLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);//�õ�Char�ĳ���
    tmpch = new char[wLen + 1];                                             //��������ĵ�ַ��С
    WideCharToMultiByte(CP_ACP, 0, str, -1, tmpch, wLen, NULL, NULL);       //��CStringת����char*

    for (i = 0; tmpch[i] != '\0'; i++) ch[i] = tmpch[i];
    ch[i] = '\0';
}
/*ͨ���׽���

*/
CCommuSocket::CCommuSocket(CallBack &callback):m_callback(callback){
    m_pBuffer = new char[1024];
}
void CCommuSocket::OnReceive(int nErrorCode){
    m_intLen = Receive(m_pBuffer, sizeof(m_pBuffer), 0);
    m_pBuffer[m_intLen] = 0;
    CString strMessage(m_pBuffer);
    while (m_intLen > 0){
        memset(m_pBuffer, 0, sizeof(m_pBuffer));
        m_intLen = Receive(m_pBuffer, sizeof(m_pBuffer), 0);
        m_pBuffer[m_intLen] = 0;
        strMessage += m_pBuffer;
    }
    if (strMessage == _T("//�Ͽ�����")){
        Close();
        m_callback.OnConnectClose();
    }
    else{
        //��CString תΪchar*
        char* szMessage = new char[1024];
        CString2Char(strMessage, szMessage);

        //����Ϣ�������ݿ�
        char *errmsg;
        char sql[1224];
        sprintf(sql, "insert into messageLog(id,sender,time,message) values(null,'client ',datetime('now','localtime'),'%s');", szMessage);
        if (SQLITE_OK != sqlite3_exec(m_db, sql, NULL, NULL, &errmsg))
        {
            CString err(errmsg);
            AfxMessageBox(_T("�������ݿ�ʧ�ܣ�")+err);
        }
        
        m_callback.OnRecvMessage(strMessage);
    }
    

    CAsyncSocket::OnReceive(nErrorCode);
}
void CCommuSocket::OnSend(int nErrorCode){

    Send(m_pBuffer, m_intLen, 0);
    m_intLen = 0;
    memset(m_pBuffer, 0, sizeof(m_pBuffer));
    AsyncSelect(FD_READ);
    CAsyncSocket::OnSend(nErrorCode);
}

CFileCommuSocket::CFileCommuSocket(CallBack& callback) :CCommuSocket(callback){}
void CFileCommuSocket::OnReceive(int nErrorCode){
    char fName[64] = { 0 };
    Receive((char*)&fName, sizeof(fName), 0);//�����ļ���
    long long siz;
    Receive((char*)&siz, sizeof(siz), 0);//�����ļ�����
    siz = siz / 1024;
    std::string strPath = "FileCache/";
    strPath += fName;
    fp = fopen(const_cast<char*>(strPath.c_str()), "wb+");//�����ļ�
    long long int index = 0;
    int num;
    while (index <= siz){//�ֿ�����ļ�
        num = Receive(m_pBuffer, 1024, 0);
        if (num <= 0)
            break;
        fwrite(m_pBuffer, (int)num, 1, fp);
        index++;
    }
    fclose(fp);
    //֪ͨpresenter �յ��ļ���
    CString s(fName);
    m_callback.OnRecvFile(s);

    CAsyncSocket::OnReceive(nErrorCode);
}
void CFileCommuSocket::OnSend(int nErrorCode){
    char fName[64] = { 0 };
    //���ļ�
    char name[32], ext[16];
    _splitpath(m_pBuffer, NULL, NULL, name, ext);
    strcat_s(name, 32, ext);
    strcpy_s(fName, 32, name);
    fp = fopen(m_pBuffer, "rb");
    //�����ļ�
    Send((char*)&fName, sizeof(fName), 0);//�����ļ���
    fseek(fp, 0, SEEK_END);
    long long siz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    Send((CHAR*)&siz, sizeof(siz), 0);//�����ļ�����
    siz = siz / 1024;
    long long index = 0;
    int num;
    while (1) { // �ֿ鷢���ļ�
        num = fread(m_pBuffer, 1, 1024, fp);
        if (num == 0)
            break;
        Send(m_pBuffer, num, 0);
        index++;
    }
    fclose(fp);
    AsyncSelect(FD_READ);
    CAsyncSocket::OnSend(nErrorCode);
}

/*�����׽���

*/
CListenSocket::CListenSocket(CallBack &callback) :m_callback(callback){}
void CListenSocket::OnAccept(int nErrorCode){
    //FD_ACCEPT�¼�����Ϣ��Ӧ����
    CCommuSocket* pSocket = new CCommuSocket(m_callback);
    
    if (Accept(*pSocket)){
        //���ӳɹ�     
        m_pSocket = pSocket;  
        m_callback.OnConnectSuccess();//���ûص�����֪ͨPresenter     
        pSocket->AsyncSelect(FD_READ);//����ͨ��Socket��Read����������
    }
    else{
        delete pSocket;
    }


    //�������ݿ⣬����
    if (SQLITE_OK != sqlite3_open("messaageLog.db", &pSocket->m_db)){
        AfxMessageBox(_T("�������ݿ�ʧ�ܣ�"));
        return;
    }
    /*else{
    AfxMessageBox(_T("�������ݿ�ɹ���"));
    }*/
    //����
    char *errmsg = NULL;
    char *sql;
    sql = "create table if not exists messageLog(id integer primary key autoincrement,sender text,time datetime,message text);";
    if (SQLITE_OK != sqlite3_exec(pSocket->m_db, sql, NULL, NULL, &errmsg))
    {
        CString err(errmsg);
        AfxMessageBox(_T("����ʧ�ܣ�") + err);
        return;
    }
    /*else{
    AfxMessageBox(_T("����ɹ���"));
    }*/
    CAsyncSocket::OnAccept(nErrorCode);
}

CFileListenSocket::CFileListenSocket(CallBack& callback) :CListenSocket(callback){}
void CFileListenSocket::OnAccept(int nErrorCode){
    CFileCommuSocket* pSocket = new CFileCommuSocket(m_callback);

    if (Accept(*pSocket)){
        //���ӳɹ�     
        m_pFileSocket = pSocket;
        //todo �ĳ�OnfileConnectSuccess   
        pSocket->AsyncSelect(FD_READ);//����ͨ��Socket��Read����������
    }
    else{
        delete pSocket;
    }

    CAsyncSocket::OnAccept(nErrorCode);
}


/*ServerModel

*/
void ServerModel::startServer(int intPort, CallBack &callback){

    //������Ϣ����ͨ��
    m_ListenSocket = new CListenSocket(callback);
    if (m_ListenSocket->m_hSocket == INVALID_SOCKET){
        //���������׽���,����FD_ACCEPT�¼�
        if (!m_ListenSocket->Create(intPort, SOCK_STREAM, FD_ACCEPT)){
            AfxMessageBox(_T("Socket����ʧ�ܣ�"));
            m_ListenSocket->Close();
            PostQuitMessage(0);//�˳�����
            return;
        }
        //���Կ�ʼ����        
    }
    if (!m_ListenSocket->Listen(1)){
        int nErrorCode = m_ListenSocket->GetLastError();
        if (nErrorCode != WSAEWOULDBLOCK){//��������̱߳�����
            AfxMessageBox(_T("����ʧ�ܣ�"));
            m_ListenSocket->Close();
            PostQuitMessage(0);
            return;
        }
    }


    //�ļ�����ͨ��
    m_FileListenSocket = new CFileListenSocket(callback);
    if (m_FileListenSocket->m_hSocket == INVALID_SOCKET){
        //���������׽���,����FD_ACCEPT�¼�
        if (!m_FileListenSocket->Create(intPort+1, SOCK_STREAM, FD_ACCEPT)){//������һ���˿�
            AfxMessageBox(_T("Socket����ʧ�ܣ�"));
            m_FileListenSocket->Close();
            PostQuitMessage(0);//�˳�����
            return;
        }
        //���Կ�ʼ����        
    }
    if (!m_FileListenSocket->Listen(1)){
        int nErrorCode = m_FileListenSocket->GetLastError();
        if (nErrorCode != WSAEWOULDBLOCK){//��������̱߳�����
            AfxMessageBox(_T("����ʧ�ܣ�"));
            m_FileListenSocket->Close();
            PostQuitMessage(0);
            return;
        }
    }
}
void ServerModel::sendMessage(CString strText){
    
    //��CString תΪchar*
    char* szMessage = new char[1024];
    CString2Char(strText, szMessage);
    int nLen = strlen(szMessage);

    // ����Ϣ�������ݿ�
    char *errmsg;
    char sql[1224];
    sprintf(sql, "insert into messageLog(id,sender,time,message) values(null,'server',datetime('now','localtime'),'%s');", szMessage);
    if (SQLITE_OK != sqlite3_exec(m_ListenSocket->m_pSocket->m_db, sql, NULL, NULL, &errmsg))
    {
        AfxMessageBox(_T("�������ݿ�ʧ�ܣ�"));
    }

    //���ݸ�socket���ͣ����͵���
    m_ListenSocket->m_pSocket->m_pBuffer = szMessage;
    m_ListenSocket->m_pSocket->m_intLen = nLen;
    m_ListenSocket->m_pSocket->AsyncSelect(FD_WRITE);//����ͨ���׽��ֵ�OnSend����
}
void ServerModel::sendFile(CString strFilePath){
    //��CString תΪchar*
    char* szFileName = new char[1024];
    CString2Char(strFilePath, szFileName);
    int nLen = strlen(szFileName);
    //�����ļ�������Ϣ
    m_FileListenSocket->m_pFileSocket->m_pBuffer = szFileName;
    m_FileListenSocket->m_pFileSocket->m_intLen = nLen;
    m_FileListenSocket->m_pFileSocket->AsyncSelect(FD_WRITE);
}

CString strResult("");
int sqlCallback(void *data, int argc, char **argv, char **azColName){
    
    for (int i = 1; i < argc; i++){
        CString s(argv[i]);
        strResult += s + ' ';
    }
    strResult += "\r\n";
    return 0;
}

CString ServerModel::showMessageLog(){
    strResult = "";
    char *errmsg;
    char* sql = "select * from messageLog";
    if (SQLITE_OK != sqlite3_exec(m_ListenSocket->m_pSocket->m_db, sql, sqlCallback, NULL, &errmsg))
    {
        AfxMessageBox(_T("���ݿ��ѯʧ�ܣ�"));
    }
    return strResult;
}
