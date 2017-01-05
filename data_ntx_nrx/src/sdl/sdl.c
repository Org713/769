#include "sdl.h"
//#include "xil_exception.h"
#define DEBUG
#ifdef DEBUG
    unsigned int sdlc_sig_again = 0;
#endif
//#define NULL 0
//static Sdl_Core sdlc;
extern Sdl_Core sdlc;
//static void DisIrq(void)
//{
////	__asm ("swi 0");
//}
//
//static void EnIrq(void)
//{
////	__asm ("swi 1");
//}
static void TmrInit()
{
    sdlc.tmr_que_head = 0;	
    return;
}
extern unsigned long long SysGetTime(void);



unsigned long long GetTime()//ms
{//
    //return 0;// SysSimGetTime()/10;//
    return SysGetTime();
    //return 0;
}

static void SigQueInit()
{
    int i;
    for(i=0;i<SDL_PRI_MAX;i++)
    {
        sdlc.sig_que_head[i]=0;
        sdlc.sig_que_rear[i]=0;
    }	
    return;
}

int AddSignalIrq(Signal *sig)
{
    unsigned short pri = sig->pri;

    if((!sig->func) ||(pri>=SDL_PRI_MAX))
        return 0;
    #ifdef DEBUG
        if(sig->enabled)
        {
            sdlc_sig_again++;  //ͬһ�ź��ظ����
        }
    #endif
    sig->next = 0;
    sig->enabled = 1;

    if(sdlc.sig_que_rear[pri])
    {
        sdlc.sig_que_rear[pri]->next = sig;
    }
    else
    {
        sdlc.sig_que_head[pri]=sig;
    }
    sdlc.sig_que_rear[pri] =  sig;
    return 1;
}

int AddSignal(Signal *sig)
{	

    int ret;
//	Xil_ExceptionDisable();

    //DisIrq();
    ret = AddSignalIrq(sig);
//	Xil_ExceptionEnable();
    //EnIrq();
    return ret;
}

void ExecSignal(Signal *sig)
{
    sig->next = 0;
    if(sig->enabled)
    {
        sig->enabled = 0;
//		TestSignalPrint(sig);
        if(sig->func(sig))//����0˵���ź���ȷִ��
        {
            AddSignal(sig);	
        }
    }
}

int CheckSignalPri(unsigned short pri)
{
    Signal *sig = 0;
    
    //DisIrq();
    if(sdlc.sig_que_head[pri])
    {
        sig = sdlc.sig_que_head[pri];
        if(sdlc.sig_que_head[pri] == sdlc.sig_que_rear[pri])
        {
            sdlc.sig_que_head[pri] =0;
            sdlc.sig_que_rear[pri] =0;
        }
        else
        {
            sdlc.sig_que_head[pri] = sig->next;
        }
    }
    //EnIrq();
    if(sig)
    {
        ExecSignal(sig);
    }
    return (int)sig;
}
int CheckSignal()
{
    int i;
//	Xil_ExceptionDisable();
    for(i=0;i<SDL_PRI_MAX;i++)
    {
        if(CheckSignalPri(i))
        {
            break;
        }
    }
//	Xil_ExceptionEnable();
    return SDL_PRI_MAX-i;
}

/*
 * ����duration��С����������ң�c_tmr��ʼָ�����ͷ��
 * ��tmr->duration����c_tmr->duration������ʱ����p_tmrָ��c_tmr��p_tmr = c_tmr��������c_tmr=c_tmr->next���бȽϣ�
 * ��С��c_tmr->duration�����tmr���뵽c_tmr��ǰ�棬p_tmr->next = tmr��
 * ����tmr��nextָ��c_tmr��tmr->next = c_tmr��
*/
int SetTimer(Timer *tmr,unsigned long long duration)
{
    Timer * p_tmr,*c_tmr;

    tmr->duration = GetTime(sdlc)+duration;// ��ȡlocaltime��
    tmr->enabled = 1;////////

    //DisIrq();
//	DbgPrint("SetTimer():node ���ö�ʱ\n");	
    p_tmr = 0;

    c_tmr = sdlc.tmr_que_head;  
    while(c_tmr)       //  ����duration��С����������ҡ�                        
    {                                       
        if(c_tmr->duration <= tmr->duration)
        {
            p_tmr = c_tmr;
            c_tmr = c_tmr->next;
        }
        else
        {
            break;
        }
    }
    if(p_tmr)    //�鵽���ʵ�λ�ã���tmr�ŵ�������ȥ��
    {
        p_tmr->next = tmr;		
    }
    else
    {
        sdlc.tmr_que_head = tmr;		
    }
    tmr->next = c_tmr;
    //EnIrq();
    return 1;
}

void CheckTimer()
{
    Timer * c_tmr;
    unsigned long long c_time = GetTime(sdlc);

    while(sdlc.tmr_que_head)
    {
        if(sdlc.tmr_que_head->duration <= c_time)
        {
            c_tmr = sdlc.tmr_que_head;
            sdlc.tmr_que_head = c_tmr->next;
            if(c_tmr->enabled)
            {
    //			TestTimerPrint(c_tmr);
                ExecSignal((Signal*)c_tmr);
                //AddSignalIrq((Signal*)c_tmr);
            }
        }
        else
        {
            break;
        }
        
    }
}

void CancelTimer(Timer* tmr)
{
    Timer * p_tmr,*c_tmr;
    
    p_tmr = 0;
    c_tmr = sdlc.tmr_que_head;
    while(c_tmr)
    {
        if(c_tmr != tmr) //һ��һ��������
        {
            p_tmr = c_tmr;//p_tmr��ʱ����ͷ
            c_tmr = c_tmr->next;
        }
        else
        {
            if(p_tmr)  //�ڷ�ͷ λ���ҵ�tmr
            {
                p_tmr->next = c_tmr->next;
            }
            else   //����Ҫ�ҵ�tmr����ͷλ�õ�
            {
                sdlc.tmr_que_head = c_tmr->next;
            }

            break;
        }
    }
}

void SdlCoreInit()
{		
    SigQueInit();
    TmrInit();
}

void SdlCoreRun()
{
    while(1)
    {		
        CheckSignal();
        CheckTimer(); 
    }
}
 
