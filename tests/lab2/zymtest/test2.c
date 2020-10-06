/*Basic function part*/

float MyFunc1(int a, float x){
    if(a <= 0){
        return x+x*x;
    }else{
        return MyFunc1(a-1, x+.1);
    }
}

int main(void){
    float result;

    result = Myfunc1(5, 1.) + MyFunc1(0, MyFunc1(0, .1));

    return 0;
}