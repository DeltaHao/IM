#pragma once


// CDragEdit ����ʵ���ı�����ק�ļ���ʾ·��

class CDragEdit : public CEdit
{
	DECLARE_DYNAMIC(CDragEdit)

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDropFiles(HDROP hDropInfo);
};


