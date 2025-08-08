enum {UNDEF, LINEAR, LOG10};
typedef struct {
	int dimension ;
	double ZL, ZU,limit, *A;
	double *linT, *linV;
} chParam;
typedef struct _list {
	void*val;
	char *name;
	struct _list *next;	
}list;
typedef struct {
	chParam*polyfit;
	unsigned int input;
	double dLinLim;
	unsigned long ulLinDim;
	double*linT, *linV;
}CalibrationData;
int jsonLoad(char* , list** );
