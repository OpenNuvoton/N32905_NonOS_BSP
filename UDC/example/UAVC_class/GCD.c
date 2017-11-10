#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wblib.h"

UINT8 GCD(UINT16 m1, UINT16 m2)
{
	UINT16 m;
	if(m1<m2)
	{
		m=m1; m1=m2; m2=m;
	}
	if(m1%m2==0)
		return m2;
	else
		return (GCD(m2,m1%m2));		
}
