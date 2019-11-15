#include <iostream>
#include"test.h"

using namespace std;

//在C++中想调用C中的函数

//extern "C" void show();           //一个一个函数调用的方法

int main()
{
	show();

	return 0;
}

