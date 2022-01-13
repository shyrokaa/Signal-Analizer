/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  FreqPanel                        1       /* callback function: OnFrequencyPanel */
#define  FreqPanel_GRAPH_2                2       /* control type: graph, callback function: (none) */
#define  FreqPanel_GRAPH                  3       /* control type: graph, callback function: (none) */
#define  FreqPanel_SWITCHPANEL            4       /* control type: binary, callback function: OnSwitchPanelCB */
#define  FreqPanel_NSIZE                  5       /* control type: numeric, callback function: (none) */
#define  FreqPanel_MODESWITCH             6       /* control type: binary, callback function: (none) */
#define  FreqPanel_COMMANDBUTTON_3        7       /* control type: command, callback function: OnExpBtt */
#define  FreqPanel_COMMANDBUTTON          8       /* control type: command, callback function: OnGenerateBtt */
#define  FreqPanel_TIMER                  9       /* control type: timer, callback function: OnTimer */
#define  FreqPanel_SAMPLER                10      /* control type: numeric, callback function: (none) */
#define  FreqPanel_NP                     11      /* control type: numeric, callback function: (none) */
#define  FreqPanel_IS                     12      /* control type: numeric, callback function: (none) */
#define  FreqPanel_CT                     13      /* control type: numeric, callback function: (none) */
#define  FreqPanel_PP                     14      /* control type: numeric, callback function: (none) */
#define  FreqPanel_FP                     15      /* control type: numeric, callback function: (none) */
#define  FreqPanel_FE                     16      /* control type: textButton, callback function: (none) */
#define  FreqPanel_RFQ                    17      /* control type: numeric, callback function: (none) */
#define  FreqPanel_RPL                    18      /* control type: numeric, callback function: (none) */
#define  FreqPanel_RING                   19      /* control type: slide, callback function: OnWType */
#define  FreqPanel_WINDOWTYPE             20      /* control type: slide, callback function: OnWType */

#define  WavePanel                        2       /* callback function: OnMainPanel */
#define  WavePanel_DECORATION_3           2       /* control type: deco, callback function: (none) */
#define  WavePanel_DECORATION             3       /* control type: deco, callback function: (none) */
#define  WavePanel_PLOTBTT                4       /* control type: command, callback function: SwitchTime */
#define  WavePanel_NEXTBTT                5       /* control type: command, callback function: SwitchTime */
#define  WavePanel_PREVBTT                6       /* control type: command, callback function: SwitchTime */
#define  WavePanel_RING                   7       /* control type: ring, callback function: OnRing */
#define  WavePanel_APPLYBTT               8       /* control type: command, callback function: OnApplyBtt */
#define  WavePanel_NUMERIC_3              9       /* control type: numeric, callback function: (none) */
#define  WavePanel_ALPHA_VAL              10      /* control type: numeric, callback function: (none) */
#define  WavePanel_NUMERIC_2              11      /* control type: numeric, callback function: (none) */
#define  WavePanel_NUMERIC_5              12      /* control type: numeric, callback function: (none) */
#define  WavePanel_NUMERIC_4              13      /* control type: numeric, callback function: (none) */
#define  WavePanel_NUMERIC_8              14      /* control type: numeric, callback function: (none) */
#define  WavePanel_NUMERIC_7              15      /* control type: numeric, callback function: (none) */
#define  WavePanel_NUMERIC_9              16      /* control type: numeric, callback function: (none) */
#define  WavePanel_NUMERIC_10             17      /* control type: numeric, callback function: (none) */
#define  WavePanel_NUMERIC_6              18      /* control type: numeric, callback function: (none) */
#define  WavePanel_NUMERIC                19      /* control type: numeric, callback function: (none) */
#define  WavePanel_ENVBTT                 20      /* control type: command, callback function: OnEnv */
#define  WavePanel_LOADBTT                21      /* control type: command, callback function: Onfileloader */
#define  WavePanel_IDC_GRAPH_DISPLAY2     22      /* control type: graph, callback function: (none) */
#define  WavePanel_HIST_DISPLAY           23      /* control type: graph, callback function: (none) */
#define  WavePanel_IDC_GRAPH_DISPLAY1     24      /* control type: graph, callback function: (none) */
#define  WavePanel_DECORATION_2           25      /* control type: deco, callback function: (none) */
#define  WavePanel_STOP                   26      /* control type: numeric, callback function: (none) */
#define  WavePanel_START                  27      /* control type: numeric, callback function: (none) */
#define  WavePanel_ALPHA                  28      /* control type: scale, callback function: OnAlpha */
#define  WavePanel_SWITCHPANEL            29      /* control type: binary, callback function: OnSwitchPanelCB */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK OnAlpha(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnApplyBtt(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnEnv(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnExpBtt(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Onfileloader(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFrequencyPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnGenerateBtt(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMainPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnRing(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSwitchPanelCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnWType(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SwitchTime(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif