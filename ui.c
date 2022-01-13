#include "toolbox.h"
#include <math.h>
#include <ansi_c.h>
#include <formatio.h>
#include <utility.h>
#include <analysis.h>
#include <userint.h>
#include "ui.h"
#define SAMPLE_RATE		0
#define NPOINTS			1

//the 2 panels used in the project
int wavePanel = 0;
int freqPanel = 0;

//signal information
double sampleRate = 0.0;
int npoints = 0;

// int panelHandle;
static double *rowSignal;
double waveInfo[2];
int minIndex,maxIndex;
double minVal,maxVal,mean;
int z_cross;
double sum_1;
double sum_2;
double dispersion;

//filters + envelope
static double *env_rowSignal;
static double *filtered_rowSignal;

static int filterType = 0;
static double alpha;
static	int index = 0;

//time and histoagram

//for split sections of the signal

ssize_t hist_array[20];
double x_array[20];
int y_array[20] = {0};

//for the whole signal

ssize_t hist_array_full[100];
double x_array_full[100];
int y_array_full[100] = {0};

//stored data
int select_multiplyer;
BOOL init_split = TRUE;
static double *rowSignal_split;
static double *filtered_rowSignal_split;

//values for the signal power spectrum

//window type used in the generation of the spectrum
//is picked using the ring control
int window = 3;

int mode;
int numRead;
int splitindex = 0;
int timer = 0;
int spectrumFilterType = 0;


int main (int argc, char *argv[])
{
	int error = 0;
	
	/* initialize and load resources */
	nullChk (InitCVIRTE (0, argv, 0));
	errChk (wavePanel = LoadPanel (0, "ui.uir", WavePanel));
	errChk (freqPanel = LoadPanel (0, "ui.uir", FreqPanel));
	
	/* display the panel and run the user interface */
	errChk (DisplayPanel (wavePanel));
	errChk (RunUserInterface ());

Error:

	if (wavePanel > 0)
		DiscardPanel (wavePanel);
	return 0;
}



int CVICALLBACK OnMainPanel (int panel, int event, void *callbackData,
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

int CVICALLBACK Onfileloader (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			//python script calls and signal to vector conversion
			
			system ("python FileProcessing.py"); 
			FileToArray("waveInfo.txt", waveInfo, VAL_DOUBLE, 2, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS, VAL_ASCII);
			sampleRate = waveInfo[SAMPLE_RATE];
			npoints = waveInfo[NPOINTS];
			rowSignal = (double *) calloc(npoints, sizeof(double));
			FileToArray("waveData.txt",rowSignal, VAL_DOUBLE, npoints, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS, VAL_ASCII);
			
			//signal information 
			MaxMin1D(rowSignal,npoints,&maxVal,&maxIndex,&minVal,&minIndex);

			//histogram of the whole signal -  a bit messy on display
			
			for(int i = 0; i < 100; i++)
			{
				hist_array_full[i] = 0;
				y_array_full[i] = 0;
				x_array_full[i] = 0;
			}				
			Histogram (rowSignal, npoints, minVal, maxVal, hist_array_full, x_array_full, 100);
			
			for (int i=0;i<100;i++)
            		y_array_full[i] +=(int)hist_array_full[i];
			
			
			//zero crossing - simplified, no functions whatsoever
			z_cross = 0;
			for(int i = 0 ; i < npoints-1; i++)
			{
				if((rowSignal[i] > 0 && rowSignal[i+1] < 0) || (rowSignal[i] < 0 && rowSignal[i+1] > 0))
					z_cross++;		
				
			}
			
			//dispersion - some math involved 
			
			sum_1 = 0;
			sum_2 = 0;
			
			for(int i = 0 ; i < npoints; i++)
			{
				sum_1 += rowSignal[i];
				sum_2 += rowSignal[i] * rowSignal[i]; 
			}
			mean = sum_1 / npoints;
			dispersion = sum_2 / npoints - mean * mean;
			
			//plots and sets
			
			SetCtrlVal(panel,WavePanel_NUMERIC,minVal);
			SetCtrlVal(panel,WavePanel_NUMERIC_2,mean);
			SetCtrlVal(panel,WavePanel_NUMERIC_3,maxVal);
			SetCtrlVal(panel,WavePanel_NUMERIC_4,(double)minIndex);
			SetCtrlVal(panel,WavePanel_NUMERIC_5,(double)maxIndex);
			SetCtrlVal(panel,WavePanel_NUMERIC_6,sampleRate);
			SetCtrlVal(panel,WavePanel_NUMERIC_7,npoints);
			SetCtrlVal(panel,WavePanel_NUMERIC_9,z_cross);
			SetCtrlVal(panel,WavePanel_NUMERIC_10,dispersion);
		
			PlotY(panel,WavePanel_IDC_GRAPH_DISPLAY1,rowSignal,npoints,VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_BLACK);
			PlotXY (panel, WavePanel_HIST_DISPLAY, x_array_full, y_array_full, 100, VAL_DOUBLE, VAL_INTEGER, VAL_VERTICAL_BAR, VAL_EMPTY_SQUARE, VAL_SOLID, 1,VAL_BLACK);
			
			
			//activating the buttons for more controls
			
			SetCtrlAttribute(panel,WavePanel_APPLYBTT,ATTR_DIMMED,0);
			SetCtrlAttribute(panel,WavePanel_ENVBTT,ATTR_DIMMED,0);
			
			
			break;
	}
	return 0;
}

