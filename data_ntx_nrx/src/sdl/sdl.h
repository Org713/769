#ifndef SDL_H
#define SDL_H


typedef struct _signal
{
	struct _signal* next;	//nextָ�룬�г��źŶ���������
	int (*func)(struct _signal*);//�źŵĺ���ָ��
	void *dst;					//�źŵ�Ŀ��ģ��ָ��
	unsigned short pri;			//���ȼ�
	unsigned short enabled;		//ʹ��
	void *param;				//�źŵĲ���ָ��
	void *src;					//�źŵ�Դģ��ָ��
	
}Signal;

typedef struct _timer
{
	struct _timer* next;
	int (*func)(struct _timer*);
	void *dst;
	unsigned short pri;
	unsigned short enabled;
	//��duration λ�õ�������������ֽڶ��룬�Ա���signal�ṹ�������Ӧ��ypchen
	unsigned long long duration;	//����ʱ��
	
}Timer;

#define SDL_PRI_MAX 4
#define SDL_PRI_NORM 2
#define SDL_PRI_LOW  3
#define SDL_PRI_HIGH 1
#define SDL_PRI_URG  0

typedef struct
{

	Timer* tmr_que_head;			//��ʱ������ͷ
	Signal* sig_que_head[SDL_PRI_MAX];	//�źŶ���ͷָ������
	Signal* sig_que_rear[SDL_PRI_MAX];	//�źŶ���βָ������

}Sdl_Core;

extern unsigned long long GetTime();

extern int AddSignal(Signal *sig);
extern int AddSignalIrq(Signal *sig);
extern void ExecSignal(Signal *sig);
extern int SetTimer(Timer *tmr,unsigned long long duration);
extern void CancelTimer(Timer *tmr);
extern void SdlCoreInit();
extern void SdlCoreRun();
extern int CheckSignal();
extern void CheckTimer();

#endif
