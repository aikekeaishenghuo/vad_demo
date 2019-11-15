#include <stdio.h>
#include <stdlib.h>
static int array[3];
//int *array= NULL;
int main()
{

    int *ptest = NULL;

    ptest = (int*)malloc(3*sizeof(int));

    ptest[0] = 1;
    ptest[1] = -32;
    ptest[2] = 21 ; 
    for(int i=0;i<3;i++){
        array[i] =(int)ptest[i];
	  printf("gcw\n");
//        ++ptest;
    }
//    array[0] =(int)*ptest;
    printf("val1:%d val2:%d cal3:%d\n",array[0],array[1],array[2]);

    return 0;

}
