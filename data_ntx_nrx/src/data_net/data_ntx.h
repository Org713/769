#ifndef _DATA_NTX_H
#define _DATA_NTX_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"

#define DATA_LENGTH 100 //���ݳ���
//2017.1.2
#define NET_QUE_MAX_NUM 20//������г���
#define NET_PKT_MAX_TTL 100//���������ʱ��
//#define NET_PKT_RESEND_FROM_FM_TMR  100//������������ж�������ʱ��
//#define RT_FAIL_TMR 1000//·��ʧ��ʱ�䶨ʱ��������������е�����
#define LOCAL_NET_PKT 0
#define RLY_NET_PKT 1

typedef struct _data_interface_tx_req_param
{
	unsigned short *data;
	unsigned short len;
	unsigned short da;
	unsigned short pri;
	unsigned short svc_type;
	unsigned short pkt_sn;//2016.12.30 ����� cfmʹ��
}Data_Interface_Tx_Req_Param;

typedef struct _t_data_interface_tx_cfm_param
{
	unsigned short cfm_flag;//0 1 2
	unsigned short cfm_pkt_sn;//2016.12.30  �ظ�cfm�����
}T_Data_Interface_Tx_Cfm_Param;

typedef struct _t_data_ntx_to_data_nrx_test_param
{
	unsigned short *data;
	unsigned short len;
	unsigned short da;
	unsigned short pri;
	unsigned short svc_type;
	unsigned short sa;  //����
}T_Data_Ntx_To_Data_Nrx_Test_Param;

typedef struct _net_que_elmt
{
	unsigned short pkt_type;//���ذ����м̰� ��־
	unsigned short TTL;
	Net_Pkt net_pkt;
}Net_Que_Elmt;

typedef struct _net_que_list
{
	unsigned short top;
	unsigned short btm;
	unsigned short size;
	//unsigned short curt_snd_flag;//���ڷ��ͱ��
	Net_Que_Elmt que_elmt[NET_QUE_MAX_NUM];

}Net_Que_List;

typedef struct _t_data_ntx_rt_req_param
{
	unsigned short da;
	unsigned short fail_ra[3];
	unsigned short local_data_pri_id;// Ϊ��·��cfm�ҵ���data_ntx[?][da]   2017.1.4
}T_Data_Ntx_Rt_Req_Param;

typedef struct _data_ntx_rt_cfm_param
{
	unsigned short cfm_flag;
	unsigned short ra;
}Data_Ntx_Rt_Cfm_Param;

typedef struct _t_dlc_txpump_tx_req_param
{
	unsigned short *net_pkt;
	unsigned short len;
	unsigned short ra;
	unsigned short pri;
	unsigned short data_pri_id;//����������������Ϊ���²�cfmʱ���ҵ���Ӧ��data_ntx[][]   2017.1.4
	unsigned short module_id;
}T_Dlc_Txpump_Tx_Req_Param;

typedef struct _t_data_ntx_rt_find_req_param
{
	unsigned short da;
}T_Data_Ntx_Rt_Find_Req_Param;

typedef struct _data_rly_tx_req_param
{
	unsigned short *net_pkt;
	unsigned short da;
	unsigned short len;
	unsigned short pri;
}Data_Rly_Tx_Req_Param;

typedef struct _t_data_rly_tx_cfm_param
{
	unsigned short cfm_flag;
}T_Data_Rly_Tx_Cfm_Param;

typedef struct _dlc_txpump_tx_cfm_param
{
	unsigned short cfm_flag;
	unsigned short nsn;
}Dlc_Txpump_Tx_Cfm_Param;

typedef struct _data_ntx_rt_find_cfm_param
{
	unsigned short ra;
}Data_Ntx_Rt_Find_Cfm_Param;

typedef struct _data_ntx
{
	void *entity;
	Mib *mib;
	u8 state;
	unsigned short	data_pri_id;//�������ȼ�ID  0.1.2.3
	unsigned short	module_id;//��ģ���id�� 0-31  2016.12.28

	unsigned short	rcv_data[DATA_LENGTH];//����

	Net_Que_List	net_que_list;//�������ݰ����� 2017.1.2

	unsigned short	pkt_sn;//���͵���������
	unsigned short	wait_cfm_nsn;//�ȴ�cfm����������

	unsigned short	*net_pkt;//�������ַ  ����·��ʱ��ʱ����
	unsigned short	ra;	//��һ�����յ�ַ   ����·��ʱ��ʱ����
	unsigned short	pri;  //����·��ʱ��ʱ����
	unsigned short	len;   //����·��ʱ��ʱ����
	unsigned short	pkt_type;//���������or�м������
	unsigned short	da;//·��̽��ʹ��

	unsigned short	fail_ra[3];
	unsigned short	retry_cnt;

	Timer	tx_req_timer;//�ȴ�����cfm��ʱ��
	Timer	rt_find_timer;//·��̽���ȴ���ʱ��

	/***************sig received*****************/

	Signal	data_interface_tx_req;//2017.1.3
	Data_Interface_Tx_Req_Param data_interface_tx_req_param;

	Signal	data_rly_tx_req;//2017.1.2
	Data_Rly_Tx_Req_Param data_rly_tx_req_param;

	Signal	data_ntx_rt_cfm;//2017.1.3
	Data_Ntx_Rt_Cfm_Param data_ntx_rt_cfm_param;

	Signal	data_ntx_rt_find_cfm;
	Data_Ntx_Rt_Find_Cfm_Param data_ntx_rt_find_cfm_param;

	Signal	dlc_txpump_tx_cfm;
	Dlc_Txpump_Tx_Cfm_Param dlc_txpump_tx_cfm_param;

	Signal	data_tf_ind;

	/***************sig to send*****************/
	Signal*	data_interface_tx_cfm;

	Signal*	data_ntx_rt_req;
	Signal* dlc_txpump_tx_req;
	Signal* data_ntx_rt_find_req;
	Signal*	data_rly_tx_cfm;
	
	//�� �� �� ����
	Signal*	data_ntx_to_data_nrx_test;
}Data_Ntx;

extern void DataNtxSetup(Data_Ntx* proc);
extern void DataNtxInit(Data_Ntx* proc);


#endif