int CVICALLBACK OnEnv (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			//i don't need to reset the time controls here because the filtered signal has not been reloaded
			
			//some cleanup and some python script calls
			DeleteGraphPlot(panel,WavePanel_IDC_GRAPH_DISPLAY2,-1,VAL_IMMEDIATE_DRAW);
			
			system ("python EnvelopeSignal.py"); 		
			env_rowSignal = (double *) calloc(npoints, sizeof(double));		
			FileToArray("waveEnv.txt",env_rowSignal,VAL_DOUBLE,npoints,1,VAL_GROUPS_TOGETHER,VAL_GROUPS_AS_COLUMNS,VAL_ASCII);
			
			PlotY(panel,WavePanel_IDC_GRAPH_DISPLAY2,env_rowSignal,npoints,VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_BLUE);
			
			break;
	}
	return 0;
}


int CVICALLBACK OnRing (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	//it would be more practic to call the getctrlval in other functions but im lazy to this is what happens
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel,WavePanel_RING,&filterType);
			break;
	}
	return 0;
}



int CVICALLBACK OnApplyBtt (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			//enabling the additional plot options + resetting some values for functionality
			init_split = TRUE;
			SetCtrlAttribute(panel,WavePanel_PLOTBTT,ATTR_DIMMED,0);
			SetCtrlAttribute(panel,WavePanel_PREVBTT,ATTR_DIMMED,1);
			SetCtrlAttribute(panel,WavePanel_NEXTBTT,ATTR_DIMMED,1);
			SetCtrlAttribute(panel,WavePanel_START,ATTR_DIMMED,0);
			SetCtrlAttribute(panel,WavePanel_STOP,ATTR_DIMMED,0);
			
			switch(filterType)
			{
			case 0:
				//mediation
				filtered_rowSignal = (double *) calloc(npoints, sizeof(double));
				
				DeleteGraphPlot(panel,WavePanel_IDC_GRAPH_DISPLAY2,-1,VAL_IMMEDIATE_DRAW);
				
				MedianFilter(rowSignal,npoints,16,0,filtered_rowSignal);
				PlotY(panel,WavePanel_IDC_GRAPH_DISPLAY2,filtered_rowSignal,npoints,VAL_DOUBLE, VAL_THIN_LINE, VAL_ASTERISK, VAL_SOLID, 1, VAL_RED);
				
				break;
			case 1:
				//1st order
				filtered_rowSignal = (double *) calloc(npoints, sizeof(double));
				
				for(int i = 0 ; i < npoints ; i++)
				{
				filtered_rowSignal[i] = 0;	
				}
				for(int i = 1 ; i < npoints; i++)
				{
				filtered_rowSignal[i] = (1-alpha) * filtered_rowSignal[i-1] + alpha * rowSignal[i];
				}
				
				DeleteGraphPlot(panel,WavePanel_IDC_GRAPH_DISPLAY2,-1,VAL_IMMEDIATE_DRAW);
				PlotY(panel,WavePanel_IDC_GRAPH_DISPLAY2,filtered_rowSignal,npoints,VAL_DOUBLE, VAL_THIN_LINE, VAL_ASTERISK, VAL_SOLID, 1, VAL_RED);
				
				break;
					
			}
			break;
	}
	return 0;
}

