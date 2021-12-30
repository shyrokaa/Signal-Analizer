#include "toolbox.h"
#include <math.h>
#include <ansi_c.h>
#include <formatio.h>
#include <utility.h>
#include <analysis.h>
#include <userint.h>
#include "ui.h"


int CVICALLBACK OnFrequencyPanel (int panel, int event, void *callbackData,
									  int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			QuitUserInterface(0);
			break;
	}
	return 0;
}




