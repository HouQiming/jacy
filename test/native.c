#include <stdio.h>
#include <string.h>
#include "native.h"

void FillStructMembers(TNativeStruct* pstruct){
	pstruct->ival=42;
	pstruct->dval=123.45;
	strcpy(pstruct->sval,"some_string");
	strcpy(pstruct->sval2,"some_string2");
}

void PrintStruct(TNativeStruct* pstruct){
	printf("ival=%d,dval=%lf,sval=%s,sval2=%s\n",pstruct->ival,pstruct->dval,pstruct->sval,pstruct->sval2);
}
