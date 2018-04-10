#ifndef RUNFILE_H
#define RUNFILE_H
#include <QThread>
#include <operateprogramxml.h>
#include <QMap>
#include "subFile.h"

#define RF_PROGRAM_DEBUG            0x03
#define RF_PROGRAM_DEBUG_ERROR      0x01
#define RF_PROGRAM_DEBUG_INFOR      0x02

class RunFile;
typedef int (RunFile::*ExeLine)(int lineNumber);
class RunFile : public QThread
{
//Q_OBJECT
public:
    RunFile(QString fileName);      //���캯����fileName��ʾ��Ҫ���е��ļ���
    //virtual ~RunFile();
    OperateProgramXml * op;         //����XML�ļ��Ľӿ�
    bool init();					//��ʼ��������������������ͻ����򣬲��Ҵ��ļ���ʼ������fileNameָ�����ļ�
    bool init(int lineNumber , int mode);  //  ��lineNumber�ƶ����к������ļ���
											//����ģʽ��mode: 0- forward ,1-inverse
    void run();							//QThread �ص�����������Ҫ�ⲿ����
    void stop();							//����ֹͣ��
    int exeLine(int lineNumber);				//�ڲ��������к�����lineNumber��ʾ�����кš�
    int exeInverseLine(int lineNumber);	//�ڲ��������к�����
    QString getFileName();					//��ȡ��ǰ�����ļ������������ӳ���ݹ����ʱȷ����ǰ�����ĸ��ӳ���ʹ�á�
    int getCurrentNumber();				//��ȡ��ǰ�����кţ�
    void setPause();							//��ͣ����ļ������С�
    void interpPause();						//������ͣ������������ͣ������ֻ��ʹ��������ͣ���˶�ֹͣ��Ҫ����motion��Ӧ��ڡ�

private:
    int inverse;								//�Ƿ����������еı�־�� 0: 
    QString fileName;							//���캯��������ļ�����						
    ProgramXmlLine line;						//�����һ�г�������
    int exeMovj(int lineNumber);				//ִ��movj����
    int exeMovl(int lineNumber);				//ִ��movl����
    int exeMovc(int lineNumber);				//ִ��movc����
    int exeSpeed(int lineNumber);			//ִ��speed����
    int exeDin(int lineNumber);				//ִ��DIN����
    int exeDout(int lineNumber);				//�ƶ�DOUT����
    int exePause(int lineNumber);			//ִ����ָͣ��
    int exeTimer(int lineNumber);			//ִ�ж�ʱ��ָ��
    int exeLabel(int lineNumber);			//��¼��ǩ
    int exeJump(int lineNumber);				//ִ����ת
    int exeCall(int lineNumber);				//ִ�е���
    int exeRet(int lineNumber);				//ִ�з���
    int exeNop(int lineNumber);				//ִ�г���ʼNOP
    int exeEnd(int lineNumber);				//ִ�г������END
    void scanLabel();						//�ļ���ʼʱɨ���ǩ����
    int exeCondition();					//ִ����������IF
    void waitForMotion();					//��ִ�к�������֮ǰ��Ҫ�ȴ��˶�ִ����ϡ�
    ExeLine exe[COMMAND_END_TYPE+1];			//ִ�г������ָ���б�
    int state;									//����ִ��״̬��
    int movcCount;								//���������movCָ��ĸ�����
    int jointType;								//�ؽ�����: movC����������Ĺؽ����ͱ���һ�¡�

    double default_vj;							//Ĭ�ϵ�movj���ٶ�ֵ����movjָ���в������ٶȲ�����ʹ����������е��ٶȡ�
    double default_vl;							//Ĭ�ϵ�movl��movc�����ٶ�ֵ����movl��movcָ���в��������ٶȲ�����ʹ����������е��ٶȡ�
    double default_vr;							//Ĭ�ϵ�movl��movc�Ľ��ٶ�ֵ����movl��movcָ���в�������ٶȲ�����ʹ����������е��ٶȡ�
    double default_acc;							//Ĭ�ϼ��ٶ�
    int currentFileLine;						//��ǰ�ļ��кš�
    int pause;									//��ָͣ��״̬��0: ִ����ָͣ��ʱ����ʾ��ͣ��1: ��ʾ����ѭ����������ͣ������
    int runFilePause;							//�ļ���ͣ����ʾ������ͣ������������ͣ���͡�0 : ��ʾδ��ͣ��1 : ��ʾ��ͣ���͡�
    QMap <QString,int> labelMap;				//���������ļ��б�ǩ����Ϣ��һ����ǩ����������Ϣ: qstring���͵��ļ�������һ��int�͵��к�
    QList <SubFile> subList;					//�ӳ����б�
    ProgramXmlLine nextLine;					//��һ�г������ݣ�������ִ��ʱʹ�á�
//Q_SIGNALS:
//    void FileNameChanged(QString fileName);
//    void FileClosed();

};

#endif // RUNFILE_H
