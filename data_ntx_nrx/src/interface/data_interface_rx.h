#ifndef _DATA_INTERFACE_RX_H
#define _DATA_INTERFACE_RX_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"


typedef struct _data_interface_rx_ind_param
{
	unsigned short *data;//数据
	unsigned short len;
	unsigned short sa;
	unsigned short pri;
	unsigned short svc_type;
}Data_Interface_Rx_Ind_Param;

typedef struct _t_svc_data_rx_ind_param
{
	unsigned short *data;
	unsigned short len;
	unsigned short sa;
	unsigned short pri;
	unsigned short svc_type;
}T_Svc_Data_Rx_Ind_Param;

typedef struct _data_interface_rx
{
	void *entity;
	Mib *mib;
	u8 state;
	
	unsigned short data_pri_id;//本模块的id号（与数据优先级相等）[0.1.2.3]  2016.12.27


	/***************sig received*****************/
	Signal	data_interface_rx_ind;
	Data_Interface_Rx_Ind_Param  data_interface_rx_ind_param;
	/***************sig to send*****************/
	Signal *svc_data_rx_ind;
	

}Data_Interface_Rx;

extern void DataInterfaceRxSetup(Data_Interface_Rx* proc);
extern void DataInterfaceRxInit(Data_Interface_Rx* proc);

#endif