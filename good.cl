(* Good.cl - 测试正常COOL程序的各种特性 *)

class Main {
   main(): Object {{ 
      {
         out_string("Hello, World!\n");
         0;
      }
   }};
};

class A {
   x: Int <- 5;
   
   foo(): Int { x };
   
   bar(y: Int): Int { y + x };
};

class B inherits A {
   z: String <- "hello";
   
   foo(): Int { x + 10 };  (* 方法重写 *)
   
   baz(): String { z };
};

class C {
   self_test(): SELF_TYPE { self };
   
   compute(): Int {
      {
         let x: Int <- 5,
             y: Int <- 10 in
            x + y
      }
   };
};

class List {
   data: Int;
   next: List;
   
   length(): Int {
      if (isvoid next) then
         1
      else
         1 + next.length()
      fi
   };
   
   sum(): Int {
      if (isvoid next) then
         data
      else
         data + next.sum()
      fi
   };
};

class Shape {
   area(): Int { 0 };
};

class Rectangle inherits Shape {
   width: Int;
   height: Int;
   
   area(): Int { width * height };
};

class Circle inherits Shape {
   radius: Int;
   
   area(): Int { 3 * radius * radius };
};

class TestMethods {
   static_call(): Int { (new A).foo() };
   
   dispatch_test(s: String): String {
      s.concat(" world")
   };
   
   case_test(obj: Shape): Int {
      case obj of
         shape: Shape => shape.area();
         rectangle: Rectangle => rectangle.area() + 100;
         circle: Circle => circle.area() + 200;
      esac
   };
};
