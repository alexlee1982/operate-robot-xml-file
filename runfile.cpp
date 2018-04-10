#include "runfile.h"
#include "usr_motion_api.h"
#include "opXmlErrorType.h"
RunFile::RunFile(QString fileName)
{
	//��ʼ��״̬
    this->fileName = fileName;
    exe[COMMAND_MOVJ_TYPE]=&RunFile::exeMovj;
    exe[COMMAND_MOVL_TYPE]=&RunFile::exeMovl;
    exe[COMMAND_MOVC_TYPE]=&RunFile::exeMovc;
    exe[CMOMAND_SPEED_TYPE]=&RunFile::exeSpeed;
    exe[COMMAND_DOUT_TYPE]=&RunFile::exeDout;
    exe[COMMAND_DIN_TYPE]=&RunFile::exeDin;
    exe[COMMAND_JUMP_TYPE]=&RunFile::exeJump;
    exe[COMMAND_LABEL_TYPE]=&RunFile::exeLabel;
    exe[COMMAND_CALL_TYPE]=&RunFile::exeCall;
    exe[COMMAND_PAUSE_TYPE]=&RunFile::exePause;
    exe[COMMAND_TIMER_TYPE]=&RunFile::exeTimer;
    exe[COMMAND_RET_TYPE]=&RunFile::exeRet;
    exe[COMMAND_NOP_TYPE]=&RunFile::exeNop;
    exe[COMMAND_END_TYPE]=&RunFile::exeEnd;
    movcCount = 0;
    pause = 0;
    currentFileLine = 0;
    op = new OperateProgramXml();
    state = 0;
    inverse = 0;
    runFilePause = 0;
    line.commandType = COMMAND_NONE_TYPE;
}
bool RunFile::init()
{
    int ret = 0;
    // open file ,
    ret = op->init(fileName);
    //currentFileLine = 0;
    if(ret)
    {
        if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_ERROR)
        {
            qDebug()<<"failed to open file  " << fileName << " errocode is " << ret;
        }
        return ret;
    }
    start();
    return true;
}
bool RunFile::init(int lineNumber, int mode)
{
    currentFileLine = lineNumber;
    inverse = mode;
    init();
}
void RunFile::run()
{
    int ret = 0;
	//ִ�������ļ�֮ǰɨ��һ�������ļ���label��
	scanLabel();
	//��ʼ�������ȡ�ļ���Ϣ����������Ϊδ��ȡ
    line.commandType = COMMAND_NONE_TYPE;
    
	//��������ʱ��һֱִ�е�����END��
	//��������ʱ��һֱִ�е�����NOP��
    while(((line.commandType!=COMMAND_END_TYPE)&&(!state)&&(!inverse))
          ||
          ((line.commandType!=COMMAND_NOP_TYPE)&&(!state)&&(inverse)))
    {
  	  //�ж������Ƿ���ͣ
      if(!runFilePause)
      {
        //�ж��Ƿ�Ϊ��������
        if(!inverse)
        {
          	//����
            ret = exeLine(currentFileLine);
            if(ret)
            {
            	if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_ERROR)
        		{
            		qDebug()<<"failed to forward exe line =   " << currentFileLine<< ", errocode is " << ret;
        		}
                break;
            }
            currentFileLine++;
        }
        else
        {
        	//����
            ret = exeInverseLine(currentFileLine);
            if(ret)
            {
              	if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_ERROR)
        		{
            		qDebug()<<"failed to inverse exe line =   " << currentFileLine<< ", errocode is " << ret;
        		}
                break;
            }
            currentFileLine--;
        }

      }
        msleep(100);
    }
	if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_INFOR)
    {
    	qDebug() << "exiting this thread";
    }    
}
void RunFile::stop()
{	
	if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_INFOR)
    {
    	qDebug() << "Runfile : stopping the thread";
    } 
    state = 1;
}
int RunFile::exeLine(int lineNumber)
{
    int ret =0;
	//���ļ��ж�ȡĳһ�У����뵽line��
    ret = op->readLine(lineNumber);
    line = op->getLine();
	if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_INFOR)
    {
	    qDebug("RunFile: exeLine commandType = %d",line.commandType);
    } 
    
    if(ret)
    {
        if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_ERROR)
        {
            qDebug()<<"read line error  lineNumber = " << lineNumber << " errocode is " << ret;
        }
        return ret;
    }
	//����������Ͳ����˶�ָ�����Ҫ�ȴ��˶�ִ����ɡ�
    if(line.commandType > COMMAND_MOVC_TYPE)
    {
        waitForMotion();
    }
	//�����ȡ���������Ͳ���MOVC���Ǿͽ����¼�¼MOVC�ļ�����
    if(line.commandType!=COMMAND_MOVC_TYPE)
    {
      movcCount = 0;
    }
    ExeLine run = exe[line.commandType];
    ret = (this->*run)(lineNumber);
    return ret;
}


