#include "data_nrx.h"

enum{IDLE};

static int DataNtxToDataNrxTest(Signal *sig)
{
	Data_Nrx *proc = (Data_Nrx*)sig->dst;
	Data_Ntx_To_Data_Nrx_Test_Param *param = (Data_Ntx_To_Data_Nrx_Test_Param*)sig->param;

	sig = proc->data_interface_rx_ind;
	((T_Data_Interface_Rx_Ind_Param*)sig->param)->data = param->data;
	((T_Data_Interface_Rx_Ind_Param*)sig->param)->len = param->len;
	((T_Data_Interface_Rx_Ind_Param*)sig->param)->sa = param->sa;
	((T_Data_Interface_Rx_Ind_Param*)sig->param)->pri = param->pri;
	((T_Data_Interface_Rx_Ind_Param*)sig->param)->svc_type = param->svc_type;
	AddSignal(sig);
	printf("Data_Nrx[%d]::DataNtxToDataNrxTest()�յ����ݣ�����data_interface_rx_ind��\n",proc->module_id);

	return 0;
}

static int DataNrxInd(Signal *sig)
{
	Data_Nrx *proc = (Data_Nrx*)sig->dst;
	Data_Nrx_Ind_Param *param = (Data_Nrx_Ind_Param*)sig->param;

	unsigned short da,receive_pkt_sn;
	da = ((Net_Pkt*)param->net_pkt)->da;//ȡ��da
	receive_pkt_sn = ((Net_Pkt*)param->net_pkt)->pkt_sn;
	if (proc->state == IDLE)
	{
		if (da == proc->mib->local_id)  //���ص�����
		{
			if (proc->unicast_pkt_sn != receive_pkt_sn)//û�յ���
			{
				sig = proc->data_interface_rx_ind;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->data = ((Net_Pkt*)param->net_pkt)->net_payload;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->len = ((Net_Pkt*)param->net_pkt)->len;//���ݳ���,���ǰ�����
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->pri = ((Net_Pkt*)param->net_pkt)->pri;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->sa = ((Net_Pkt*)param->net_pkt)->sa;//param->sa??
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->svc_type = ((Net_Pkt*)param->net_pkt)->svc_type;
				AddSignal(sig);
			}
			else  //�յ���
			{
				return 0;
			}
		}
		else if ( (da!=proc->mib->local_id) && (da>=0) && (da<MAX_NODE_CNT) )   //����Ŀ�Ľڵ㵥����
		{
			if (proc->rly_unicast_pkt_sn != receive_pkt_sn)//û�յ���
			{
				sig = proc->data_rly_rx_ind;
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->net_pkt = param->net_pkt;
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->len = param->len;//���ĳ���
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->pri = ((Net_Pkt*)param->net_pkt)->pri;
				AddSignal(sig);
			}
			else  //�յ���
			{
				return 0;
			}
		}
		else if (da == 0xFF) //�㲥��
		{
			if (proc->broadcast_pkt_sn != receive_pkt_sn)//û�յ���
			{
				sig = proc->data_rly_rx_ind;//����û���յ����Ĺ㲥��ҲҪ�м̵�
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->net_pkt = param->net_pkt;
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->len = param->len;//���ĳ���
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->pri = ((Net_Pkt*)param->net_pkt)->pri;
				AddSignal(sig);

				sig = proc->data_interface_rx_ind;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->data = ((Net_Pkt*)param->net_pkt)->net_payload;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->len = ((Net_Pkt*)param->net_pkt)->len;//���ݳ���,���ǰ�����
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->pri = ((Net_Pkt*)param->net_pkt)->pri;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->sa = ((Net_Pkt*)param->net_pkt)->sa;//param->sa??
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->svc_type = ((Net_Pkt*)param->net_pkt)->svc_type;
				AddSignal(sig);
			}
			else  //�յ���
			{
				return 0;
			}
		}
		else if (da == 0xFE) //�鲥��
		{
			if (proc->multicast_pkt_sn != receive_pkt_sn)//û�յ���
			{
				sig = proc->data_interface_rx_ind;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->data = ((Net_Pkt*)param->net_pkt)->net_payload;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->len = ((Net_Pkt*)param->net_pkt)->len;//���ݳ���,���ǰ�����
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->pri = ((Net_Pkt*)param->net_pkt)->pri;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->sa = ((Net_Pkt*)param->net_pkt)->sa;//param->sa??
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->svc_type = ((Net_Pkt*)param->net_pkt)->svc_type;
				AddSignal(sig);
			}
			else  //�յ���
			{
				return 0;
			}
		}
	}
	return 0;
}


void DataNrxInit(Data_Nrx* proc)
{
	proc->state = IDLE;
	proc->unicast_pkt_sn = 0xff;		//����  unicast
	proc->rly_unicast_pkt_sn = 0xff;	//����Ŀ�Ľڵ㵥�� rly_unicast
	proc->broadcast_pkt_sn = 0xff;	//�㲥  broadcast
	proc->multicast_pkt_sn = 0xff;	//�鲥  multicast
}

void DataNrxSetup(Data_Nrx* proc)
{
	Signal *sig;
	//unsigned short i;	
	sig = &proc->data_ntx_to_data_nrx_test;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataNtxToDataNrxTest;
	sig->pri=SDL_PRI_NORM;
	sig->param=&proc->data_ntx_to_data_nrx_test_param;

	sig = &proc->data_nrx_ind;
	sig->next = 0;
	sig->src = 0;
	sig->dst = proc;
	sig->func = DataNrxInd;
	sig->pri = SDL_PRI_NORM;
	sig->param = &proc->data_nrx_ind_param;
}