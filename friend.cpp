#include <iostream>
using namespace std;
class B;
class A{
public:
  int a;
  A(int x){
    a=x;
  }
  friend void swap(A&,B&);
  void display(){
    cout<<"Value of a is "<<a<<endl;
  }
};

class B{
public:
  int b;
  B(int y){
    b=y;
  }
  friend void swap(A&,B&);
  void display(){
    cout<<"Value of b is "<<b<<endl;
  }
};


void swap(A& x,B& y){
  int temp;
  temp = x.a;
  x.a = y.b;
  y.b = temp;
}

int main(){
  A ob1 = A(10);
  B ob2 = B(5);
  ob1.display();
  ob2.display();
  swap(ob1,ob2);
  ob1.display();
  ob2.display();

  return 0;
}
