// DragEdit.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Client.h"
#include "DragEdit.h"


// CDragEdit

IMPLEMENT_DYNAMIC(CDragEdit, CEdit)

BEGIN_MESSAGE_MAP(CDragEdit, CEdit)
    ON_WM_DROPFILES()
END_MESSAGE_MAP()



// CDragEdit ��Ϣ�������



void CDragEdit::OnDropFiles(HDROP hDropInfo)//���WM_DROPFILE��Ϣ��Ӧ����
{
    
    UINT count;
    TCHAR filePath[MAX_PATH] = { 0 };
    count = DragQueryFile(hDropInfo, -1, NULL, 0);
    if (1 == count)
    {      
        DragQueryFile(hDropInfo, 0, filePath, sizeof(filePath));
        this->SetWindowTextW(filePath);
        UpdateData(FALSE);
        DragFinish(hDropInfo); //�Ϸųɹ����ͷ��ڴ�
    }
    else
    {
        CString szFilePath;
        for (UINT i = 0; i < count; i++)
        {
            int pahtLen = DragQueryFile(hDropInfo, i, filePath, sizeof(filePath));
            szFilePath = szFilePath + filePath + _T("\r\n");
        }
        this->SetWindowTextW(szFilePath);
        UpdateData(FALSE);
        DragFinish(hDropInfo);
    }
}
