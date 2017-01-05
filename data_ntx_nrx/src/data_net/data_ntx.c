#include "data_ntx.h"

enum{IDLE,WAIT_TX_CFM,WAIT_RT_CFM,WAIT_RT_FIND_CFM};
enum{NO_REACH,SEND_SUCCESS,NET_BUSY,WAIT_SEND,SENT};

//������������������д��ָ���ĵ�ַ��  
static void WtNetPkt(Data_Ntx* proc, Net_Pkt* net_pkt, Data_Interface_Tx_Req_Param* param)
{
	//unsigned short test;
	//memset(des_data,0,(param->len + NET_FRM_HEAD_LEN));  //yun:������ռ� ���
	//test = sizeof(net_pkt);
	memset(net_pkt, 0, sizeof(Net_Pkt));  // ���������

	//������֡ͷ,������ţ�����ڷ��͵�ʱ��Ż���д��
	//((Net_Frm*)des_data)->pri_sa_da_mul_sn = (proc->mib->local_id) << 8;  //yun:Դ��ַ
	//((Net_Frm*)des_data)->pri_sa_da_mul_sn |= (da & 0x1f)<<3;  //yun:Ŀ�ĵ�ַ
	//((Net_Frm*)des_data)->pri_sa_da_mul_sn |= pri<<13;  //yun:���ȼ�
	//((Net_Frm*)des_data)->sn_len |= (len &0x3ff);  //yun:���� ��û���

	net_pkt->svc_type = param->svc_type;
	//net_pkt->pkt_sn = proc->pkt_sn;//���ط�������
	net_pkt->pkt_sn = param->pkt_sn;//�ϲ�data_interface_tx����
	net_pkt->da = param->da;
	net_pkt->sa = proc->mib->local_id;
	net_pkt->pri = param->pri;
	net_pkt->len = param->len;

	memcpy(net_pkt->net_payload, param->data, param	->len);  //û�������ͷ
	//proc->pkt_sn++;
	return;
}

//���뵽�����У�����ifLocalNetPkt=1�������������=0�����м������
static int WtNetQue(Data_Ntx* proc, Net_Pkt *net_pkt, unsigned short ifLocalNetPkt)
{
	unsigned short test;
	unsigned short btm,top,size;
	top = proc->net_que_list.top;
	btm = proc->net_que_list.btm;
	size = proc->net_que_list.size; 
	if(size >= NET_QUE_MAX_NUM)  //20
	{
		printf("net_que_list() ������\n");
		return 0;
	}
	else
	{
		if (ifLocalNetPkt)  //���������
		{
			proc->net_que_list.que_elmt[top].pkt_type = LOCAL_NET_PKT;
			proc->net_que_list.que_elmt[top].TTL = NET_PKT_MAX_TTL;
		}
		else
		{
			proc->net_que_list.que_elmt[top].pkt_type = RLY_NET_PKT;
			proc->net_que_list.que_elmt[top].TTL = NET_PKT_MAX_TTL;
		}
		test = sizeof(Net_Pkt);
		memcpy(&(proc->net_que_list.que_elmt[top].net_pkt), net_pkt, sizeof(Net_Pkt));//���ﻹûȷ�����������ͷ����

		proc->net_que_list.top++;
		proc->net_que_list.top %= NET_QUE_MAX_NUM;
		proc->net_que_list.size++;
		return 1;
	}	
}

//������
int RdNetQue(Data_Ntx* proc,unsigned short **net_pkt,unsigned short *da, unsigned short *len, unsigned short *pri, unsigned short *pkt_type)
{	
	unsigned short btm;
	if (proc->net_que_list.size != 0)
	{
		btm = proc->net_que_list.btm;
		*net_pkt		= (unsigned short*)&proc->net_que_list.que_elmt[btm].net_pkt;
		*da			= proc->net_que_list.que_elmt[btm].net_pkt.da;
		*len		= proc->net_que_list.que_elmt[btm].net_pkt.len;
		*pri		= proc->net_que_list.que_elmt[btm].net_pkt.pri;
		*pkt_type	= proc->net_que_list.que_elmt[btm].pkt_type;
		return 1;
	}
	return 0;
}

