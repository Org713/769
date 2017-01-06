#include "data_rly_tx.h"
enum{IDLE, WAIT_TX_CFM};

//���뵽�����У�����ifLocalNetPkt=1�������������=0�����м������
static int WtRlyQue(Data_Rly_Tx* proc, Net_Pkt *net_pkt, unsigned short pri)
{
	unsigned short test;
	unsigned short btm,top,size;
	top = proc->rly_que_list.top;
	btm = proc->rly_que_list.btm;
	size = proc->rly_que_list.size; 
	if(size >= RLY_QUE_MAX_NUM)  //20
	{
		printf("rly_que_list() ������\n");
		return 0;
	}
	else
	{
		proc->rly_que_list.que_elmt[top].pri = pri;
		proc->rly_que_list.que_elmt[top].TTL = RLY_QUE_MAX_TTL;
		memcpy(&(proc->rly_que_list.que_elmt[top].net_pkt), net_pkt, sizeof(Net_Pkt));//2017.1.5

		proc->rly_que_list.top++;
		proc->rly_que_list.top %= RLY_QUE_MAX_NUM;
		proc->rly_que_list.size++;
		return 1;
	}	
}

//������
int RdRlyQue(Data_Rly_Tx* proc,unsigned short **net_pkt,unsigned short *da, unsigned short *len, unsigned short *pri)
{	
	unsigned short btm;
	if (proc->rly_que_list.size != 0)
	{
		btm = proc->rly_que_list.btm;
		*net_pkt		= (unsigned short*)&proc->rly_que_list.que_elmt[btm].net_pkt;
		*da			= proc->rly_que_list.que_elmt[btm].net_pkt.da;
		//*len		= proc->rly_que_list.que_elmt[btm].net_pkt.len;
		*len		= sizeof(proc->rly_que_list.que_elmt[btm].net_pkt);//���ĳ��ȣ����ǰ������ݵĳ���
		*pri		= proc->rly_que_list.que_elmt[btm].net_pkt.pri;
		return 1;
	}
	return 0;
}

//ɾ�������е������
void DeleteRlyQue(Data_Rly_Tx* proc)
{
	unsigned short btm=proc->rly_que_list.btm;
	if(proc->rly_que_list.size)
	{
		printf("DeleteRlyQue() ɾ�������������\n");

		proc->rly_que_list.que_elmt[btm].pri = 0;
		proc->rly_que_list.que_elmt[btm].TTL = 0;

		proc->rly_que_list.que_elmt[btm].net_pkt.svc_type = 0;
		proc->rly_que_list.que_elmt[btm].net_pkt.pkt_sn = 0;
		proc->rly_que_list.que_elmt[btm].net_pkt.da = 0;
		proc->rly_que_list.que_elmt[btm].net_pkt.sa = 0;
		proc->rly_que_list.que_elmt[btm].net_pkt.pri = 0;
		proc->rly_que_list.que_elmt[btm].net_pkt.len = 0;
		memset(proc->rly_que_list.que_elmt[btm].net_pkt.net_payload, 0, NET_PAYLOAD_SIZE_TEST);

		proc->rly_que_list.btm ++;
		proc->rly_que_list.btm %= RLY_QUE_MAX_NUM;
		proc->rly_que_list.size --;
	}

}


static int DataRlyTxCfm(Signal* sig)
{
	Data_Rly_Tx *proc = (Data_Rly_Tx*)sig->dst;
	Data_Rly_Tx_Cfm_Param *param = (Data_Rly_Tx_Cfm_Param*)sig->param;
	
	unsigned short	isSucc_flag;
	unsigned short 	da;
	unsigned short  pri;
	unsigned short	*temp_net_pkt;//ȡ�����ʱ��ʱ���
	unsigned short	len;
	unsigned short	ttl;
	unsigned short  btm;

	if (proc->state == WAIT_TX_CFM)
	{
		if (param->cfm_flag == SUCC)
		{
			DeleteRlyQue(proc);//ɾ���ð�
			while (proc->rly_que_list.size != 0)	//����δ���Ͱ�
			{
				btm = proc->rly_que_list.btm;
				ttl = proc->rly_que_list.que_elmt[btm].TTL;
				if (ttl == 0) //�����ttl����
				{
					DeleteRlyQue(proc);
				}
				else  //û�г���
				{
					isSucc_flag = RdRlyQue(proc, &temp_net_pkt, &da, &len, &pri);
					if (isSucc_flag)
					{
						printf("ȡ�м̶���\n");
					}
					sig = proc->data_rly_tx_req[pri][da];
					((T_Data_Rly_Tx_Req_Param*)sig->param)->net_pkt = temp_net_pkt;
					((T_Data_Rly_Tx_Req_Param*)sig->param)->da = da;
					((T_Data_Rly_Tx_Req_Param*)sig->param)->len = len;
					((T_Data_Rly_Tx_Req_Param*)sig->param)->pri = pri;
					AddSignal(sig);

					//�Ƿ�С����ʹ洢�ռ�
					if (proc->rly_que_list.size < MIN_STORE_SPACE)//С����ʹ洢�ռ�
					{
						if (!proc->jam_flag)//����ӵ�����ָʾ
						{
							return 0;
						}
						else  //û�и���ӵ�����ָʾ
						{
							sig = proc->data_rly_tx_jam_ind;
							((T_Data_Rly_Tx_Jam_Ind_Param*)sig->param)->jam_flag = 0;
							AddSignal(sig);
							proc->jam_flag = 0;//ӵ�����
							return 0;
						}
					}
					else
					{
						return 0;//��С����ʹ洢�ռ�
					}
				}
			}
			proc->state = IDLE;//û�����������
			return 0;
		}
		else    //��һ�� ����ʧ����
		{
			//�м����������ʧ�ܣ�������㲻�����·���
		}
	}
	return 0;
}

