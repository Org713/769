#include "data_interface_tx.h"
#include <string.h>
enum{IDLE,WAIT_TX_CFM};
enum{NO_REACH,SEND_SUCCESS,NET_BUSY,WAIT_SEND,SENT};

unsigned short wait_cfm_pkt_cnt = 0;//Ҫ�ȴ�cfm�����
unsigned short receive_cfm_pkt_cnt = 0;//���յ���cfm�����

//д���� 2016.12.27 ���������ȼ�û��ϵ��ֱ�ӷ��ڶ��еĺ���
int DataQueWt(Data_Interface_Tx* proc,unsigned short svc_type,unsigned short pkt_sn,unsigned short pri,unsigned short da,unsigned short len, unsigned short *data)
{
	unsigned short btm,top,size;
	top = proc->data_que_list.top;
	btm = proc->data_que_list.btm;
	size = proc->data_que_list.size; 
	if(size >= DATA_QUE_MAX_NUM)  //20
	{
		printf("data_que_list() ������\n");
		return 0;
	}
	else
	{
		proc->data_que_list.data_que_elmt[top].svc_type = svc_type;
		proc->data_que_list.data_que_elmt[top].pkt_sn = pkt_sn;
		proc->data_que_list.data_que_elmt[top].pri = pri;
		proc->data_que_list.data_que_elmt[top].da = da;
		proc->data_que_list.data_que_elmt[top].len = len;
		memcpy(proc->data_que_list.data_que_elmt[top].data, data, len);

		proc->data_que_list.top++;
		proc->data_que_list.top %= DATA_QUE_MAX_NUM;
		proc->data_que_list.size++;
		return 1;
	}	
}

//������
int DataQueRd(Data_Interface_Tx* proc,unsigned short **data,unsigned short *len,unsigned short *da,unsigned short *pri,unsigned short *svc_type, unsigned short *pkt_sn)
{	
	unsigned short btm;
	if (proc->data_que_list.size != 0)
	{
		btm = proc->data_que_list.btm;

		*data	  = proc->data_que_list.data_que_elmt[btm].data;
		*len	  = proc->data_que_list.data_que_elmt[btm].len;
		*da		  = proc->data_que_list.data_que_elmt[btm].da;
		*pri	  = proc->data_que_list.data_que_elmt[btm].pri;
		*svc_type = proc->data_que_list.data_que_elmt[btm].svc_type;
		*pkt_sn	  = proc->data_que_list.data_que_elmt[btm].pkt_sn;
		return 1;
	}
	return 0;
}
//ɾ�������е�Ԫ��  
void DeleteDataQueElmt(Data_Interface_Tx* proc)
{
	unsigned short btm=proc->data_que_list.btm;
	if(proc->data_que_list.size)
	{
		printf("DeleteDataQueElmt() ɾ�����������ݰ�\n");
		memset(proc->data_que_list.data_que_elmt[btm].data, 0, sizeof(proc->data_que_list.data_que_elmt[btm].data));
		//proc->data_que_list.data_que_elmt[btm].data = NULL;
		proc->data_que_list.data_que_elmt[btm].len = 0;
		proc->data_que_list.data_que_elmt[btm].svc_type = 0;
		proc->data_que_list.data_que_elmt[btm].da = 0;
		proc->data_que_list.data_que_elmt[btm].pkt_sn = 0;
		proc->data_que_list.data_que_elmt[btm].pri = 0;

		proc->data_que_list.btm ++;
		proc->data_que_list.btm %= DATA_QUE_MAX_NUM;
		proc->data_que_list.size --;
	}

}

