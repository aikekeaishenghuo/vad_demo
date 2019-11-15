#include <iostream>
 
using namespace std;
const int MAX = 4;
 
int main ()
{
 const char *names[MAX] = {
                   "Zara Ali",
                   "Hina Ali",
                   "Nuha Ali",
                   "Sara Ali",
   };
     char arr[MAX];
     for (int i = 0; i < MAX; i++)
     {
          printf("指针%c",names[i]);
     }

//     for (int i = 0; i < MAX; i++)
//     {
//          printf("数组%d\n",arr[i]);
//     }

     
//   for (int i = 0; i < MAX; i++)
//   {
//      cout << "Value of names[" << i << "] = ";
//      cout << names[i] << endl;
//   }
   return 0;
}