//ɾ�������е������
void DeleteNetPkt(Data_Ntx* proc)
{
	unsigned short btm=proc->net_que_list.btm;
	if(proc->net_que_list.size)
	{
		printf("DeleteNetPkt() ɾ�������������\n");

		proc->net_que_list.que_elmt[btm].pkt_type = 0;
		proc->net_que_list.que_elmt[btm].TTL = 0;

		proc->net_que_list.que_elmt[btm].net_pkt.svc_type = 0;
		proc->net_que_list.que_elmt[btm].net_pkt.pkt_sn = 0;
		proc->net_que_list.que_elmt[btm].net_pkt.da = 0;
		proc->net_que_list.que_elmt[btm].net_pkt.sa = 0;
		proc->net_que_list.que_elmt[btm].net_pkt.pri = 0;
		proc->net_que_list.que_elmt[btm].net_pkt.len = 0;
		memset(proc->net_que_list.que_elmt[btm].net_pkt.net_payload, 0, sizeof(proc->net_que_list.que_elmt[btm].net_pkt.net_payload));
		
		proc->net_que_list.btm ++;
		proc->net_que_list.btm %= NET_QUE_MAX_NUM;
		proc->net_que_list.size --;
	}

}

static int DataInterfaceTxReq(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	Data_Interface_Tx_Req_Param *param = (Data_Interface_Tx_Req_Param*)sig->param;

	//unsigned short svc_type;
	unsigned short isSucc_flag;
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	Net_Pkt net_pkt;//�������
	unsigned short *temp_net_pkt;//ȡ�����ʱ��ʱ���
	//unsigned short pkt_sn;//�ϲ�İ� ��� 2016.12.30
	unsigned short pkt_type;
	printf("Data_Ntx[%d]::DataInterfaceTxReq()�յ�req,\n",proc->module_id);

	//svc_type = param->svc_type;
	//pri = param->pri;
	//da = param->da;
	//len = param->len;
	//memcpy(proc->rcv_data, param->data, param->len);//����Ҫ���͵����� 2016.12.28

	//sig = proc->data_interface_tx_cfm;
	//((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_flag = SEND_SUCCESS;
	//((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_pkt_sn = param->pkt_sn;
	//AddSignal(sig);

	////���ԣ�����������
	//sig = proc->data_ntx_to_data_nrx_test;
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->data = proc->rcv_data;//2016.12.28
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->len = param->len;
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->da = param->da;
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->pri = param->pri;
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->svc_type = param->svc_type;
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->sa = proc->mib->local_id;
	//AddSignal(sig);
	//printf("Data_Ntx[%d]::DataInterfaceTxReq()����-->�ղ���\n",proc->module_id);
	

	if (proc->state != IDLE)
	{
		//���������
		WtNetPkt(proc, &net_pkt, param);

		//�ŵ������Ͷ����У����Ϊ�����������������ttlֵ
		isSucc_flag = WtNetQue(proc, &net_pkt, 1);

		if (isSucc_flag)
		{
			printf("�Ѿ��Ž�������\n");
		}
	}
	else if (proc->state == IDLE)
	{
		//�������
		WtNetPkt(proc, &net_pkt, param);
		//�ŵ������Ͷ����У����Ϊ�����������������ttlֵ
		isSucc_flag = WtNetQue(proc, &net_pkt, 1);
		if (isSucc_flag)
		{
			printf("�Ѿ��Ž�������\n");
		}
		//��ȡ �����е������   
		isSucc_flag = RdNetQue(proc, &temp_net_pkt, &da, &len, &pri, &pkt_type);
		if(isSucc_flag)
		{
			printf("ȡ�����\n");
		}
		//���ر����⼸�����������ź���ʹ�ã��м̡��ȴ�cfm
		proc->net_pkt = temp_net_pkt;
		proc->len = len;
		proc->pri = pri;
		proc->da = da;
		proc->pkt_type = pkt_type;

		if  ((MAX_NODE_CNT > da)&&(da >= 0))	//����
		{
			memset(proc->fail_ra, 0xff, sizeof(proc->fail_ra));
			proc->retry_cnt = 0;

			sig = proc->data_ntx_rt_req; //·�ɲ�ѯ
			((T_Data_Ntx_Rt_Req_Param*)sig->param)->da = da;
			memcpy(((T_Data_Ntx_Rt_Req_Param*)sig->param)->fail_ra, proc->fail_ra, sizeof(proc->fail_ra));
			((T_Data_Ntx_Rt_Req_Param*)sig->param)->local_data_pri_id = proc->data_pri_id;// Ϊ��·��cfm�ҵ���data_ntx[?][da]

			AddSignal(sig);
			
			proc->state = WAIT_RT_CFM;

		}
		else    //�ǵ���
		{
			proc->ra = 0xff;
			sig = proc->dlc_txpump_tx_req;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt	= (unsigned short*)proc->net_pkt;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len		= proc->len;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra			= proc->ra;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri		= proc->pri;
			AddSignal(sig);
			proc->wait_cfm_nsn = ((Net_Pkt*)proc->net_pkt)->pkt_sn;//�ȴ�cfm����������
			SetTimer(&proc->tx_req_timer,3);//��ʱ��ʱ�䣿������
			proc->state = WAIT_TX_CFM;
		}
	}
	return 0;
}

