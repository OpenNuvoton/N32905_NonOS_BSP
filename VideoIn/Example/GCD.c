#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wblib.h"

UINT16 GCD(UINT16 m1, UINT16 m2)
{
#if 0
	UINT16 m;
	if(m1<m2)
	{
		m=m1; m1=m2; m2=m;
	}
	if(m1%m2==0)
		return m2;
	else
		return (GCD(m2,m1%m2));		
#else
	int i;
	for(i=m1;i>=1;i--)
	{
		if(((m1%i)==0) && ((m2%i)==0))
			break;
	}
	return i;
#endif 		
}
