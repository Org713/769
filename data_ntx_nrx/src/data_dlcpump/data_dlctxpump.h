#ifndef _DATA_DLC_TX_PUMP_H
#define _DATA_DLC_TX_PUMP_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"

typedef struct _dlc_txpump_tx_req_param
{
	unsigned short *net_pkt;
	unsigned short len;
	unsigned short ra;
	unsigned short pri;
	unsigned short data_pri_id;//后面这两个参数是为了cfm时能找到对应的data_ntx[][]   2017.1.4
	unsigned short module_id;
}Dlc_Txpump_Tx_Req_Param;

typedef struct _t_dlc_txpump_tx_cfm_param
{
	unsigned short cfm_flag;
	unsigned short nsn;
}T_Dlc_Txpump_Tx_Cfm_Param;

typedef struct _data_dlctxpump
{
	void *entity;
	Mib *mib;
	u8 state;

	/***************sig received*****************/
	Signal	dlc_txpump_tx_req[DATA_PRI_NUM][MAX_NODE_CNT];
	Dlc_Txpump_Tx_Req_Param dlc_txpump_tx_req_param[DATA_PRI_NUM][MAX_NODE_CNT];

	/***************sig to send*****************/
	Signal*	dlc_txpump_tx_cfm[DATA_PRI_NUM][MAX_NODE_CNT];
}Data_Dlctxpump;


extern void DataDlcTxPumpSetup(Data_Dlctxpump* proc);
extern void DataDlcTxPumpInit(Data_Dlctxpump* proc);


#endif