int CVICALLBACK OnAlpha (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel,WavePanel_ALPHA,&alpha);
			SetCtrlVal(panel,WavePanel_ALPHA_VAL,alpha);
			break;
	}
	return 0;
}

//most unnecesarly complex function in this file probably
int CVICALLBACK SwitchTime (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			switch(control)
			{
					
			case WavePanel_NEXTBTT:
				
				if(index + select_multiplyer < npoints)
				{
					index += select_multiplyer;
					z_cross = 0;
					for(int i =  index ; i < select_multiplyer + index ; i++)
					{
						rowSignal_split[i - index] = rowSignal[i];
						filtered_rowSignal_split[i - index] = filtered_rowSignal[i];
					}
					
					for(int i = 0 ; i < select_multiplyer - 1; i++)
						if((rowSignal_split[i] > 0 && rowSignal_split[i+1] < 0) || (rowSignal_split[i] < 0 && rowSignal_split[i+1] > 0))
							z_cross++;	
					
					
					//local max and min for the next period
					MaxMin1D(rowSignal_split,select_multiplyer,&maxVal,&maxIndex,&minVal,&minIndex);
					
					
					sum_1 = 0;
					sum_2 = 0;
					
					for(int i = 0 ; i < select_multiplyer; i++)
					{
						sum_1 += filtered_rowSignal[i];
						sum_2 += filtered_rowSignal[i] * filtered_rowSignal[i]; 
					}
					mean = sum_1 / select_multiplyer;
					dispersion = sum_2 / select_multiplyer - mean * mean;
					
					//just a safety cleanup of the histogram vectors
					for(int i = 0; i < 20; i++)
					{
						hist_array[i] = 0;
						y_array[i] = 0;
						x_array[i] = 0;
					
					}
					
					Histogram (rowSignal_split, select_multiplyer, minVal, maxVal, hist_array, x_array, 20);
				 	
					for (int i=0;i<20;i++)
	            		y_array[i] +=(int)hist_array[i];
					
					SetCtrlVal(panel,WavePanel_NUMERIC,minVal);
					SetCtrlVal(panel,WavePanel_NUMERIC_2,mean);
					SetCtrlVal(panel,WavePanel_NUMERIC_3,maxVal);
					SetCtrlVal(panel,WavePanel_NUMERIC_4,(double)minIndex);
					SetCtrlVal(panel,WavePanel_NUMERIC_5,(double)maxIndex);
					SetCtrlVal(panel,WavePanel_NUMERIC_7,select_multiplyer);
					SetCtrlVal(panel,WavePanel_NUMERIC_8,(int)(index/sampleRate));
					SetCtrlVal(panel,WavePanel_NUMERIC_9,z_cross);
					SetCtrlVal(panel,WavePanel_NUMERIC_10,dispersion);
					
					
					//initial plots and removal of previous charts
					DeleteGraphPlot(panel,WavePanel_IDC_GRAPH_DISPLAY1,-1,VAL_IMMEDIATE_DRAW);
					DeleteGraphPlot(panel,WavePanel_IDC_GRAPH_DISPLAY2,-1,VAL_IMMEDIATE_DRAW);
					DeleteGraphPlot(panel, WavePanel_HIST_DISPLAY,-1,VAL_IMMEDIATE_DRAW);
					
					PlotY(panel,WavePanel_IDC_GRAPH_DISPLAY1,rowSignal_split,select_multiplyer,VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_BLACK);
					PlotY(panel,WavePanel_IDC_GRAPH_DISPLAY2,filtered_rowSignal_split,select_multiplyer,VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
					PlotXY (panel, WavePanel_HIST_DISPLAY, x_array, y_array, 20, VAL_DOUBLE, VAL_INTEGER, VAL_VERTICAL_BAR, VAL_EMPTY_SQUARE, VAL_SOLID, 1,VAL_BLACK);
				}
				break;
			
			case WavePanel_PREVBTT:
				
				if(index - select_multiplyer >= 0)
				{
					index -= select_multiplyer;
					z_cross = 0;
					for(int i =  index ; i < select_multiplyer + index;i++)
					{
						rowSignal_split[i - index ] = rowSignal[i];
						filtered_rowSignal_split[i - index] = filtered_rowSignal[i];
					}
					
					for(int i = 0 ; i < select_multiplyer - 1; i++)
						if((rowSignal_split[i] > 0 && rowSignal_split[i+1] < 0) || (rowSignal_split[i] < 0 && rowSignal_split[i+1] > 0))
							z_cross++;	
					
					//local max and min for the previous period
						
					MaxMin1D(rowSignal_split,select_multiplyer,&maxVal,&maxIndex,&minVal,&minIndex);
		
					sum_1 = 0;
					sum_2 = 0;
					
					for(int i = 0 ; i < select_multiplyer; i++)
					{
						sum_1 += filtered_rowSignal[i];
						sum_2 += filtered_rowSignal[i] * filtered_rowSignal[i]; 
					}
					mean = sum_1 / select_multiplyer;
					dispersion = sum_2 / select_multiplyer - mean * mean;
					
					
					//just a safety cleanup of the histogram vectors
					for(int i = 0; i < 20; i++)
					{
						hist_array[i] = 0;
						y_array[i] = 0;
						x_array[i] = 0;
					
					}
					
					Histogram (rowSignal_split, select_multiplyer, minVal, maxVal, hist_array, x_array, 20);
				 	
					for (int i=0;i<20;i++)
	            		y_array[i] +=(int)hist_array[i];
					
					SetCtrlVal(panel,WavePanel_NUMERIC_8,(int)(index/sampleRate));
					SetCtrlVal(panel,WavePanel_NUMERIC,minVal);
					SetCtrlVal(panel,WavePanel_NUMERIC_2,mean);
					SetCtrlVal(panel,WavePanel_NUMERIC_3,maxVal);
					SetCtrlVal(panel,WavePanel_NUMERIC_4,(double)minIndex);
					SetCtrlVal(panel,WavePanel_NUMERIC_5,(double)maxIndex);
					SetCtrlVal(panel,WavePanel_NUMERIC_7,select_multiplyer);
					SetCtrlVal(panel,WavePanel_NUMERIC_8,(int)(index/sampleRate));
					SetCtrlVal(panel,WavePanel_NUMERIC_9,z_cross);
					SetCtrlVal(panel,WavePanel_NUMERIC_10,dispersion);
					
					//initial plots and removal of previous charts
					DeleteGraphPlot(panel,WavePanel_IDC_GRAPH_DISPLAY1,-1,VAL_IMMEDIATE_DRAW);
					DeleteGraphPlot(panel,WavePanel_IDC_GRAPH_DISPLAY2,-1,VAL_IMMEDIATE_DRAW);
					DeleteGraphPlot(panel, WavePanel_HIST_DISPLAY,-1,VAL_IMMEDIATE_DRAW);
					
					PlotY(panel,WavePanel_IDC_GRAPH_DISPLAY1,rowSignal_split,select_multiplyer,VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_BLACK);
					PlotY(panel,WavePanel_IDC_GRAPH_DISPLAY2,filtered_rowSignal_split,select_multiplyer,VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
					PlotXY (panel, WavePanel_HIST_DISPLAY, x_array, y_array, 20, VAL_DOUBLE, VAL_INTEGER, VAL_VERTICAL_BAR, VAL_EMPTY_SQUARE, VAL_SOLID, 1,VAL_BLACK);
				}
				break;
				
			case WavePanel_PLOTBTT:
				//this button should do nothing after it was pressed once,it's here only for data initialization
				if(init_split)
				{
					int start,stop;
					GetCtrlVal(panel,WavePanel_START,&start);
					GetCtrlVal(panel,WavePanel_STOP,&stop);
					
					select_multiplyer = (stop - start) * sampleRate;
					
					rowSignal_split = (double *) calloc(select_multiplyer, sizeof(double));
					filtered_rowSignal_split = (double *) calloc(select_multiplyer, sizeof(double));
					
					index = 0;
					z_cross = 0;
					for(int i = 0 ; i < select_multiplyer; i++)
					{
						rowSignal_split[i] = rowSignal[i];
						filtered_rowSignal_split[i] = filtered_rowSignal[i];
					}
					
					
					for(int i = 0 ; i < select_multiplyer - 1; i++)
						if((rowSignal_split[i] > 0 && rowSignal_split[i+1] < 0) || (rowSignal_split[i] < 0 && rowSignal_split[i+1] > 0))
							z_cross++;	
					
					//local max and min for the 0-1 period
					
					MaxMin1D(rowSignal_split,select_multiplyer,&maxVal,&maxIndex,&minVal,&minIndex);
					
					
					sum_1 = 0;
					sum_2 = 0;
					
					for(int i = 0 ; i < select_multiplyer; i++)
					{
						sum_1 += filtered_rowSignal[i];
						sum_2 += filtered_rowSignal[i] * filtered_rowSignal[i]; 
					}
					mean = sum_1 / select_multiplyer;
					dispersion = sum_2 / select_multiplyer - mean * mean;
					
					//just a safety cleanup of the histogram vectors
					
					for(int i = 0; i < 20; i++)
					{
						hist_array[i] = 0;
						y_array[i] = 0;
						x_array[i] = 0;
					}
						
					Histogram (rowSignal_split,select_multiplyer, minVal, maxVal, hist_array, x_array, 20);
				 	
					for (int i=0;i<20;i++)
                		y_array[i] +=(int)hist_array[i];
				
					SetCtrlVal(panel,WavePanel_NUMERIC,minVal);
					SetCtrlVal(panel,WavePanel_NUMERIC_2,mean);
					SetCtrlVal(panel,WavePanel_NUMERIC_3,maxVal);
					SetCtrlVal(panel,WavePanel_NUMERIC_4,(double)minIndex);
					SetCtrlVal(panel,WavePanel_NUMERIC_5,(double)maxIndex);
					SetCtrlVal(panel,WavePanel_NUMERIC_7,select_multiplyer);
					SetCtrlVal(panel,WavePanel_NUMERIC_8,(int)(index/sampleRate));
					SetCtrlVal(panel,WavePanel_NUMERIC_9,z_cross);
					SetCtrlVal(panel,WavePanel_NUMERIC_10,dispersion);
							
					//initial plots and removal of previous charts
					DeleteGraphPlot(panel,WavePanel_IDC_GRAPH_DISPLAY1,-1,VAL_IMMEDIATE_DRAW);
					DeleteGraphPlot(panel,WavePanel_IDC_GRAPH_DISPLAY2,-1,VAL_IMMEDIATE_DRAW);
					DeleteGraphPlot(panel, WavePanel_HIST_DISPLAY,-1,VAL_IMMEDIATE_DRAW);
					
					PlotY(panel,WavePanel_IDC_GRAPH_DISPLAY1,rowSignal_split,select_multiplyer,VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_BLACK);
					PlotY(panel,WavePanel_IDC_GRAPH_DISPLAY2,filtered_rowSignal_split,select_multiplyer,VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
					PlotXY (panel, WavePanel_HIST_DISPLAY, x_array, y_array, 20, VAL_DOUBLE, VAL_INTEGER, VAL_VERTICAL_BAR, VAL_EMPTY_SQUARE, VAL_SOLID, 1,VAL_BLACK);
					
					init_split = FALSE;
					SetCtrlAttribute(panel,WavePanel_PLOTBTT,ATTR_DIMMED,1);
					SetCtrlAttribute(panel,WavePanel_PREVBTT,ATTR_DIMMED,0);
					SetCtrlAttribute(panel,WavePanel_NEXTBTT,ATTR_DIMMED,0);
					SetCtrlAttribute(panel,WavePanel_START,ATTR_DIMMED,1);
					SetCtrlAttribute(panel,WavePanel_STOP,ATTR_DIMMED,1);
				
				}
				
				break;
			}
			
			break;
	}
	return 0;
}



int CVICALLBACK OnSwitchPanelCB (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(panel ==	wavePanel)
			{
				SetCtrlVal(freqPanel, FreqPanel_SWITCHPANEL, 1);
				DisplayPanel(freqPanel);
				HidePanel(panel);
			}
			else
			{
				SetCtrlVal(wavePanel, WavePanel_SWITCHPANEL, 0);
				DisplayPanel(wavePanel);
				HidePanel(panel);
			}
			break;
	}
	return 0;
}


int CVICALLBACK OnExpBtt (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			
			//creating the filename
			//filter_window_time
			
			char filename[128];
			
    		char* windowname;
			char* filtername;
			char timename[10];
			sprintf(timename, "%d", timer);
			
			if(spectrumFilterType == 0)
				filtername = "Butterworth_6";
			else
				filtername = "Chebysev_4";
			
			strcpy(filename, filtername);
			strcat(filename, "_");
			
			if(window == 3)
				windowname = "Blackman-Harris";
			else
				windowname = "Welch";
			
			
			strcat(filename, windowname);
			strcat(filename, "_");
			strcat(filename, timename);
			strcat(filename, ".jpeg");
			
			//saving process
			int imagehdl;
			GetCtrlDisplayBitmap(freqPanel,FreqPanel_GRAPH_2,1,&imagehdl);
			SaveBitmapToJPEGFile(imagehdl,filename,JPEG_PROGRESSIVE,100);
			
			break;
	}
	return 0;
}