int RunFile::exeMovj(int lineNumber)
{

    if(movcCount >0&&movcCount != 3)
    {
        if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_ERROR)
        {
            qDebug()<<"can not exe movj before end of movc  count = " << movcCount ;
        }
        return CAN_NOT_DO_MOVJ_BEFORE_END_OF_MOVC;
    }
    //movcCount = 0;
    JointMoveInformation mj;
    if(line.p.coord == JOINT)
    {
        mj.endPoint.j1 = line.p.p[0];
        mj.endPoint.j2 = line.p.p[1];
        mj.endPoint.j3 = line.p.p[2];
        mj.endPoint.j4 = line.p.p[3];
        mj.endPoint.j5 = line.p.p[4];
        mj.endPoint.j6 = line.p.p[5];
    }
    else
    {
        if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_ERROR)
        {
            qDebug()<<"can not exe movj with a non joint pos = " << line.p.coord;
        }
        return ADD_MOVJ_WITH_NON_JOINT_POS;
    }
    if(line.vel.vj < 0)
    {
        mj.vJ = default_vj;
    }
    else
    {
        mj.vJ = line.vel.vj;
    }

    if(line.acc < 0)
    {
        mj.acc = default_acc;
        mj.dec = default_acc;
    }
    else
    {
        mj.acc = line.acc;
        mj.dec = line.acc;
    }
    if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_INFOR)
    {
        qDebug()<<"exe movj p = " << mj.endPoint.j1 << mj.endPoint.j2 << mj.endPoint.j3 << mj.endPoint.j4 << mj.endPoint.j5 << mj.endPoint.j6
               <<"vj = " << mj.vJ << "acc = " << mj.acc << "dec = " << mj.dec ;
    }
    CTRL_AddJointMove(&mj,lineNumber,0);
    return 0;
}
int RunFile::exeMovl(int lineNumber)
{
    if(movcCount >0&&movcCount != 3)
    {
        if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_ERROR)
        {
            qDebug()<<"can not exe movl before end of movc  count = " << movcCount ;
        }
        return CAN_NOT_DO_MOVL_BEFORE_END_OF_MOVC;
    }
    //movcCount = 0;
	//�ж�����λ�õ�����ϵ���ͣ�ѡ���Ӧ��motion�ӿڡ�
    if(line.p.coord == JOINT)
    {
        LinearMoveInformation ml;
        ml.acc = line.acc;
        ml.dec = line.acc;
        ml.vL = line.vel.vl;
        ml.vR = line.vel.vr;

        if(line.vel.vl < 0)
        {
            ml.vL = default_vl;
        }
        if(line.vel.vr < 0)
        {
            ml.vR = default_vr;
        }
        if(line.acc < 0)
        {
            ml.acc = default_acc;
            ml.dec = default_acc;
        }

        ml.endPoint.j1 = line.p.p[0];
        ml.endPoint.j2 = line.p.p[1];
        ml.endPoint.j3 = line.p.p[2];
        ml.endPoint.j4 = line.p.p[3];
        ml.endPoint.j5 = line.p.p[4];
        ml.endPoint.j6 = line.p.p[5];
        if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_INFOR)
        {
            qDebug()<<"exe movl p = " << ml.endPoint.j1 << ml.endPoint.j2 << ml.endPoint.j3 << ml.endPoint.j4 << ml.endPoint.j5 << ml.endPoint.j6
                   <<"vl = " << ml.vL << "acc = " << ml.acc << "dec = " << ml.dec <<"vr = " << ml.vR ;
        }
        CTRL_AddLinearMove(&ml,lineNumber,0);
    }
    else if(line.p.coord == WORLD)
    {
        LinearDescartesMoveInformation dml;
        dml.acc = line.acc;
        dml.dec = line.acc;
        dml.vL = line.vel.vl;
        dml.vR = line.vel.vr;

        if(line.vel.vl < 0)
        {
            dml.vL = default_vl;
        }
        if(line.vel.vr < 0)
        {
            dml.vR = default_vr;
        }
        if(line.acc < 0)
        {
            dml.acc = default_acc;
            dml.dec = default_acc;
        }


        dml.endPoint.x = line.p.p[0];
        dml.endPoint.y = line.p.p[1];
        dml.endPoint.z = line.p.p[2];
        dml.zyx.a = line.p.p[3];
        dml.zyx.b = line.p.p[4];
        dml.zyx.c = line.p.p[5];

        dml.user.x = 0.0;
        dml.user.y = 0.0;
        dml.user.z = 0.0;
        dml.user.a = 0.0;
        dml.user.b = 0.0;
        dml.user.c = 0.0;

        dml.tool.x = 0.0;
        dml.tool.y = 0.0;
        dml.tool.z = 0.0;
        dml.tool.rx = 0.0;
        dml.tool.ry = 0.0;
        dml.tool.rz = 0.0;
        if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_INFOR)
        {
            qDebug()<<"exe movdl p = " << dml.endPoint.x << dml.endPoint.y << dml.endPoint.z << dml.zyx.a << dml.zyx.b << dml.zyx.c
                   <<"vl = " << dml.vL << "acc = " << dml.acc << "dec = " << dml.dec <<"vr = " << dml.vR ;
        }
        CTRL_AddDescartesLinearMove(&dml,lineNumber,0);
    }
    else if(line.p.coord == USR || line.p.coord == TOOL)
    {
        LinearDescartesMoveInformation dml;
        dml.acc = line.acc;
        dml.dec = line.acc;
        dml.vL = line.vel.vl;
        dml.vR = line.vel.vr;

        if(line.vel.vl < 0)
        {
            dml.vL = default_vl;
        }
        if(line.vel.vr < 0)
        {
            dml.vR = default_vr;
        }
        if(line.acc < 0)
        {
            dml.acc = default_acc;
            dml.dec = default_acc;
        }

        dml.endPoint.x = line.p.p[0];
        dml.endPoint.y = line.p.p[1];
        dml.endPoint.z = line.p.p[2];
        dml.zyx.a = line.p.p[3];
        dml.zyx.b = line.p.p[4];
        dml.zyx.c = line.p.p[5];

        dml.user.x = line.usr.x;
        dml.user.y = line.usr.y;
        dml.user.z = line.usr.z;
        dml.user.a = line.usr.a;
        dml.user.b = line.usr.b;
        dml.user.c = line.usr.c;

        dml.tool.x = line.tool.x;
        dml.tool.y = line.tool.y;
        dml.tool.z = line.tool.z;
        dml.tool.rx = line.tool.a;
        dml.tool.ry = line.tool.b;
        dml.tool.rz = line.tool.c;
        if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_INFOR)
        {
            qDebug()<<"exe movdl p = " << dml.endPoint.x << dml.endPoint.y << dml.endPoint.z << dml.zyx.a << dml.zyx.b << dml.zyx.c
                   <<"vl = " << dml.vL << "acc = " << dml.acc << "dec = " << dml.dec <<"vr = " << dml.vR ;
        }
        CTRL_AddDescartesLinearMove(&dml,lineNumber,0);
    }
    else
    {
        return UNKNONW_COORDINATE_TYPE;
    }

    return 0;
}
int RunFile::exeMovc(int lineNumber)
{
    CircularMoveInformation mc;
    CircularDescartesMoveInformation dmc;
    static JointPoint position[3];
	//����3��movc�ſ����·�һ���˶�ָ�
	//����3��movc������λ�ò������ͱ�����һ�µġ�
    if(movcCount == 0 )
    {
        jointType = line.p.coord;
    }
    else if(jointType!=line.p.coord)
    {
        return MOVC_POSTION_TYPE_CONFLIC;
    }
	//����������5��movcʱ�����Խ���3��movc��ָ����ڶ���Բ���еĵ�һ��
    else if(movcCount == 3)
    {
        position[0] = position[2];
        movcCount = 1;
    }
    movcCount ++;
    position[movcCount-1].j1 = line.p.p[0];
    position[movcCount-1].j2 = line.p.p[1];
    position[movcCount-1].j3 = line.p.p[2];
    position[movcCount-1].j4 = line.p.p[3];
    position[movcCount-1].j5 = line.p.p[4];
    position[movcCount-1].j6 = line.p.p[5];
	//��¼movc�е�λ����Ϣ�������һ��movc�вŻ�����ִ���˶�ָ�

    if(movcCount == 3)
    {
        if(line.p.coord == JOINT)
        {
            mc.acc = line.acc;
            mc.dec = line.acc;
            mc.vL = line.vel.vl;
            mc.vR = line.vel.vr;

            if(line.vel.vl < 0)
            {
                mc.vL = default_vl;
            }
            if(line.vel.vr < 0)
            {
                mc.vR = default_vr;
            }
            if(line.acc < 0)
            {
                mc.acc = default_acc;
                mc.dec = default_acc;
            }

            for(int i=0; i < 3;i++)
            {
                mc.endPoint[i] = position[i];
            }
            if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_INFOR)
            {
                qDebug()<<"exe movc p1 = " <<mc.endPoint[0].j1 << mc.endPoint[0].j2 << mc.endPoint[0].j3 << mc.endPoint[0].j4 << mc.endPoint[0].j5 << mc.endPoint[0].j6
                        <<"p2 = " <<mc.endPoint[1].j1 << mc.endPoint[1].j2 << mc.endPoint[1].j3 << mc.endPoint[1].j4 << mc.endPoint[1].j5 << mc.endPoint[1].j6
                       <<"p3 = " <<mc.endPoint[2].j1 << mc.endPoint[2].j2 << mc.endPoint[2].j3 << mc.endPoint[2].j4 << mc.endPoint[2].j5 << mc.endPoint[2].j6
                       <<"vl = " << mc.vL << "acc = " << mc.acc << "dec = " << mc.dec <<"vr = " << mc.vR ;
            }
            CTRL_AddCircularMove(&mc,lineNumber,0);
        }
        else if(line.p.coord == WORLD)
        {
            dmc.acc = line.acc;
            dmc.dec = line.dec;
            dmc.vL = line.vel.vl;
            dmc.vR = line.vel.vr;
            if(line.vel.vl < 0)
            {
                dmc.vL = default_vl;
            }
            if(line.vel.vr < 0)
            {
                dmc.vR = default_vr;
            }
            if(line.acc < 0)
            {
                dmc.acc = default_acc;
                dmc.dec = default_acc;
            }


            dmc.free = 0;
            for(int i =0;i<3;i++)
            {
                dmc.endPoint[i].x = position[i].j1;
                dmc.endPoint[i].y = position[i].j2;
                dmc.endPoint[i].z = position[i].j3;
                dmc.zyx[i].a = position[i].j4;
                dmc.zyx[i].b = position[i].j5;
                dmc.zyx[i].c = position[i].j6;
            }
            dmc.user.x = 0.0;
            dmc.user.y = 0.0;
            dmc.user.z = 0.0;
            dmc.user.a = 0.0;
            dmc.user.b = 0.0;
            dmc.user.c = 0.0;

            dmc.tool.x = 0.0;
            dmc.tool.y = 0.0;
            dmc.tool.z = 0.0;
            dmc.tool.rx = 0.0;
            dmc.tool.ry = 0.0;
            dmc.tool.rz = 0.0;

            CTRL_AddDescartesCircularMove(&dmc,lineNumber,0);
        }
        else if(line.p.coord == USR || line.p.coord == TOOL)
        {

            dmc.acc = line.acc;
            dmc.free = 0;

            dmc.acc = line.acc;
            dmc.dec = line.dec;
            dmc.vL = line.vel.vl;
            dmc.vR = line.vel.vr;
            if(line.vel.vl < 0)
            {
                dmc.vL = default_vl;
            }
            if(line.vel.vr < 0)
            {
                dmc.vR = default_vr;
            }
            if(line.acc < 0)
            {
                dmc.acc = default_acc;
                dmc.dec = default_acc;
            }


            for(int i =0;i<3;i++)
            {
                dmc.endPoint[i].x = position[i].j1;
                dmc.endPoint[i].y = position[i].j2;
                dmc.endPoint[i].z = position[i].j3;
                dmc.zyx[i].a = position[i].j4;
                dmc.zyx[i].b = position[i].j5;
                dmc.zyx[i].c = position[i].j6;
            }
            dmc.user.x = line.usr.x;
            dmc.user.y = line.usr.y;
            dmc.user.z = line.usr.z;
            dmc.user.a = line.usr.a;
            dmc.user.b = line.usr.b;
            dmc.user.c = line.usr.c;

            dmc.tool.x = line.tool.x;
            dmc.tool.y = line.tool.y;
            dmc.tool.z = line.tool.z;
            dmc.tool.rx = line.tool.a;
            dmc.tool.ry = line.tool.b;
            dmc.tool.rz = line.tool.c;

            CTRL_AddDescartesCircularMove(&dmc,lineNumber,0);
        }
        else
        {
            return UNKNONW_COORDINATE_TYPE;
        }

    }
    return 0;

}
int RunFile::exeSpeed(int lineNumber)
{
    default_vj = line.vel.vj;
    default_vl = line.vel.vl;
    default_vr = line.vel.vr;
    default_acc = line.acc;
    return 0;
}

