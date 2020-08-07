#include "stdafx.h"
#include "Server.h"
#include "ServerModel.h"
#include "ServerView_ServerPresenter.h"
#include <string>
void CString2Char(CString str, char ch[]){
    int i;
    char *tmpch;
    int wLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);//得到Char的长度
    tmpch = new char[wLen + 1];                                             //分配变量的地址大小
    WideCharToMultiByte(CP_ACP, 0, str, -1, tmpch, wLen, NULL, NULL);       //将CString转换成char*

    for (i = 0; tmpch[i] != '\0'; i++) ch[i] = tmpch[i];
    ch[i] = '\0';
}
/*通信套接字

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
    if (strMessage == _T("//断开连接")){
        Close();
        m_callback.OnConnectClose();
    }
    else{
        //将CString 转为char*
        char* szMessage = new char[1024];
        CString2Char(strMessage, szMessage);

        //将消息存入数据库
        char *errmsg;
        char sql[1224];
        sprintf(sql, "insert into messageLog(id,sender,time,message) values(null,'client ',datetime('now','localtime'),'%s');", szMessage);
        if (SQLITE_OK != sqlite3_exec(m_db, sql, NULL, NULL, &errmsg))
        {
            CString err(errmsg);
            AfxMessageBox(_T("插入数据库失败！")+err);
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
    Receive((char*)&fName, sizeof(fName), 0);//接受文件名
    long long siz;
    Receive((char*)&siz, sizeof(siz), 0);//接受文件长度
    siz = siz / 1024;
    std::string strPath = "FileCache/";
    strPath += fName;
    fp = fopen(const_cast<char*>(strPath.c_str()), "wb+");//创建文件
    long long int index = 0;
    int num;
    while (index <= siz){//分块接收文件
        num = Receive(m_pBuffer, 1024, 0);
        if (num <= 0)
            break;
        fwrite(m_pBuffer, (int)num, 1, fp);
        index++;
    }
    fclose(fp);
    //通知presenter 收到文件了
    CString s(fName);
    m_callback.OnRecvFile(s);

    CAsyncSocket::OnReceive(nErrorCode);
}
void CFileCommuSocket::OnSend(int nErrorCode){
    char fName[64] = { 0 };
    //打开文件
    char name[32], ext[16];
    _splitpath(m_pBuffer, NULL, NULL, name, ext);
    strcat_s(name, 32, ext);
    strcpy_s(fName, 32, name);
    fp = fopen(m_pBuffer, "rb");
    //发送文件
    Send((char*)&fName, sizeof(fName), 0);//发送文件名
    fseek(fp, 0, SEEK_END);
    long long siz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    Send((CHAR*)&siz, sizeof(siz), 0);//发送文件长度
    siz = siz / 1024;
    long long index = 0;
    int num;
    while (1) { // 分块发送文件
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

/*监听套接字

*/
CListenSocket::CListenSocket(CallBack &callback) :m_callback(callback){}
void CListenSocket::OnAccept(int nErrorCode){
    //FD_ACCEPT事件的消息响应函数
    CCommuSocket* pSocket = new CCommuSocket(m_callback);
    
    if (Accept(*pSocket)){
        //连接成功     
        m_pSocket = pSocket;  
        m_callback.OnConnectSuccess();//利用回调函数通知Presenter     
        pSocket->AsyncSelect(FD_READ);//触发通信Socket的Read函数读数据
    }
    else{
        delete pSocket;
    }


    //连接数据库，建表
    if (SQLITE_OK != sqlite3_open("messaageLog.db", &pSocket->m_db)){
        AfxMessageBox(_T("连接数据库失败！"));
        return;
    }
    /*else{
    AfxMessageBox(_T("连接数据库成功！"));
    }*/
    //建表
    char *errmsg = NULL;
    char *sql;
    sql = "create table if not exists messageLog(id integer primary key autoincrement,sender text,time datetime,message text);";
    if (SQLITE_OK != sqlite3_exec(pSocket->m_db, sql, NULL, NULL, &errmsg))
    {
        CString err(errmsg);
        AfxMessageBox(_T("建表失败！") + err);
        return;
    }
    /*else{
    AfxMessageBox(_T("建表成功！"));
    }*/
    CAsyncSocket::OnAccept(nErrorCode);
}

