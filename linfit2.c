/*
   Simple RexxSort() function in C
 */

#define INCL_RXFUNC               
#define INCL_RXSHV                
#include <rexxsaa.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct _Point 
{
	double x,y;
} Point, *pPoint;

int compfn(  const void *p1, const  void *p2 ) 
{
	return ((pPoint)p1)->x - ((pPoint)p2)->x ;
}

int linreg(int n, pPoint pt, double* m, double* b, double* r){
	double   sumx = 0.0;                      
	double   sumx2 = 0.0;                     
	double   sumxy = 0.0;                     
	double   sumy = 0.0;                      
	double   sumy2 = 0.0;                     
	double denom = 0.0;
	int i = 0;

	for ( i=0;i<n;i++){ 
		sumx  += pt[i].x;       
		sumx2 += pt[i].x*pt[i].x;  
		sumxy += pt[i].x * pt[i].y;
		sumy  += pt[i].y;      
		sumy2 += pt[i].y*pt[i].y; 
	} 

	denom = (n * sumx2 - (sumx*sumx));
	if (denom == 0) {
		*m = 0;
		*b = 0;
		if (r) *r = 0;
		return 1;
	}

	*m = (n * sumxy  -  sumx * sumy) / denom;
	*b = (sumy * sumx2  -  sumx * sumxy) / denom;
	if (r!=NULL) {
		*r = (sumxy - sumx * sumy / n) /    
			sqrt((sumx2 - (sumx*sumx)/n) *
					(sumy2 - (sumy*sumy)/n));
	}

	return 0; 
}
int calcmean(int n, pPoint pt, double *sum){
	int i = 0;
	*sum = 0;
	if(!n) return 1;
	for( i=0; i < n; i++) (*sum) += pt[i].y;
	*sum /= n;
	return 0;
}
int stddev(int n, pPoint pt, double *ret){
	int i;
	double mean, sum = 0;

	if (calcmean(n, pt, &mean)) return 1;
	for( i=0; i < n; i++) sum += (pt[i].y - mean)*(pt[i].y - mean);
	*ret = sqrt(sum/(n-1));
	return 0;

}
long loadVars( long argc, PRXSTRING argv, PRXSTRING retstr , long*count, pPoint* ppDest) {
	int i = 0;                    
	SHVBLOCK shvblock = {0};       
	long numVars = 0;
	pPoint pSortBlock = NULL;       
	char buff[ 256]  = "";         
	PSZ pszStem = NULL;            


	
	if ( ( argc != 1 ) || !RXVALIDSTRING( argv[0] ) )
		return 1;

	pszStem = RXSTRPTR(argv[0]);

	
	MAKERXSTRING( shvblock.shvname, buff, sprintf( buff, "%s.%d", pszStem, i ) );
	MAKERXSTRING( shvblock.shvvalue, 0, 0 );
	shvblock.shvcode = RXSHV_SYFET;

	if ( RexxVariablePool(&shvblock) != 0 )
		return 1;

	if ( !RXVALIDSTRING( shvblock.shvvalue ) )
		return 1;

	numVars = atoi( shvblock.shvvalue.strptr );
	RexxFreeMemory( shvblock.shvvalue.strptr ); 
	*count = numVars;
	if ( numVars ) {
		pSortBlock = calloc( (size_t)numVars, sizeof( pSortBlock[0] ) );
		if ( pSortBlock == NULL )
			return 40;

		
		for ( i = 0; i < numVars; i++ ) {
			MAKERXSTRING( shvblock.shvname, buff, sprintf( buff, "%s.%i",
						pszStem, i+1 ) );
			MAKERXSTRING( shvblock.shvvalue, 0, 0 );
			shvblock.shvcode = RXSHV_SYFET;
			RexxVariablePool(&shvblock);
			sscanf(RXSTRPTR(shvblock.shvvalue),"%le:%le",&(pSortBlock[i].x), &(pSortBlock[i].y) );
			RexxFreeMemory( shvblock.shvvalue.strptr ); 
		}
		*ppDest = pSortBlock;
	}
	return 0;
}
APIRET APIENTRY rflinfit( PUCHAR Name, unsigned long argc, PRXSTRING argv,
		PSZ QueueName, PRXSTRING retstr ) {
	pPoint pSortBlock = NULL;       
	long numVars = 0;              
	int rc;
	double k,b;
	Name = Name;                   
	QueueName = QueueName;

	rc = loadVars(argc, argv, retstr, &numVars, &pSortBlock);
	if(!rc){
		linreg(numVars, pSortBlock, &k, &b, NULL );
		free( pSortBlock );
	}

	
	retstr->strlength = sprintf( (char *)retstr->strptr, "%le:%le",k,b);
	return rc;
}
APIRET APIENTRY rfmean( PUCHAR Name, unsigned long argc, PRXSTRING argv,
		PSZ QueueName, PRXSTRING retstr ) {
	pPoint pSortBlock = NULL;       
	int rc;
	long numVars = 0;              
	double res;
	Name = Name;                   
	QueueName = QueueName;

	rc = loadVars(argc, argv, retstr, &numVars, &pSortBlock);
	if(!rc){
		rc = calcmean(numVars, pSortBlock, &res );
		free( pSortBlock );
	}

	
	retstr->strlength = sprintf( (char *)retstr->strptr, "%le", res);
	return rc;
}
APIRET APIENTRY rfstddev( PUCHAR Name, unsigned long argc, PRXSTRING argv,
		PSZ QueueName, PRXSTRING retstr ) {
	pPoint pSortBlock = NULL;       
	int rc;
	long numVars = 0;              
	double res;
	Name = Name;                   
	QueueName = QueueName;

	rc = loadVars(argc, argv, retstr, &numVars, &pSortBlock);
	if(!rc){
		rc = stddev(numVars, pSortBlock, &res );
		free( pSortBlock );
	}

	
	retstr->strlength = sprintf( (char *)retstr->strptr, "%le", res);
	return rc;
}