static int DataRlyTxReq(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	Data_Rly_Tx_Req_Param *param = (Data_Rly_Tx_Req_Param*)sig->param;
	
	if (proc->state != IDLE)
	{
		if (WtNetQue(proc, (Net_Pkt*)param->net_pkt, 0))
		{
			printf("�Ѿ��Ž�������\n");
		}
	}
	else if (proc->state == IDLE)
	{
		//�������DataInterfaceTxReq()��ͬ�������Ժ����
	}
	
	return 0;
}
//·�ɲ�ѯ��cfm
static int DataNtxRtCfm(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	Data_Ntx_Rt_Cfm_Param *param = (Data_Ntx_Rt_Cfm_Param*)sig->param;
	
	if (proc->state = WAIT_RT_CFM)
	{
		if (param->cfm_flag == SUCC)
		{
			proc->ra = param->ra;//·�ɲ�ѯ���� �սڵ�
			sig = proc->dlc_txpump_tx_req;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt		= (unsigned short*)proc->net_pkt;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len			= proc->len;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra				= proc->ra;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri			= proc->pri;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->data_pri_id	= proc->data_pri_id;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->module_id		= proc->module_id;

			AddSignal(sig);
			SetTimer(&proc->tx_req_timer,3);//��ʱ��ʱ�䣿������
			proc->wait_cfm_nsn = ((Net_Pkt*)proc->net_pkt)->pkt_sn;//�ȴ�cfm����������
			proc->state = WAIT_TX_CFM;
		}
		else
		{
			sig = proc->data_ntx_rt_find_req; //·�ɲ�ѯʧ�ܣ�����·��̽��
			((T_Data_Ntx_Rt_Find_Req_Param*)sig->param)->da = proc->da;
			AddSignal(sig);
			SetTimer(&proc->rt_find_timer,3);//��ʱ��ʱ�䣿������
			proc->state = WAIT_RT_FIND_CFM;
		}
	}	
	return 0;
}
//·��̽����cfm
static int DataNtxRtFindCfm(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	Data_Ntx_Rt_Find_Cfm_Param *param = (Data_Ntx_Rt_Find_Cfm_Param*)sig->param;
	if (proc->state == WAIT_RT_FIND_CFM)
	{
		if (param->ra != 0xff)  //�ҵ���һ��
		{
			proc->ra = param->ra;//����ra
			sig = proc->dlc_txpump_tx_req;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt	= (unsigned short*)proc->net_pkt;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len		= proc->len;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra			= proc->ra;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri		= proc->pri;
			AddSignal(sig);
			proc->wait_cfm_nsn = ((Net_Pkt*)proc->net_pkt)->pkt_sn;//�ȴ�cfm����������
			SetTimer(&proc->tx_req_timer,3);//��ʱ��ʱ�䣿������
			proc->state = WAIT_TX_CFM;
		}
		else  //δ�ҵ���һ��
		{
			sig = proc->data_interface_tx_cfm;
			((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_flag = FAIL;
			AddSignal(sig);
			proc->state = IDLE;
		}
	}
	return 0;
}

static int DlcTxpumpTxCfm(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	Dlc_Txpump_Tx_Cfm_Param *param = (Dlc_Txpump_Tx_Cfm_Param*)sig->param;
	
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	unsigned short pkt_type;
	unsigned short *temp_net_pkt;//ȡ�����ʱ��ʱ���
	unsigned short btm;
	unsigned short ttl;
	if (proc->state == WAIT_TX_CFM)   //2017.1.5�޸�
	{
		if (param->cfm_flag == SUCC)
		{
			if (proc->pkt_type == LOCAL_NET_PKT)//���������
			{
				if (param->nsn == proc->wait_cfm_nsn)//�ȴ�cfm����������
				{
					sig = proc->data_interface_tx_cfm;
					((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_flag = SUCC;
					((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_pkt_sn = proc->wait_cfm_nsn;//�ظ����ϲ�İ���ţ��ϲ�cfmʹ�ã�
					AddSignal(sig);
				}
				else
				{
					return 0;
				}
			}
			else if (proc->pkt_type == RLY_NET_PKT)//�м������
			{
				sig = proc->data_rly_tx_cfm;
				((T_Data_Rly_Tx_Cfm_Param*)sig->param)->cfm_flag = SUCC;
				AddSignal(sig);
			}
			//��ոð�,��ɾ���ð�
			DeleteNetPkt(proc);
			//�ж��Ƿ���δ���͵İ�
			while (proc->net_que_list.size != 0)//�����л��������
			{
				RdNetQue(proc, &temp_net_pkt, &da, &len, &pri, &pkt_type);
				btm = proc->net_que_list.btm;
				ttl = proc->net_que_list.que_elmt[btm].TTL;
				//�����ttl�Ƿ���
				if (ttl == 0)//����
				{
					//ɾ���ð�
					DeleteNetPkt(proc);
				}
				else  //δ����
				{
					//��ǰ��ĵ������ж�һ����
					proc->net_pkt = temp_net_pkt;//���ر��棬���ź���ʹ��
					proc->len = len;
					proc->pri = pri;
					proc->da = da;
					proc->pkt_type = pkt_type;
					if ((MAX_NODE_CNT > da)&&(da >= 0))	//����
					{
						memset(proc->fail_ra, 0xff, sizeof(proc->fail_ra));
						proc->retry_cnt = 0;

						sig = proc->data_ntx_rt_req; //·�ɲ�ѯ
						((T_Data_Ntx_Rt_Req_Param*)sig->param)->da = da;
						memcpy(((T_Data_Ntx_Rt_Req_Param*)sig->param)->fail_ra, proc->fail_ra, sizeof(proc->fail_ra));
						AddSignal(sig);
			
						proc->state = WAIT_RT_CFM;
					}
					else    //�ǵ���
					{
						proc->ra = 0xff;
						sig = proc->dlc_txpump_tx_req;
						((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt	= proc->net_pkt;
						((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len		= proc->len;
						((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra			= proc->ra;
						((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri		= proc->pri;
						AddSignal(sig);
						proc->wait_cfm_nsn = ((Net_Pkt*)proc->net_pkt)->pkt_sn;//�ȴ�cfm����������
						SetTimer(&proc->tx_req_timer,3);//��ʱ��ʱ�䣿������
						proc->state = WAIT_TX_CFM;
					}
					return 0;
				}
			}
			proc->state = IDLE;
			return 0;
		}
		else  //ʧ��
		{
			proc->retry_cnt++;
			if (proc->retry_cnt > 3)  //����ʧ�ܴ�������3��
			{
				if (proc->pkt_type == LOCAL_NET_PKT)//���������
				{
					sig = proc->data_interface_tx_cfm;
					((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_flag = FAIL;
					((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_pkt_sn = proc->wait_cfm_nsn;//�ظ����ϲ�İ���ţ��ϲ�cfmʹ�ã�
					AddSignal(sig);
				}
				else if (proc->pkt_type == RLY_NET_PKT)//�м������
				{
					sig = proc->data_rly_tx_cfm;
					((T_Data_Rly_Tx_Cfm_Param*)sig->param)->cfm_flag = FAIL;
					AddSignal(sig);
				}
				//��ոð�,��ɾ���ð�
				DeleteNetPkt(proc);
				//�ж��Ƿ���δ���͵İ�
				while (proc->net_que_list.size != 0)//�����л��������
				{
					RdNetQue(proc, &temp_net_pkt, &da, &len, &pri, &pkt_type);
					btm = proc->net_que_list.btm;
					ttl = proc->net_que_list.que_elmt[btm].TTL;
					if (ttl == 0)//����
					{
						//ɾ���ð�
						DeleteNetPkt(proc);
					}
					else  //δ����
					{
						//��ǰ��ĵ������ж�һ����
						proc->net_pkt = temp_net_pkt;//���ر��棬���ź���ʹ��
						proc->len = len;
						proc->pri = pri;
						proc->da = da;
						proc->pkt_type = pkt_type;
						if ((MAX_NODE_CNT > da)&&(da >= 0))	//����
						{
							memset(proc->fail_ra, 0xff, sizeof(proc->fail_ra));
							proc->retry_cnt = 0;

							sig = proc->data_ntx_rt_req; //·�ɲ�ѯ
							((T_Data_Ntx_Rt_Req_Param*)sig->param)->da = da;
							memcpy(((T_Data_Ntx_Rt_Req_Param*)sig->param)->fail_ra, proc->fail_ra, sizeof(proc->fail_ra));
							AddSignal(sig);
			
							proc->state = WAIT_RT_CFM;
						}
						else    //�ǵ���
						{
							proc->ra = 0xff;
							sig = proc->dlc_txpump_tx_req;
							((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt	= proc->net_pkt;
							((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len		= proc->len;
							((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra			= proc->ra;
							((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri		= proc->pri;
							AddSignal(sig);
							proc->wait_cfm_nsn = ((Net_Pkt*)proc->net_pkt)->pkt_sn;//�ȴ�cfm����������
							SetTimer(&proc->tx_req_timer,3);//��ʱ��ʱ�䣿������
							proc->state = WAIT_TX_CFM;
						}
						return 0;
					}
				}
				proc->state = IDLE;
				return 0;
			}
			else  //����ʧ�ܴ���δ����3��,���²���·��,����ʧ�ܵ��սڵ㷴����·��ģ�飬���ڲ�������·��
			{
				proc->fail_ra[proc->retry_cnt-1] = proc->ra;
				sig = proc->data_ntx_rt_req;
				((T_Data_Ntx_Rt_Req_Param*)sig->param)->da = proc->da;
				memcpy(((T_Data_Ntx_Rt_Req_Param*)sig->param)->fail_ra, proc->fail_ra, sizeof(proc->fail_ra));
				AddSignal(sig);
				proc->state = WAIT_RT_CFM;
			}
		}
	}
	return 0;
}

static int DataTfInd(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	//Data_Interface_Tx_Req_Param *param = (Data_Interface_Tx_Req_Param*)sig->param;
	unsigned short btm,top,size;
	unsigned short ttl;
	btm = proc->net_que_list.btm;
	size = proc->net_que_list.size;
	top = proc->net_que_list.top;
	//�����е�ÿһ���������TTL��Ҫ������
	while (size != 0)
	{
		ttl = proc->net_que_list.que_elmt[btm].TTL;
		if (ttl != 0)
		{
			proc->net_que_list.que_elmt[btm].TTL--;
		}
		btm++;
		btm %= NET_QUE_MAX_NUM;
		size--;
	}
	return 0;
}

static int TxReqTimerOut(Timer *tmr)
{
	Data_Ntx *proc = (Data_Ntx*)tmr->dst;

	return 0;
}

static int RtFindTimerOut(Timer *tmr)
{
	Data_Ntx *proc = (Data_Ntx*)tmr->dst;
	Signal *sig;
	//����·��̽���������ȡ��һ��ra
	//??������Щ����,��SDLͼ����������
	if (proc->ra != 0xff)  //�ҵ�
	{
		//����ra����
		sig = proc->dlc_txpump_tx_req;
		((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt	= (unsigned short*)proc->net_pkt;
		((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len		= proc->len;
		((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra			= proc->ra;
		((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri		= proc->pri;
		AddSignal(sig);
		SetTimer(&proc->tx_req_timer,3);//��ʱ��ʱ�䣿������
		proc->state = WAIT_TX_CFM;
	}
	else
	{
		sig = proc->data_interface_tx_cfm;
		((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_flag = FAIL;
		AddSignal(sig);
		proc->state = IDLE;
	}

	return 0;
}

void DataNtxInit(Data_Ntx* proc)
{
	proc->state = IDLE;
	proc->pkt_sn = 1;
	proc->net_pkt = NULL;
	proc->ra = 0;
	proc->retry_cnt = 0;
	proc->pri = 0;
	proc->len = 0;
	proc->da = 0;
	memset(proc->fail_ra, 0, sizeof(proc->fail_ra));
}

void DataNtxSetup(Data_Ntx* proc)
{
	Signal *sig;
	Timer *tmr;

	sig = &proc->data_interface_tx_req;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataInterfaceTxReq;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->data_interface_tx_req_param;

	sig = &proc->data_rly_tx_req;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataRlyTxReq;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->data_rly_tx_req_param;

	sig = &proc->data_ntx_rt_cfm;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataNtxRtCfm;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->data_ntx_rt_cfm_param;

	sig = &proc->data_ntx_rt_find_cfm;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataNtxRtFindCfm;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->data_ntx_rt_find_cfm_param;

	sig = &proc->dlc_txpump_tx_cfm;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DlcTxpumpTxCfm;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->dlc_txpump_tx_cfm_param;

	sig = &proc->data_tf_ind;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataTfInd;
	sig->pri=SDL_PRI_NORM;
	sig->param = 0;

	tmr = &proc->tx_req_timer;
	tmr->next=0;
	tmr->dst=proc;
	tmr->pri=SDL_PRI_NORM;
	tmr->func=TxReqTimerOut;

	tmr = &proc->rt_find_timer;
	tmr->next=0;
	tmr->dst=proc;
	tmr->pri=SDL_PRI_NORM;
	tmr->func=RtFindTimerOut;
}