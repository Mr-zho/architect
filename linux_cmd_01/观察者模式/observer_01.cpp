#include <iostream>
using namespace std;

/*
 观察者模式
*/

// 订阅者1
class DisplayA{
public:
    void Show(float temp)
    {
        cout << "DisplayA::temp:" << temp << endl;
    }
};

// 订阅者2
class DisplayB{
public:
    void Show(float temp)
    {
        cout << "DisplayB::temp:" << temp << endl;
    }
};


class WeatherData{
};


class DataCenter{
public:
    float CalcTemperature()
    {
        WeatherData * data = GetWeatherData();

        /* 
            数据中心的一通处理
            返回一个数据
        */
        float temper = 6.6;
        return temper;
    }
private:
    WeatherData * GetWeatherData()
    {
        return NULL;
    }
};

int main()
{
    DataCenter * datacenter = new DataCenter;
    DisplayA * da1 = new DisplayA;
    DisplayB * da2 = new DisplayB;

    float temper = datacenter->CalcTemperature();
    
    da1->Show(temper);
    da2->Show(temper);
    return 0;
}