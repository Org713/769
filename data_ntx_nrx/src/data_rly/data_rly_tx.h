#ifndef DATA_RLY_TX_H
#define DATA_RLY_TX_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"

#define RLY_QUE_MAX_NUM 20
#define RLY_QUE_MAX_TTL 100	//�м̶��������������ʱ��

#define MAX_STORE_SPACE 18	//���洢�ռ�
#define MIN_STORE_SPACE 2	//��ʹ洢�ռ�


typedef struct _t_data_rly_tx_req_param
{
	unsigned short *net_pkt;
	unsigned short da;
	unsigned short len;
	unsigned short pri;
}T_Data_Rly_Tx_Req_Param;

typedef struct _data_rly_tx_cfm_param
{
	unsigned short cfm_flag;
}Data_Rly_Tx_Cfm_Param;

typedef struct _t_data_rly_tx_jam_ind_param
{
	unsigned short jam_flag;//jam_flag=0/1, 1:ӵ��   0��ӵ�����
}T_Data_Rly_Tx_Jam_Ind_Param;

typedef struct _data_rly_rx_ind_param
{
	unsigned short *net_pkt;
	unsigned short len;
	unsigned short pri;
}Data_Rly_Rx_Ind_Param;

typedef struct _rly_que_elmt
{
	unsigned short TTL;
	unsigned short pri;
	Net_Pkt net_pkt;
}Rly_Que_Elmt;

typedef struct _rly_que_list
{
	unsigned short top;
	unsigned short btm;
	unsigned short size;
	Rly_Que_Elmt que_elmt[RLY_QUE_MAX_NUM];
}Rly_Que_List;

typedef struct _data_rly_tx
{
	void *entity;
	Mib *mib;
	u8 state;

	Rly_Que_List rly_que_list;//�м̶���

	//unsigned short  jam_free_ind_flag;//ӵ�����ָʾ
	unsigned short	jam_flag;//ӵ��ָʾ

	/***************sig received*****************/
	Signal	data_rly_tx_cfm[DATA_PRI_NUM][MAX_NODE_CNT];
	Data_Rly_Tx_Cfm_Param data_rly_tx_cfm_param[DATA_PRI_NUM][MAX_NODE_CNT];

	Signal  data_rly_rx_ind;
	Data_Rly_Rx_Ind_Param data_rly_rx_ind_param;
	/***************sig to send*****************/
	Signal*	data_rly_tx_req[DATA_PRI_NUM][MAX_NODE_CNT];
	Signal*	data_rly_tx_jam_ind;
}Data_Rly_Tx;

extern void DataRlyTxInit(Data_Rly_Tx* proc);
extern void DataRlyTxSetup(Data_Rly_Tx* proc);


#endif