CFileListenSocket::CFileListenSocket(CallBack& callback) :CListenSocket(callback){}
void CFileListenSocket::OnAccept(int nErrorCode){
    CFileCommuSocket* pSocket = new CFileCommuSocket(m_callback);

    if (Accept(*pSocket)){
        //连接成功     
        m_pFileSocket = pSocket;
        //todo 改成OnfileConnectSuccess   
        pSocket->AsyncSelect(FD_READ);//触发通信Socket的Read函数读数据
    }
    else{
        delete pSocket;
    }

    CAsyncSocket::OnAccept(nErrorCode);
}


/*ServerModel

*/
void ServerModel::startServer(int intPort, CallBack &callback){

    //文字消息传输通道
    m_ListenSocket = new CListenSocket(callback);
    if (m_ListenSocket->m_hSocket == INVALID_SOCKET){
        //创建监听套接字,激发FD_ACCEPT事件
        if (!m_ListenSocket->Create(intPort, SOCK_STREAM, FD_ACCEPT)){
            AfxMessageBox(_T("Socket创建失败！"));
            m_ListenSocket->Close();
            PostQuitMessage(0);//退出窗口
            return;
        }
        //可以开始监听        
    }
    if (!m_ListenSocket->Listen(1)){
        int nErrorCode = m_ListenSocket->GetLastError();
        if (nErrorCode != WSAEWOULDBLOCK){//如果不是线程被阻塞
            AfxMessageBox(_T("监听失败！"));
            m_ListenSocket->Close();
            PostQuitMessage(0);
            return;
        }
    }


    //文件传输通道
    m_FileListenSocket = new CFileListenSocket(callback);
    if (m_FileListenSocket->m_hSocket == INVALID_SOCKET){
        //创建监听套接字,激发FD_ACCEPT事件
        if (!m_FileListenSocket->Create(intPort+1, SOCK_STREAM, FD_ACCEPT)){//监听另一个端口
            AfxMessageBox(_T("Socket创建失败！"));
            m_FileListenSocket->Close();
            PostQuitMessage(0);//退出窗口
            return;
        }
        //可以开始监听        
    }
    if (!m_FileListenSocket->Listen(1)){
        int nErrorCode = m_FileListenSocket->GetLastError();
        if (nErrorCode != WSAEWOULDBLOCK){//如果不是线程被阻塞
            AfxMessageBox(_T("监听失败！"));
            m_FileListenSocket->Close();
            PostQuitMessage(0);
            return;
        }
    }
}
void ServerModel::sendMessage(CString strText){
    
    //将CString 转为char*
    char* szMessage = new char[1024];
    CString2Char(strText, szMessage);
    int nLen = strlen(szMessage);

    // 将消息存入数据库
    char *errmsg;
    char sql[1224];
    sprintf(sql, "insert into messageLog(id,sender,time,message) values(null,'server',datetime('now','localtime'),'%s');", szMessage);
    if (SQLITE_OK != sqlite3_exec(m_ListenSocket->m_pSocket->m_db, sql, NULL, NULL, &errmsg))
    {
        AfxMessageBox(_T("插入数据库失败！"));
    }

    //传递给socket发送，发送的是
    m_ListenSocket->m_pSocket->m_pBuffer = szMessage;
    m_ListenSocket->m_pSocket->m_intLen = nLen;
    m_ListenSocket->m_pSocket->AsyncSelect(FD_WRITE);//触发通信套接字的OnSend函数
}
void ServerModel::sendFile(CString strFilePath){
    //将CString 转为char*
    char* szFileName = new char[1024];
    CString2Char(strFilePath, szFileName);
    int nLen = strlen(szFileName);
    //触发文件发送消息
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
        AfxMessageBox(_T("数据库查询失败！"));
    }
    return strResult;
}
