#ifndef _RT_FIND_H
#define _RT_FIND_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"

typedef struct _data_ntx_rt_find_req_param
{
	unsigned short da;
}Data_Ntx_Rt_Find_Req_Param;

typedef struct _t_data_ntx_rt_find_cfm_param
{
	unsigned short ra;
}T_Data_Ntx_Rt_Find_Cfm_Param;

typedef struct _rt_find
{
	void *entity;
	Mib *mib;
	u8 state;

	/***************sig received*****************/
	Signal  data_ntx_rt_find_req[DATA_PRI_NUM][MAX_NODE_CNT];
	Data_Ntx_Rt_Find_Req_Param data_ntx_rt_find_req_param[DATA_PRI_NUM][MAX_NODE_CNT];

	/***************sig to send*****************/
	Signal*	data_ntx_rt_find_cfm[DATA_PRI_NUM][MAX_NODE_CNT];
}Rt_Find;

extern void RtFindInit(Rt_Find* proc);
extern void RtFindSetup(Rt_Find* proc);


#endif