#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>

#include "json.h"
#include "json_load.h"
typedef struct _FreeList{
	void*ptr;
	struct _FreeList* next;
}FreeList;
FreeList *freeStart, *freeCurr;
int fetchCoeff(json_value *, list **);
void saveFree (void*ptr){
	freeCurr->ptr = ptr;
	freeCurr->next = calloc(1, sizeof(FreeList));
	freeCurr = freeCurr->next;
}
void freeAll(){
	if(!freeStart->ptr) return;
	free(freeStart->ptr);
	freeCurr = freeStart->next;
	free(freeStart);
	while(freeCurr){
		free(freeCurr->ptr);
		freeStart = freeCurr;
		freeCurr = freeCurr->next;
		free(freeStart);
	}
}
int jsonLoad(char* filename , list** out) {
	FILE *fp;
	struct stat filestatus;
	int file_size;
	char* file_contents;
	int idx, rc;
	json_char* json;
	json_value* value;
	list*pListStart, *pListCurr;
	freeStart = calloc(1,sizeof(FreeList));
	freeCurr = freeStart;

	if ( stat(filename, &filestatus) != 0) {
		return 1;
	}
	file_size = filestatus.st_size;
	file_contents = (char*)malloc(filestatus.st_size);
	if ( file_contents == NULL) {
		return 1;
	}

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fclose(fp);
		free(file_contents);
		return 1;
	}
	rc = fread(file_contents,1, file_size, fp) ;
	if ( rc != file_size ) {
		fclose(fp);
		free(file_contents);
		return 1;
	}
	fclose(fp);


	json = (json_char*)file_contents;

	value = json_parse(json,file_size);
	if(!value||(value->type != json_array)){
		free(file_contents);
		return 1;
	}
	for(idx = 0; idx<value->u.array.length;idx++){
		if (!idx){
			if (fetchCoeff(value->u.array.values[idx], &pListStart)){
				free(file_contents);freeAll();
				return 1;
			}
			pListCurr = pListStart;
		}else{
			if(fetchCoeff(value->u.array.values[idx], &(pListCurr->next))){
				free(file_contents);freeAll();
				return 1;
			}
			pListCurr = pListCurr->next;
		}

	}
	*out = pListStart;
	free(file_contents);
	return 0;	
}

int fetchCoeff(json_value *value, list **out){
	int idx, idxA, idxO, idxOL;
	double **ppdLinDest;
	char*pszName = 0;
	char* name;
	list*pList;
	json_value* pjvVals = 0;
	json_value* val = 0;
	json_value* valLin = 0;
	json_value* valChP = 0;
	chParam * arrChParam = 0;
	CalibrationData * dataOut;
	dataOut = malloc(sizeof(CalibrationData));
	saveFree(dataOut );
	dataOut->input = UNDEF;				
	for (idx = 0; idx<value->u.object.length; idx++){
		if(!strcmp(value->u.object.values[idx].name,"name"))
			pszName = value->u.object.values[idx].value->u.string.ptr;
		else if(!strcmp(value->u.object.values[idx].name,"vals"))
			pjvVals = value->u.object.values[idx].value;
		else if(!strcmp(value->u.object.values[idx].name,"input")){
			if(!strcmp(value->u.object.values[idx].value->u.string.ptr, "log10"))
				dataOut->input = LOG10;				
			else if(!strcmp(value->u.object.values[idx].value->u.string.ptr, "linear"))
				dataOut->input = LINEAR;				
		}
		else if(!strcmp(value->u.object.values[idx].name,"linlim"))
			dataOut->dLinLim = value->u.object.values[idx].value->u.dbl;
		else if(!strcmp(value->u.object.values[idx].name,"lin")){
			for(idxOL = 0; idxOL < value->u.object.values[idx].value->u.object.length; idxOL++){ 
				valLin = value->u.object.values[idx].value->u.object.values[idxOL].value;
				if(!strcmp(value->u.object.values[idx].value->u.object.values[idxOL].name,"T")) ppdLinDest = &dataOut->linT;
				else if(!strcmp(value->u.object.values[idx].value->u.object.values[idxOL].name,"V")) ppdLinDest = &dataOut->linV;
				*ppdLinDest = malloc(valLin->u.array.length*sizeof(double));
				saveFree(*ppdLinDest );
				dataOut->ulLinDim = valLin->u.array.length; 
				for(idxA = 0; idxA <  valLin->u.array.length; idxA++)
					(*ppdLinDest)[idxA] = valLin->u.array.values[idxA]->u.dbl;
				
			}
		}	
	}
	if(!pszName || !pjvVals){
		return 1;
	}

	pList = malloc(sizeof(list));
	saveFree(pList );
	pList->next = NULL;
	pList->name = malloc((sizeof(char)*strlen(pszName)));
	saveFree(pList->name);
	strcpy(pList->name, pszName);
	arrChParam = malloc(pjvVals->u.array.length*sizeof(chParam));
	saveFree(arrChParam );
	for(idx = 0; idx<pjvVals->u.array.length;idx++){
		valChP = pjvVals->u.array.values[idx];
		for(idxO = 0; idxO < valChP->u.object.length; idxO++){ 
			val = valChP->u.object.values[idxO].value;
			name = valChP->u.object.values[idxO].name;
			if(!strcmp(name,"ZU")){
				arrChParam[idx].ZU = val->u.dbl;
			}else if(!strcmp(name,"ZL")){
				arrChParam[idx].ZL = val->u.dbl;
			}else if(!strcmp(name,"A")){
				arrChParam[idx].dimension = val->u.array.length;
				arrChParam[idx].A = malloc(val->u.array.length*sizeof(double));
				saveFree(arrChParam[idx].A);
				for(idxA = 0; idxA <  val->u.array.length; idxA++)
					arrChParam[idx].A[idxA] = val->u.array.values[idxA]->u.dbl;

			}else if(!strcmp(name,"limit")){
				arrChParam[idx].limit = val->u.dbl;
			}
		}

	}
	dataOut->polyfit = arrChParam;
	pList->val = (void*)dataOut;
	*out = pList;
	return 0;
}
