/*
    I konw it's weird, even stupid, to code C like this. w(ﾟДﾟ)w
    HOWEVER, we have to use some tricky cases to test your answer.
*/

float GVAR;
void NeverEverDeclareLikeThis;
int GARRAY[2333];

void MyFunc1(int floatNum, float intNum, void voidNums[]){
    int IKnowYouAreVoid;
    return MyFunc2(IKnowYouAreVoid);
}

float MyFunc2(void){
    int IAmVoid[0];
    return MyFunc1(.0, 0, IAmVoid);
}

int main(void){

    int a; int b; int c;

    a = b = c = (85 == 84 + 0.4);

    if(a = b){
        GARRAY[ ( MyFunc2() ) ] = GARRAY[c = 1.*.1 == 1.1];
    }else if (MyFunc3(NotDeclared)){

    }else;

    return 0.;
}