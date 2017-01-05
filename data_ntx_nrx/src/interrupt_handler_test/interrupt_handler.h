#ifndef _INTERRUPT_HANDLER_H
#define _INTERRUPT_HANDLER_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"

#define TEST_PKT_NUM 20 
#define DATA_LENGTH 100 //数据长度
//发数据统计
typedef struct _send_data_statistics
{
	u16 pkt_num;
	u16 len[TEST_PKT_NUM];
	u16 pri[TEST_PKT_NUM];
}Send_Data_Statistics;
//收数据统计
typedef struct _receive_data_statistics
{
	u16 pkt_num;
	u16 len[TEST_PKT_NUM];
	u16 pri[TEST_PKT_NUM];
}Receive_Data_Statistics;

typedef struct _receive_buffer
{
	u16 svc_type;
	u16 data[DATA_LENGTH];
}Receive_Buffer;
//优先级 收 buffer
typedef struct _receive_data_pri
{
	u16 pkt_num;
	Receive_Buffer buffer[TEST_PKT_NUM];
}Receive_Data_Pri;

//测试指示信号
typedef struct _test_ind_param
{
	unsigned short svc_type;
	unsigned short pkt_sn;
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	unsigned short *data;//数据
}Test_Ind_Param;


typedef struct _t_svc_data_tx_req_param
{
	unsigned short svc_type;
	unsigned short pkt_sn;
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	unsigned short *data;//数据
}T_Svc_Data_Tx_Req_Param;

typedef struct _svc_data_tx_cfm_param
{
	unsigned short cfm_flag;//0 1 2 3 4
	unsigned short pri;
	unsigned short pkt_sn;
}Svc_Data_Tx_Cfm_Param;


typedef struct _svc_data_rx_ind_param
{
	unsigned short *data;
	unsigned short len;
	unsigned short sa;
	unsigned short pri;
	unsigned short svc_type;
}Svc_Data_Rx_Ind_Param;

typedef struct _interrupt_handler
{
	void *entity;
	Mib *mib;
	u8 state;

	Send_Data_Statistics send_data_statistics[MAX_NODE_CNT];//发送统计 2016.12.28
	Receive_Data_Statistics receive_data_statistics[MAX_NODE_CNT];//接收统计

	Receive_Data_Pri receive_data_pri[DATA_PRI_NUM];//4个优先级buffer

	/***************sig received*****************/
	Signal	test_ind;
	Test_Ind_Param test_ind_param;

	Signal	svc_data_tx_cfm[DATA_PRI_NUM];
	Svc_Data_Tx_Cfm_Param svc_data_tx_cfm_param[DATA_PRI_NUM];

	Signal	svc_data_rx_ind[DATA_PRI_NUM];
	Svc_Data_Rx_Ind_Param svc_data_rx_ind_param[DATA_PRI_NUM];
	/***************sig to send*****************/
	Signal *svc_data_tx_req[DATA_PRI_NUM];//给不同的优先级 data_interface_tx[0--3]

}Interrupt_Handler;

extern void InterruptHandlerInit(Interrupt_Handler* proc);
extern void InterruptHandlerSetup(Interrupt_Handler* proc);


#endif