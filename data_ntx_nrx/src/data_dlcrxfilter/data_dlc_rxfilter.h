#ifndef _DATA_DLC_RX_FILTER_H
#define _DATA_DLC_RX_FILTER_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"

typedef struct _t_data_nrx_ind_param
{
	unsigned short	*net_pkt;
	unsigned short	len;
	unsigned short	sa;
}T_Data_Nrx_Ind_Param;


typedef struct _data_dlc_rxfilter
{
	void *entity;
	Mib *mib;
	u8 state;

	/***************sig received*****************/

	/***************sig to send*****************/
	Signal*	data_nrx_ind[DATA_PRI_NUM][MAX_NODE_CNT];
}Data_Dlc_RxFilter;


extern void DataDlcRxFilterSetup(Data_Dlc_RxFilter* proc);
extern void DataDlcRxFilterInit(Data_Dlc_RxFilter* proc);


#endif