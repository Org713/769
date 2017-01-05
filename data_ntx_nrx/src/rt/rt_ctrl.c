#include "rt_ctrl.h"

static int DataNtxRtReq(Signal* sig)
{
	Rt_Ctrl *proc = (Rt_Ctrl*)sig->dst;
	Data_Ntx_Rt_Req_Param *param = (Data_Ntx_Rt_Req_Param*)sig->param;
	//查找路由，找到到达param->da的下一跳ra
	unsigned short da,source_data_pri_id;
	//test
	da = param->da;
	source_data_pri_id = param->source_data_pri_id;
	sig = proc->data_ntx_rt_cfm[source_data_pri_id][da];
	((T_Data_Ntx_Rt_Cfm_Param*)sig->param)->cfm_flag = SUCC;
	((T_Data_Ntx_Rt_Cfm_Param*)sig->param)->ra = 2;
	AddSignal(sig);


	return 0;
}

void RtCtrlInit(Rt_Ctrl* proc)
{

}
void RtCtrlSetup(Rt_Ctrl* proc)
{
	unsigned short i,j;
	Signal* sig;
	for (i = 0; i < DATA_PRI_NUM; i++)
	{
		for (j = 0; j < MAX_NODE_CNT; j++)
		{
			sig = &proc->data_ntx_rt_req[i][j];
			sig->next=0;
			sig->src=0;
			sig->dst=proc;
			sig->func=DataNtxRtReq;
			sig->pri=SDL_PRI_NORM;
			sig->param = &proc->data_ntx_rt_req_param[i][j];
		}
	}
}