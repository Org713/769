#include "data_interface_rx.h"

static int DataInterfaceRxInd(Signal *sig)
{
	Data_Interface_Rx *proc = (Data_Interface_Rx*)sig->dst;
	Data_Interface_Rx_Ind_Param *param = (Data_Interface_Rx_Ind_Param*)sig->param;

	sig = proc->svc_data_rx_ind;
	((T_Svc_Data_Rx_Ind_Param*)sig->param)->data = param->data;
	((T_Svc_Data_Rx_Ind_Param*)sig->param)->len = param->len;
	((T_Svc_Data_Rx_Ind_Param*)sig->param)->sa = param->sa;
	((T_Svc_Data_Rx_Ind_Param*)sig->param)->pri = param->pri;
	((T_Svc_Data_Rx_Ind_Param*)sig->param)->svc_type = param->svc_type;
	AddSignal(sig);
	printf("Data_Interface_Rx[%d]::DataInterfaceRxInd() 收到数据，发送svc_data_rx_ind。\n",proc->data_pri_id);

	return 0;
}


void DataInterfaceRxInit(Data_Interface_Rx* proc)
{

}
void DataInterfaceRxSetup(Data_Interface_Rx*  proc)
{
	unsigned short i;
	for ( i = 0; i < DATA_PRI_NUM; i++)     //手误，将DATA_PRI_NUM写成了MAX_NODE_CNT!! 2016.12.28
	{
		proc->data_interface_rx_ind.next=0;
		proc->data_interface_rx_ind.src=0;
		proc->data_interface_rx_ind.dst=proc;
		proc->data_interface_rx_ind.func=DataInterfaceRxInd;
		proc->data_interface_rx_ind.pri=SDL_PRI_NORM;
		proc->data_interface_rx_ind.param=&proc->data_interface_rx_ind_param;
	}
}
