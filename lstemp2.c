#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#define INCL_RXFUNC
#include "json_load.h"
#include "rexxsaa.h"
#include "defines.h"
#include "rxpack.h"
#include "rxdef.h"
typedef  struct {
const char* first;
void * second;
} RXFDATA;

list *pListStart = NULL;
double rsaaGetDouble(RXSTRING*arg){
	char format[100] = {0};
	double ret;
	sprintf(format, "%%%.96ldle", arg->strlength);
	sscanf(arg->strptr,format, &ret);
	return ret;
};

double chebychev(double R, double Z, chParam *arrChParam){
	chParam *c;
	double X, T = 0, *Tc;
	int i = 0;
	do c = &arrChParam[i++]; while(R  < c->limit);
	X = ((Z - c->ZL) - (c->ZU - Z))/(c->ZU - c->ZL);
	Tc = malloc(sizeof(double)*c->dimension);
	Tc[0] = 1;
	Tc[1] = X;
	T = c->A[0]+c->A[1]*X;
	for (i = 2; i < c->dimension; i++){
		Tc[i] = 2*X*Tc[i-1]-Tc[i-2];
		T = T + c->A[i]*Tc[i];
	}
	free(Tc);
	/*
	for (i = 0; i < c->dimension; i++) T = T+c->A[i]*cos(i*acos(X));
	*/
	return T;
}
int temprByPtr(CalibrationData *data, double dInput, double *pdOut){
	double z, T0, T1, V0, V1;
	int idx = 1;
	dInput = fabs(dInput);
	if (dInput <= data->dLinLim){
		for (idx = 1; idx<data->ulLinDim; idx++){
			if(dInput < data->linV[idx]){
				T0 = data->linT[idx-1];
				V0 = data->linV[idx-1];
				T1 = data->linT[idx];
				V1 = data->linV[idx];
				*pdOut = T0+(dInput - V0)*(T1-T0)/(V1-V0);
				return 0;
			}

		}
	}
	if(data->input == LOG10)
		z = log10(dInput);
	else if(data->input == LINEAR)
		z = dInput;
	else return 1;
	*pdOut = chebychev(dInput, z, data->polyfit);
	return 0;
}
int temprByName(char*name, double dInput,list*pList, double*pdOut ){
	while(pList){
		if(!strncmp(name, pList->name, 12)){
			return temprByPtr((CalibrationData*)pList->val, dInput, pdOut);
		}
		pList = pList->next; 
	}
	return 1;
}
APIRET APIENTRY rflstempload( RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc,
		RFH_ARG2_TYPE argv, RFH_ARG3_TYPE qname , RFH_ARG4_TYPE retstr ){

	char fname[1000] = {0};
	int length;
	list**ppList, *pLoad;
	if (  argc != 1 ) return -1;
	length = argv[0].strlength >999?999:argv[0].strlength; 
	memmove(fname,argv[0].strptr,length );
	ppList = &pListStart;
	if(jsonLoad(fname, &pLoad)){
		fprintf(stderr,"Path: %s\nname: %s\n", getcwd(NULL,0), fname );
		return -1;
	}
	if((*ppList))do {
		if (!strcmp(pLoad->name,(*ppList)->name)){
			free((*ppList)->val);
			free((*ppList)->name);
			free(*ppList);
			break;
		}else ppList = &((*ppList)->next);
		
	}while((*ppList));
	*ppList = pLoad;

	return(RXFUNC_OK);

}
APIRET APIENTRY rflstemp( RFH_ARG0_TYPE name, RFH_ARG1_TYPE argc,
		RFH_ARG2_TYPE argv, RFH_ARG3_TYPE qname , RFH_ARG4_TYPE retstr ){
	double input, out;
	int length = 0;
	char fname[12] = {0};
	if (  argc != 2 ) return -1;
	length = argv[0].strlength >11?11:argv[0].strlength; 
	memmove(fname,argv[0].strptr,length );
	input =  rsaaGetDouble(&(argv[1])); 
	if(temprByName(fname, input, pListStart,  &out )) return -1;
	retstr->strlength = sprintf( (char *)retstr->strptr, "%le",out);
	return(RXFUNC_OK);
};