static int SvcDataTxReq(Signal *sig)
{
	Data_Interface_Tx *proc = (Data_Interface_Tx*)sig->dst;
	Svc_Data_Tx_Req_Param *param = (Svc_Data_Tx_Req_Param*)sig->param;

	unsigned short svc_type;
	unsigned short pkt_sn;
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	unsigned short *data;//���ݵ�ַ
	svc_type = param->svc_type;
	pkt_sn = param->pkt_sn;
	pri = param->pri;
	da = param->da;
	len = param->len;
	printf("Data_Interface_Tx[%d]::SvcDataTxReq()�յ���������,\n",proc->data_pri_id);
	//�����ǲ���Ӧ����д���У����ж�IDLE������2016.12.27
	if (proc->state == IDLE)
	{
		DataQueWt(proc, svc_type, pkt_sn, pri, da, len, param->data);//д����

		DataQueRd(proc,&data,&len,&da,&pri,&svc_type,&pkt_sn);//������

		proc->pri = pri;//����Ҫ���͵����������������cfmʱʹ��
		proc->pkt_sn = pkt_sn;
		proc->data = data;
		proc->da = da;
		proc->len = len;
		proc->svc_type = svc_type;
		//�Ƿ��ǵ���,����Ŀ�ĵ�ַ��֪
		if ( (MAX_NODE_CNT > da) && (da >= 0) )	//����
		{
			sig = proc->data_interface_tx_req[da];
			((T_Data_Interface_Tx_Req_Param*)sig->param)->data = proc->data;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->len = proc->len;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->da = proc->da;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->pri = proc->pri;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->svc_type = proc->svc_type;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->pkt_sn = proc->pkt_sn;

			printf("Data_Interface_Tx[%d]::SvcDataTxReq() ���������͸�Ŀ��data_ntx[%d],\n",proc->data_pri_id, proc->da);
			AddSignal(sig);

		}
		else  //���ǵ�����
		{
			sig = proc->data_interface_tx_req[proc->mib->local_id];
			((T_Data_Interface_Tx_Req_Param*)sig->param)->data = proc->data;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->len = proc->len;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->da = proc->da;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->pri = proc->pri;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->svc_type = proc->svc_type;
			printf("Data_Interface_Tx[%d]::SvcDataTxReq() �ǵ��������͸�Ŀ��data_ntx[%d],\n",proc->data_pri_id, proc->mib->local_id);
			AddSignal(sig);
		}
		SetTimer(&proc->tx_cfm_tmr, 3);//��ʱʱ�䣿����
		wait_cfm_pkt_cnt++;
		proc->state = WAIT_TX_CFM;
	}
	else
	{
		sig = proc->svc_data_tx_cfm;
		((T_Svc_Data_Tx_Cfm_Param*)sig->param)->cfm_flag = NET_BUSY;
		((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pri = param->pri;
		((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pkt_sn = param->pkt_sn;
		printf("Data_Interface_Tx[%d]::SvcDataTxReq() �ظ��ϲ�����æ\n",proc->data_pri_id);
		AddSignal(sig);
	}
	return 0;
}

static int DataInterfaceTxCfm(Signal *sig)
{
	Data_Interface_Tx *proc = (Data_Interface_Tx*)sig->dst;
	Data_Interface_Tx_Cfm_Param *param = (Data_Interface_Tx_Cfm_Param*)sig->param;

	unsigned short svc_type;
	unsigned short pkt_sn;
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	unsigned short *data;//����
	printf("Data_Interface_Tx[%d]::DataInterfaceTxCfm()�յ�cfm,\n",proc->data_pri_id);
	if (proc->state == WAIT_TX_CFM)
	{
		receive_cfm_pkt_cnt++;
		if (param->cfm_flag == SEND_SUCCESS)
		{
			//if (wait_cfm_pkt_cnt == receive_cfm_pkt_cnt)   //Ϊ�ȴ�cfm�����
			if (param->cfm_pkt_sn == proc->pkt_sn)
			{
				CancelTimer(&proc->tx_cfm_tmr);
				//ɾ�������ݰ�
				DeleteDataQueElmt(proc);

				sig = proc->svc_data_tx_cfm;
				((T_Svc_Data_Tx_Cfm_Param*)sig->param)->cfm_flag = param->cfm_flag;
				((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pri = proc->pri;
				((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pkt_sn = proc->pkt_sn;
				AddSignal(sig);
				//�Ƿ���δ��������
				if (proc->data_que_list.size)//����Ҫ���͵�����
				{
					DataQueRd(proc,&data,&len,&da,&pri,&svc_type, &pkt_sn);//������

					proc->pri = pri;//����Ҫ���͵����������������cfmʱʹ��
					proc->pkt_sn = pkt_sn;
					proc->data = data;
					proc->da = da;
					proc->len = len;
					proc->svc_type = svc_type;
					//�Ƿ��ǵ���,����Ŀ�ĵ�ַ��֪
					if (MAX_NODE_CNT>da && da>=0)	//����
					{
						sig = proc->data_interface_tx_req[da];
						((T_Data_Interface_Tx_Req_Param*)sig->param)->data = proc->data;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->len = proc->len;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->da = proc->da;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->pri = proc->pri;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->svc_type = proc->svc_type;
						printf("Data_Interface_Tx[%d]::DataInterfaceTxCfm(),�յ�cfm���ٷ��͸�Ŀ��data_ntx[%d],\n",proc->data_pri_id, proc->da);
						AddSignal(sig);

					}
					else  //���ǵ�����
					{
						sig = proc->data_interface_tx_req[proc->mib->local_id];
						((T_Data_Interface_Tx_Req_Param*)sig->param)->data = proc->data;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->len = proc->len;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->da = proc->da;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->pri = proc->pri;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->svc_type = proc->svc_type;
						AddSignal(sig);
					}
					SetTimer(&proc->tx_cfm_tmr, 3);//��ʱʱ�䣿����
					receive_cfm_pkt_cnt++;
					//proc->state = WAIT_TX_CFM;
				}
				else  //û��Ҫ���͵�����
				{
					proc->state = IDLE;
				}
			}
			else   //���ǵȴ���cfm��
			{

			}
		}
		else if (param->cfm_flag == NO_REACH)
		{

		}
		else if (param->cfm_flag == NET_BUSY)
		{

		}
	}

	return 0;
}



static int TxCfmTmrOut(Timer *tmr)
{
	Data_Interface_Tx *proc = (Data_Interface_Tx*)tmr->dst;
	Signal *sig;
	sig = proc->svc_data_tx_cfm;
	((T_Svc_Data_Tx_Cfm_Param*)sig->param)->cfm_flag = NO_REACH;
	((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pri = proc->pri;
	((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pkt_sn = proc->pkt_sn;
	AddSignal(sig);
	proc->state = IDLE;
	return 0;
}

void DataInterfaceTxInit(Data_Interface_Tx* proc)
{
	proc->state = IDLE;
	proc->da = 0;
	proc->pri = 0;
	proc->pkt_sn = 0;
	proc->data = NULL;
	proc->data_que_list.btm = 0;
	proc->data_que_list.top = 0;
	proc->data_que_list.size = 0;
	memset(proc->data_que_list.data_que_elmt, 0, sizeof(proc->data_que_list.data_que_elmt));
	
}

void DataInterfaceTxSetup(Data_Interface_Tx* proc)
{
	//unsigned short i;
	Timer *tmr;
	proc->svc_data_tx_req.next=0;
	proc->svc_data_tx_req.src=0;
	proc->svc_data_tx_req.dst=proc;
	proc->svc_data_tx_req.func=SvcDataTxReq;
	proc->svc_data_tx_req.pri=SDL_PRI_NORM;
	proc->svc_data_tx_req.param=&proc->svc_data_tx_req_param;
	
	proc->data_interface_tx_cfm.next=0;
	proc->data_interface_tx_cfm.src=0;
	proc->data_interface_tx_cfm.dst=proc;
	proc->data_interface_tx_cfm.func=DataInterfaceTxCfm;
	proc->data_interface_tx_cfm.pri=SDL_PRI_NORM;
	proc->data_interface_tx_cfm.param=&proc->data_interface_tx_cfm_param;
	
	
	tmr=&proc->tx_cfm_tmr;
	tmr->next=0;
	tmr->dst=proc;
	tmr->pri=SDL_PRI_NORM;
	tmr->func=TxCfmTmrOut;
}
