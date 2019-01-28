#include "SysDefines.h"


void PublicEvent(PUBLIC_EVENT Event,u32 Param,void *p)
{
	//¿¼ÂÇÒÀÀµÐÔ
	TrigManEventInput(Event,Param,p);
	NotifyList_EventInput(Event,Param,p);
	VarManEventInput(Event,Param,p);
	//WDevManEventInput(Event,Param,p);
	AppClientEventInput(Event,Param,p);
}



