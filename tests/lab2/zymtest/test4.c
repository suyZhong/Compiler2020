/*Basic array part*/

void MyFunc1(int func_nums[]){
    int i; int j;
    i = 0;
    while(i < 10){
        j = 0;
        while(j < 10){
            func_nums[i*10+j] = 1;
            j = j+1;
        }
        i = i+1;
    }
}

int main(void){
    int nums[100];

    MyFunc1(nums);

    return 0;
}