int RunFile::exeDin(int lineNumber)
{
    //call interface

    //int value = din(line.IoPort);
    //setByte(line.byteAddress,value);
    return 0;
}
int RunFile::exeDout(int lineNumber)
{
    //call interface

    //dout(line.IoPort,line.dOutValue);
    return 0;
}
int RunFile::exePause(int lineNumber)
{
    int condition = 0;

    condition = exeCondition();
	//���뵽����������У������̲߳���ִ��������������һֱ���������һֱ��pause����Ϊ1��
    while(condition&&(!state))
    {
        msleep(100);
        if(pause)
        {
            condition = 0;
            pause = 0;
        }
    }
    return 0;
}
int RunFile::exeTimer(int lineNumber)
{
   // movcCount = 0;
    double mtime = line.time*1000;
    QTime start = QTime::currentTime();
    QTime end = QTime::currentTime();
	//���뵽ִ�м�ʱ�������У���ʼ��ʱ��һֱ��ʱ��ﵽԤ���Ż��Ƴ�ȥ������state����ִ��״̬��ʾ�����̱߳�ʾֹͣҲ���Ƴ���
    while((start.msecsTo(end) < mtime)&&(!state))
    {
        //qDebug() << "timer start = " <<start <<"end = " <<end <<"mtime = " << mtime;
        //qDebug() << "time lasting is " <<start.msecsTo(end);
        msleep(100);
        end = QTime::currentTime();
    }
    return 0;
}
int RunFile::exeLabel(int lineNumber)
{
    return 0;
}
int RunFile::exeJump(int lineNumber)
{
    int condition = 0;
    condition = exeCondition();
	//�ӱ�ǩ�������ҵ�ƥ��ı�ǩ������ǰ��ָ���ǩ�ĵ�ǰ�С�
	//����δ������ִ���ӳ�����öԱ�ǩ������Ӱ�졣Ӧ�û���һ��bug��
    if(condition)
    {
        int fileLine;
        fileLine = labelMap[QString(line.lablel)];
        if(fileLine>0)
        {
            currentFileLine = fileLine;
        }
        //currentFileLine = line.lablel;
    }
    return 0;
}
int RunFile::exeCall(int lineNumber)
{
    int condition = 0;
    int ret =0 ;
    condition = exeCondition();
    if(condition)
    {
        SubFile sub;
		//����ǰ�ĳ����ļ�����Ϣ��¼����ѹ�뵽������
        sub.fileLine = currentFileLine;
        sub.fileName = this->fileName;
        subList.prepend(sub);
		qDebug() << "call sub file = " << line.fileName;
		//���ӳ���
		ret = op->init(line.fileName);
        this->fileName = line.fileName;
        currentFileLine = 0;
		//ɨ���ӳ����ǩ��
		scanLabel();
        if(ret)
        {
            return false;
        }
    }
  // Q_EMIT FileNameChanged(this->fileName);
    return 0;
}
int RunFile::exeRet(int lineNumber)
{
    int condition = 0;
    int ret =0 ;
    condition = exeCondition();
	//ִ�з���
    if(condition)
    {
    	//���ļ������ж�ȡ���һ�����ظ�����Ϣ�����ҽ�������Ϣ���������Ƴ�
        SubFile sub;
        sub = subList.first();
        subList.removeFirst();
        this->fileName = sub.fileName;
        this->currentFileLine = sub.fileLine;
        qDebug() << "return to file " << this->fileName <<"at line " << this->currentFileLine;
        ret = op->init(fileName);
		//����ɨ���ǩ��
		scanLabel();
        if(ret)
        {
            qDebug() << "open file error " <<ret;
            return false;
        }
    }
    //Q_EMIT FileNameChanged(this->fileName);
    return 0;
}
int RunFile::exeNop(int lineNumber)
{
    qDebug("RunFile: exeNope");
    return 0;
}
int RunFile::exeEnd(int lineNumber)
{
   // Q_EMIT FileClosed();
    return 0;
}

