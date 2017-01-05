#ifndef _DATA_INTERFACE_TX_H
#define _DATA_INTERFACE_TX_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"


#define DATA_QUE_MAX_NUM 20//数据队列长度
#define DATA_LENGTH 100 //数据长度

typedef struct _svc_data_tx_req_param
{
	unsigned short svc_type;
	unsigned short pkt_sn;
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	unsigned short *data;//数据
}Svc_Data_Tx_Req_Param;

typedef struct _t_data_interface_tx_req_param
{
	unsigned short *data;
	unsigned short len;
	unsigned short da;
	unsigned short pri;
	unsigned short svc_type;
	unsigned short pkt_sn;//2016.12.30 包序号 cfm使用
}T_Data_Interface_Tx_Req_Param;


typedef struct _data_interface_tx_cfm_param
{
	unsigned short cfm_flag;//0 1 2
	unsigned short cfm_pkt_sn;//2016.12.30  回复cfm包序号
}Data_Interface_Tx_Cfm_Param;

typedef struct _t_svc_data_tx_cfm_param
{
	unsigned short cfm_flag;//0 1 2 3 4
	unsigned short pri;
	unsigned short pkt_sn;
}T_Svc_Data_Tx_Cfm_Param;

//数据队列中的元素
typedef struct _data_que_elmt
{
	unsigned short svc_type;
	unsigned short pkt_sn;
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	unsigned short data[DATA_LENGTH];//数据
	//unsigned short *data;//数据

}Data_Que_Elmt;
//本地数据队列 同一个数据优先级
typedef struct _data_que_list
{
	unsigned short top;
	unsigned short btm;
	unsigned short size;
	Data_Que_Elmt data_que_elmt[DATA_QUE_MAX_NUM];

}Data_Que_List;

typedef struct _data_interface_tx
{
	void *entity;
	Mib *mib;
	u8 state;
	unsigned short data_pri_id;//本模块的id号（与数据优先级相等）[0.1.2.3]  2016.12.27

	Data_Que_List data_que_list;//数据包队列

	unsigned short *data;//保存 要发送的数据相关
	unsigned short pri;
	unsigned short pkt_sn;
	unsigned short da;
	unsigned short svc_type;
	unsigned short len;

	Timer tx_cfm_tmr;
	/***************sig received*****************/
	Signal	svc_data_tx_req;
	Svc_Data_Tx_Req_Param svc_data_tx_req_param;

	Signal	data_interface_tx_cfm;
	Data_Interface_Tx_Cfm_Param data_interface_tx_cfm_param;

	/***************sig to send*****************/
	Signal*	data_interface_tx_req[MAX_NODE_CNT];//发向不同[da]
	Signal*	svc_data_tx_cfm;
	

}Data_Interface_Tx;

extern void DataInterfaceTxSetup(Data_Interface_Tx*);
extern void DataInterfaceTxInit(Data_Interface_Tx*);

#endif