int CVICALLBACK OnGenerateBtt (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			//modul in care se va genera spectrul
			GetCtrlVal(freqPanel,FreqPanel_MODESWITCH,&mode);
			
			switch(mode)
			{
					
				//mod de lucru fara timer
				case 0:
				{	
					int size;
					SetCtrlAttribute( freqPanel, FreqPanel_TIMER, ATTR_ENABLED, 0 );
					SetCtrlVal(freqPanel,FreqPanel_SAMPLER,sampleRate);
					GetCtrlVal(freqPanel,FreqPanel_NSIZE,&size);
					double splitRowSignal[size];
					
					//init cu esantionul necesar
					if(splitindex + 2 * size < npoints)
					{
						splitindex += size;
						timer++;
						for(int i =  splitindex; i < splitindex + size ; i++)
						{
							splitRowSignal[i - splitindex] = rowSignal[i];
						}
					}
					else
					{
						splitindex = 0;
						timer = 0;
						for(int i =  splitindex; i <  splitindex + size ; i++)
						{
							splitRowSignal[i - splitindex] = rowSignal[i];
						}
					}
						
					
					WindowConst winConst;
					double powerPeak = 0.0; //valoarea maxima din spectru de putere (din autoSpectrum)
					double freqPeak = 0.0; //variabila ce reprezinta pasul in domeniul frecventei
					double df;
					char unit[32] = "V";
					double autoSpectrum[size/2]; //spectrul de putere cu un numar de valori egal cu jumatate din dimensiunea bufferuluide intrare
					double autoSpectrumRow[size/2];
					double filteredRow[size/2];
					double convertedSpectrum[size/2];
					
					
					if(window == 34)
						ScaledWindow (splitRowSignal,size, WELCH, &winConst);
					else
						ScaledWindow (splitRowSignal,size, BLKHARRIS, &winConst);
					
					AutoPowerSpectrum(splitRowSignal,size,1/ sampleRate ,autoSpectrum,&df);	
					PowerFrequencyEstimate (autoSpectrum, size / 2 , -1.0, winConst, df, 7, &freqPeak,&powerPeak); 
					SpectrumUnitConversion (autoSpectrum, size/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS, df, winConst, convertedSpectrum, unit);	
						
					SetCtrlVal( freqPanel, FreqPanel_FP, freqPeak);
					SetCtrlVal( freqPanel, FreqPanel_PP, powerPeak);
					SetCtrlVal( freqPanel, FreqPanel_CT, timer);
					SetCtrlVal( freqPanel, FreqPanel_IS, splitindex);
					SetCtrlVal( freqPanel, FreqPanel_NP, npoints);
				    
					DeleteGraphPlot (freqPanel, FreqPanel_GRAPH, -1, VAL_IMMEDIATE_DRAW);	
				    PlotWaveform( freqPanel, FreqPanel_GRAPH, convertedSpectrum, size/2 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df,VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS, VAL_RED);
									
					//FILTERS
					
					int spectrumfilterEnable;
					GetCtrlVal(freqPanel,FreqPanel_FE,&spectrumfilterEnable);
					GetCtrlVal(freqPanel,FreqPanel_RING,&spectrumFilterType);
					
					if(spectrumfilterEnable != 0)
					{
						
						double ff,rf,rip;
						//SetCtrlVal(freqPanel,FreqPanel_FFQ,sampleRate);
						ff = sampleRate;
						GetCtrlVal(freqPanel,FreqPanel_RFQ,&rf);
						GetCtrlVal(freqPanel,FreqPanel_RPL,&rip);
						
						
						double df2;
						
						
						
						switch(spectrumFilterType)
						{	
							case 0:
								//bw - 6th order
								Bw_LPF(splitRowSignal,size/3,ff,rf,6,filteredRow);
								
								
								AutoPowerSpectrum(filteredRow,size/3,1/sampleRate ,autoSpectrumRow,&df2);	
								SpectrumUnitConversion (autoSpectrumRow, size/6, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS, df2, winConst, convertedSpectrum, unit);
								
	
								DeleteGraphPlot (freqPanel, FreqPanel_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);	
								PlotWaveform( freqPanel, FreqPanel_GRAPH_2, convertedSpectrum, size/6 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df2,VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS, VAL_RED);

								break;
						
						
							case 1:
								//ch - 4th order
								Ch_LPF(splitRowSignal,size / 3,ff,rf,rip,4,filteredRow);
								
								AutoPowerSpectrum(filteredRow,size/3,1/sampleRate ,autoSpectrumRow,&df2);	
								SpectrumUnitConversion (autoSpectrumRow, size/6, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS, df2, winConst, convertedSpectrum, unit);
								
	
								DeleteGraphPlot (freqPanel, FreqPanel_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);	
								PlotWaveform( freqPanel, FreqPanel_GRAPH_2, convertedSpectrum, size/6 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df2,VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS, VAL_RED);

																
								break;
						}
						

					}
										
					break;
					
				}
				//mod de lucru cu timer
				case 1:
				{
					SetCtrlAttribute( freqPanel, FreqPanel_TIMER, ATTR_ENABLED, 1 );
					SetCtrlAttribute(freqPanel,FreqPanel_TIMER,ATTR_INTERVAL,0.2);
					break;
				}			
			}
			break;
	}
	return 0;
}