static int DataRlyRxInd(Signal *sig)
{
	Data_Rly_Tx *proc = (Data_Rly_Tx*)sig->dst;
	Data_Rly_Rx_Ind_Param *param = (Data_Rly_Rx_Ind_Param*)sig->param;

	unsigned short	isSucc_flag;
	unsigned short 	da;
	unsigned short  pri;
	unsigned short	*temp_net_pkt;//ȡ�����ʱ��ʱ���
	unsigned short	len;//���ĳ���

	if (proc->state == IDLE)
	{
		//�ŵ��м̶����У������ð�����ttlֵ��
		isSucc_flag = WtRlyQue(proc, (Net_Pkt*)param->net_pkt, param->pri);
		if (isSucc_flag)
		{
			printf("�ŵ��м̶�����\n");
		}
		//��ȡ����
		isSucc_flag = RdRlyQue(proc, &temp_net_pkt, &da, &len, &pri);
		if (isSucc_flag)
		{
			printf("ȡ�м̶���\n");
		}
		if (MAX_NODE_CNT < da) //�㲥or�鲥
		{
			da = proc->mib->local_id;	//2017.1.6
			sig = proc->data_rly_tx_req[pri][da];
		}
		else  //����
		{
			sig = proc->data_rly_tx_req[pri][da];
		}
		((T_Data_Rly_Tx_Req_Param*)sig->param)->net_pkt = temp_net_pkt;
		((T_Data_Rly_Tx_Req_Param*)sig->param)->da = da;
		((T_Data_Rly_Tx_Req_Param*)sig->param)->len = len;
		((T_Data_Rly_Tx_Req_Param*)sig->param)->pri = pri;
		AddSignal(sig);
		proc->state = WAIT_TX_CFM;
	}
	else
	{
		//�ŵ��м̶����У������ð�����ttlֵ��
		isSucc_flag = WtRlyQue(proc, (Net_Pkt*)param->net_pkt, param->pri);
		if (isSucc_flag)
		{
			printf("�ŵ��м̶�����\n");
		}
		//size�Ƿ�������洢�ռ����
		if (proc->rly_que_list.size > MAX_STORE_SPACE) //size > ���洢�ռ����
		{
			if (proc->jam_flag)  //����ӵ��ָʾ
			{
				return 0;
			}
			else  //û�и���ӵ��ָʾ
			{
				sig = proc->data_rly_tx_jam_ind;
				((T_Data_Rly_Tx_Jam_Ind_Param*)sig->param)->jam_flag = 1;
				AddSignal(sig);
				proc->jam_flag = 1;
				return 0;
			}
		}
		else  
		{
			return 0;
		}
	}
	return 0;
}


void DataRlyTxInit(Data_Rly_Tx* proc)
{
	proc->state = IDLE;
	proc->jam_flag = 0xff;
}
void DataRlyTxSetup(Data_Rly_Tx* proc)
{
	unsigned short i,j;
	Signal* sig;
	for (i = 0; i < DATA_PRI_NUM; i++)
	{
		for (j = 0; j < MAX_NODE_CNT; j++)
		{
			sig = &proc->data_rly_tx_cfm[i][j];
			sig->next=0;
			sig->src=0;
			sig->dst=proc;
			sig->func=DataRlyTxCfm;
			sig->pri=SDL_PRI_NORM;
			sig->param = &proc->data_rly_tx_cfm_param[i][j];
		}
	}

	sig = &proc->data_rly_rx_ind;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataRlyRxInd;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->data_rly_rx_ind_param;
}
	