//ѭ�����������ӿ�
void RunFile::setPause()
{
    pause = 1;
    runFilePause = 0;
}
void RunFile::scanLabel()
{
    int i =0;
    int ret;
	//ɨ�������ļ��е�label��ָ�������У�
	//����δ���ǵ��ӳ����ļ������������У�ɨ�赽END�����ӳ�����ɨ�赽RET��
    for(;line.commandType!= COMMAND_END_TYPE&&line.commandType!= COMMAND_RET_TYPE;)
    {
        ret = op->readLine(i);
        if(ret)
        {
            if(RF_PROGRAM_DEBUG&RF_PROGRAM_DEBUG_ERROR)
            {
                qDebug()<<"read line error , errorCode is  " << ret;
            }
            break;
        }
        line = op->getLine();
        if(line.commandType == COMMAND_LABEL_TYPE)
        {
            labelMap.insert(QString(line.lablel),i);
        }
        i++;
    }
    op->readLine(0);
    line = op->getLine();
}
int RunFile::exeCondition()
{
    int condition = 0;
    if(line.condition.type == VAR_TYPE_NONE)
    {
        condition = 1;
    }
    else
    {
        int valueLeft;
        int valueRight;
        switch(line.condition.type)
        {
            case DIN:
                //call interface
                valueRight = line.condition.value.dio;
                //valueLeft = din(line.condition.varAddress)
                break;
            case DOUT:
               // valueLeft = readDout(line.condition.varAddress)
                valueRight = line.condition.value.dio;
                break;
            case BYTE:
              //valueLeft = getByte(line.condition.varAddress)
                valueRight = line.condition.value.integerValue;
                break;
            case INT:
              //valueLeft = getInt(line.condition.varAddress)
                valueRight = line.condition.value.integerValue;
                break;
        default:
            condition = 0;
            break;
        }
        switch(line.condition.sign)
        {
        case EQ:
            if(valueLeft == valueRight)
                condition = 1;
            else
                condition = 0;
            break;
        case GT:
            if(valueLeft > valueRight)
                condition = 1;
            else
                condition = 0;
            break;
        case LT:
            if(valueLeft < valueRight)
                condition = 1;
            else
                condition = 0;
            break;
        case GE:
            if(valueLeft >= valueRight)
                condition = 1;
            else
                condition = 0;
            break;
        case LE:
            if(valueLeft <= valueRight)
                condition = 1;
            else
                condition = 0;
            break;
        default:
            condition = 0;
            break;
        }
    }
    return condition;

}
int RunFile::exeInverseLine(int lineNumber)
{

    int ret =0;
    ret = op->readLine(lineNumber);
    qDebug("read lineNumber = %d",lineNumber);
    line = op->getLine();
    if(ret)
    {
        return ret;
    }
	//����ִ�С������������е�˼·�ǲ���ִ����ת���ӳ�����á������������ִ�С�
	//���������еĹ����У�movc����������ȡ����ָ��ִ�С�movj��movl��Ҫִ����һ���˶�λ�õ��յ�λ�á�
	//

	
    if(line.commandType > COMMAND_MOVC_TYPE)
    {
        waitForMotion();
    }
    if(line.commandType == COMMAND_LABEL_TYPE||
       line.commandType == COMMAND_JUMP_TYPE)
    {
      return 0;
    }
    if(
       line.commandType == COMMAND_MOVL_TYPE||
       line.commandType == COMMAND_MOVJ_TYPE
       )
    {
        if(nextLine.commandType == COMMAND_MOVC_TYPE)
        {
          ExeLine run = exe[nextLine.commandType];
          ret = (this->*run)(lineNumber);
        }
        ProgramXmlLine lastLine;
        lastLine.commandType = COMMAND_NONE_TYPE;
        while(lastLine.commandType != COMMAND_MOVJ_TYPE&&
              lastLine.commandType != COMMAND_MOVL_TYPE&&
              lastLine.commandType != COMMAND_MOVC_TYPE)
        {
            int i=lineNumber - 1;
            ret = op->readLine(i);
            if(ret)
            {
                return ret;
            }
            lastLine = op->getLine();
            if(lastLine.commandType == COMMAND_MOVJ_TYPE||
               lastLine.commandType == COMMAND_MOVL_TYPE||
               lastLine.commandType == COMMAND_MOVC_TYPE)
            {
                line.p = lastLine.p ;
            }
            i--;
        }
    }
    if(line.commandType == COMMAND_MOVJ_TYPE||
       line.commandType == COMMAND_MOVL_TYPE||
       line.commandType == COMMAND_MOVC_TYPE)
    {
      nextLine = line;
    }
    if(line.commandType!=COMMAND_MOVC_TYPE)
    {
      movcCount = 0;
    }
    ExeLine run = exe[line.commandType];
    ret = (this->*run)(lineNumber);
    return ret;

}
void RunFile::waitForMotion()
{
    MotionFeedback fb;
    CTRL_GetMotionStatus(&fb);
    while(fb.motionState!=COMMAND_DONE&&(!state))
    {
        //qDebug() << "fb.motionState = " << fb.motionState << "state = " << state;
        CTRL_GetMotionStatus(&fb);
        msleep(100);
    }
    return;
}
int RunFile::getCurrentNumber()
{
  return currentFileLine;
}
void RunFile::interpPause()
{
  runFilePause = 1;
}
