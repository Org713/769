#include "mib.h"
enum {SLOT_IDLE,RSVD,RSV_TX,RSV_RX,TX,RX,NO_TX,NO_PRAM,NAV};
void MibInit(Mib* mib)
{
	u8 i,j;
	//mib->local_id = 0;
	mib->RND = 0;
	mib->mac_rx_ind_flag = 0;
	for (i = 0; i < TS_NUM; i++)
	{
		if (i == 0 || i == 99)//0.99被预约
		{
			mib->slot_table.ts_slot_table[i].ts_state = RSVD;
		}
		else if(i == 2 || i == 3 || i == 4 || i == 5) //不能PRAM
		{
			mib->slot_table.ts_slot_table[i].ts_state = NO_PRAM;
		}
		else
		{
			mib->slot_table.ts_slot_table[i].ts_state = SLOT_IDLE;
		}
		for (j = 0; j < UTS_NUM; j++)
		{
			mib->slot_table.ts_slot_table[i].uts[j].opr_type = SLOT_IDLE;
		}
	}
}