int CVICALLBACK OnTimer (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
			
			//0 diferente fata de varianta cu buton
			
			GetCtrlVal(freqPanel,FreqPanel_NSIZE,&numRead);
			double splitRowSignal[numRead];
			
			
			if(splitindex + 2 * numRead < npoints)
			{
				splitindex += numRead;
				timer++;
				for(int i =  splitindex; i < splitindex + numRead ; i++)
				{
					splitRowSignal[i - splitindex] = rowSignal[i];
				}
			}
			else
			{
				splitindex = 0;
				timer = 0;
				for(int i =  splitindex; i <  splitindex + numRead ; i++)
				{
					splitRowSignal[i - splitindex] = rowSignal[i];
				}
			}
			double autoSpectrumRow[numRead/2];
			double filteredRow[numRead/2];	
			double convertedSpectrum[numRead/2]; //frecventa estimata pentru spectrul de putere (maxim) din vectorul autoSpectrum	
			double powerPeak = 0.0; //valoarea maxima din spectru de putere (din autoSpectrum)
			double freqPeak = 0.0; //variabila ce reprezinta pasul in domeniul frecventei
			double df = 0.0;
			char unit[32] = "V"; //voltage signal - descriere a semnalului achizitionat
			double autoSpectrum[numRead/2]; //spectrul de putere cu un numar de valori egal cu jumatate din dimensiunea bufferuluide intrare

			WindowConst winConst;
			ScaledWindowEx (splitRowSignal,numRead, window, 0, &winConst);		
			AutoPowerSpectrum(splitRowSignal,numRead,1/ sampleRate ,autoSpectrum,&df);	
			PowerFrequencyEstimate (autoSpectrum, numRead / 2 , -1.0, winConst, df, 7, &freqPeak,&powerPeak); 
			SpectrumUnitConversion (autoSpectrum, numRead/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS, df, winConst, convertedSpectrum, unit);	
				
			SetCtrlVal( freqPanel, FreqPanel_FP, freqPeak);
			SetCtrlVal( freqPanel, FreqPanel_PP, powerPeak);
			SetCtrlVal( freqPanel, FreqPanel_CT, timer);
			SetCtrlVal( freqPanel, FreqPanel_IS, splitindex);
			SetCtrlVal( freqPanel, FreqPanel_NP, npoints);
		    
			DeleteGraphPlot (freqPanel, FreqPanel_GRAPH, -1, VAL_IMMEDIATE_DRAW);	
		    PlotWaveform( freqPanel, FreqPanel_GRAPH, convertedSpectrum, numRead/2 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df,VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS, VAL_RED);
			
			
			int spectrumfilterEnable;
			GetCtrlVal(freqPanel,FreqPanel_FE,&spectrumfilterEnable);
			GetCtrlVal(freqPanel,FreqPanel_RING,&spectrumFilterType);
			
			if(spectrumfilterEnable != 0)
					{
						
						double ff,rf,rip;
						//SetCtrlVal(freqPanel,FreqPanel_FFQ,sampleRate);
						ff = sampleRate;
						GetCtrlVal(freqPanel,FreqPanel_RFQ,&rf);
						GetCtrlVal(freqPanel,FreqPanel_RPL,&rip);
						
						
						double df2;
						char unit[32] = "V";
						
						double convertedSpectrum[numRead/2];
						
						switch(spectrumFilterType)
						{	
							case 0:
								//bw - 6th order
								Bw_LPF(splitRowSignal,numRead/3,ff,rf,6,filteredRow);
								
								
								AutoPowerSpectrum(filteredRow,numRead/3,1/sampleRate ,autoSpectrumRow,&df2);	
								SpectrumUnitConversion (autoSpectrumRow, numRead/6, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS, df2, winConst, convertedSpectrum, unit);
								
	
								DeleteGraphPlot (freqPanel, FreqPanel_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);	
								PlotWaveform( freqPanel, FreqPanel_GRAPH_2, convertedSpectrum, numRead/6 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df2,VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS, VAL_RED);

								break;
						
						
							case 1:
								//ch - 4th order
								Ch_LPF(splitRowSignal,numRead / 3,ff,rf,rip,4,filteredRow);
								
								AutoPowerSpectrum(filteredRow,numRead/3,1/sampleRate ,autoSpectrumRow,&df2);	
								SpectrumUnitConversion (autoSpectrumRow, numRead/6, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS, df2, winConst, convertedSpectrum, unit);
								
	
								DeleteGraphPlot (freqPanel, FreqPanel_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);	
								PlotWaveform( freqPanel, FreqPanel_GRAPH_2, convertedSpectrum, numRead/6 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df2,VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS, VAL_RED);

																
								break;
						}
						

					}
			break;
	}
	return 0;
}


int CVICALLBACK OnWType (int panel, int control, int event,	void *callbackData, int eventData1, int eventData2)
{
	//it would be more practic to call the getctrlval in other functions but im lazy to this is what happens
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(freqPanel, FreqPanel_WINDOWTYPE,&window);
			break;
	}
	return 0;
}