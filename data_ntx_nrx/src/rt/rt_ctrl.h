#ifndef _RT_CTRL_H
#define _RT_CTRL_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"


typedef struct _data_ntx_rt_req_param
{
	unsigned short da;
	unsigned short fail_ra[3];
	unsigned short source_data_pri_id;// 为了路由cfm找到本data_ntx[?][da]   2017.1.4
}Data_Ntx_Rt_Req_Param;

typedef struct _t_data_ntx_rt_cfm_param
{
	unsigned short cfm_flag;
	unsigned short ra;
}T_Data_Ntx_Rt_Cfm_Param;

typedef struct _rt_ctrl
{
	void *entity;
	Mib *mib;
	u8 state;

	/***************sig received*****************/
	Signal	data_ntx_rt_req[DATA_PRI_NUM][MAX_NODE_CNT];
	Data_Ntx_Rt_Req_Param data_ntx_rt_req_param[DATA_PRI_NUM][MAX_NODE_CNT];

	/***************sig to send*****************/
	Signal*	data_ntx_rt_cfm[DATA_PRI_NUM][MAX_NODE_CNT];
}Rt_Ctrl;

extern void RtCtrlInit(Rt_Ctrl* proc);
extern void RtCtrlSetup(Rt_Ctrl* proc);


#endif