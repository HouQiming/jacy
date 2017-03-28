#ifndef __NATIVE_H
#define __NATIVE_H
typedef struct{
	int ival;
	double dval;
	char sval[32];
	char sval2[32];
}TNativeStruct;

void FillStructMembers(TNativeStruct* pstruct);
void PrintStruct(TNativeStruct* pstruct);
#endif
