#include <iostream>
using namespace std;

/*
 设计模式:模版方法
 */

class ZooShow
{
public:
    void Show()
    {   
        Show0();
        Show1();
        Show2();
        Show3();
    }
protected:
    virtual void Show0()
    {
        cout << "ZooShow::Show0()" << endl;
    }
    virtual void Show2()
    {
        cout << "ZooShow::Show2()" << endl;
    }
    virtual void Show1()
    {
        
    }
    virtual void Show3()
    {
        
    }
};

class ZooShowEx : public ZooShow
{
public:
    virtual void Show1()
    {
        cout << "ZooShowEx::Show1()" << endl;
    }

    virtual void Show3()
    {
        cout << "ZooShowEx::Show3()" << endl;
    }
};

class ZooShowEx1 : public ZooShow
{
public:
    virtual void Show2()
    {
        cout << "ZooShowEx1::Show2()" << endl;
    }

    virtual void Show3()
    {
        cout << "ZooShowEx1::Show3()" << endl;
    }
};


/*
    接口隔离原则
*/
int main()
{
    ZooShow * new_zoo = new ZooShowEx();
    ZooShow * new_zoo1 = new ZooShowEx1();
    new_zoo1->Show();
}