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
    RunFile(QString fileName);      //
    //virtual ~RunFile();
    OperateProgramXml * op;
    bool init();
    bool init(int lineNumber , int mode);   //mode: 0- forward ,1-inverse
    void run();
    void stop();
    int exeLine(int lineNumber);
    int exeInverseLine(int lineNumber);
    QString getFileName();
    int getCurrentNumber();
    void setPause();
    void interpPause();

private:
    int inverse;
    QString fileName;
    ProgramXmlLine line;
    int exeMovj(int lineNumber);
    int exeMovl(int lineNumber);
    int exeMovc(int lineNumber);
    int exeSpeed(int lineNumber);
    int exeDin(int lineNumber);
    int exeDout(int lineNumber);
    int exePause(int lineNumber);
    int exeTimer(int lineNumber);
    int exeLabel(int lineNumber);
    int exeJump(int lineNumber);
    int exeCall(int lineNumber);
    int exeRet(int lineNumber);
    int exeNop(int lineNumber);
    int exeEnd(int lineNumber);
    void scanLabel();
    int exeCondition();
    void waitForMotion();
    ExeLine exe[COMMAND_END_TYPE+1];
    int state;
    int movcCount;
    int jointType;

    double default_vj;
    double default_vl;
    double default_vr;
    double default_acc;
    int currentFileLine;
    int pause;
    int runFilePause;
    QMap <QString,int> labelMap;
    QList <SubFile> subList;
//Q_SIGNALS:
//    void FileNameChanged(QString fileName);
//    void FileClosed();

};

#endif // RUNFILE_H
