(* Complex.cl - 综合测试各种语言特性 *)

class Main {
   main(): Object {{ 
      {
         out_string("Starting comprehensive test...\n");
         let tester: ComprehensiveTester <- new ComprehensiveTester in
            tester.run_all_tests();
         out_string("All tests completed!\n");
         0;
      }
   }};
};

class ComprehensiveTester {
   run_all_tests(): Object {
      {
         test_inheritance();
         test_polymorphism();
         test_scoping();
         test_type_inference();
         test_control_flow();
         test_let_expressions();
         test_case_expressions();
         test_arithmetic();
         test_comparison();
         test_logical_ops();
         self;
      }
   };
   
   test_inheritance(): Object {
      {
         let parent: BaseClass <- new BaseClass,
             child: DerivedClass <- new DerivedClass in
            {
               out_string("Testing inheritance: ");
               out_int(parent.base_method());
               out_string(" ");
               out_int(child.base_method());  (* 多态调用 *)
               out_string("\n");
            }
      }
   };
   
   test_polymorphism(): Object {
      {
         let shapes: ShapeList <- new ShapeList in
            {
               shapes.add_shape(new Rectangle);
               shapes.add_shape(new Circle);
               out_string("Testing polymorphism: ");
               out_int(shapes.total_area());
               out_string("\n");
            }
      }
   };
   
   test_scoping(): Object {
      {
         let x: Int <- 10 in
            {
               let x: Int <- 20 in  (* 内层作用域遮蔽外层 *)
                  out_string("Testing scoping - inner: ");
               out_int(x);  (* 应该是20 *)
               out_string(", outer: ");
               out_int(x);  (* 这里会有问题，演示作用域 *)
               out_string("\n");
            }
      }
   };
   
   test_type_inference(): Object {
      {
         let result: Int <- 
            if true then
               42
            else
               24
            fi in
            {
               out_string("Testing type inference: ");
               out_int(result);
               out_string("\n");
            }
      }
   };
   
   test_control_flow(): Object {
      {
         let counter: Int <- 0 in
            {
               while counter < 3 do
                  {
                     out_string("Counter: ");
                     out_int(counter);
                     out_string("\n");
                     counter <- counter + 1;
                  }
               pool;
               out_string("Final counter: ");
               out_int(counter);
               out_string("\n");
            }
      }
   };
   
   test_let_expressions(): Object {
      {
         let a: Int <- 5,
             b: Int <- 10,
             result: Int <- 
                let temp: Int <- a + b in
                   temp * 2 in
            {
               out_string("Testing let expressions: ");
               out_int(result);
               out_string("\n");
            }
      }
   };
   
   test_case_expressions(): Object {
      {
         let obj: Object <- new Rectangle in
            {
               let area: Int <- 
                  case obj of
                     r: Rectangle => r.get_width() * r.get_height();
                     c: Circle => 3 * c.get_radius() * c.get_radius();
                     s: Shape => s.default_area();
                     o: Object => 0;
                  esac in
                  {
                     out_string("Testing case expressions: ");
                     out_int(area);
                     out_string("\n");
                  }
            }
      }
   };
   
   test_arithmetic(): Object {
      {
         out_string("Testing arithmetic: ");
         out_int(10 + 20 * 3 - 5 / 2);
         out_string("\n");
      }
   };
   
   test_comparison(): Object {
      {
         out_string("Testing comparisons: ");
         out_string(if 5 < 10 then "5 < 10: true" else "false" fi);
         out_string(" ");
         out_string(if 5 <= 5 then "5 <= 5: true" else "false" fi);
         out_string("\n");
      }
   };
   
   test_logical_ops(): Object {
      {
         out_string("Testing logical operations: ");
         out_string(if not false then "not false: true" else "false" fi);
         out_string(" ");
         out_string(if true and false then "true and false: true" else "false" fi);
         out_string("\n");
      }
   };
};

class BaseClass {
   base_field: Int <- 100;
   
   base_method(): Int { base_field };
   
   another_base_method(): String { "base" };
};

class DerivedClass inherits BaseClass {
   derived_field: Int <- 200;
   
   base_method(): Int { base_field + derived_field };  (* 重写方法 *)
   
   derived_method(): String { "derived" };
};

class Shape {
   default_area(): Int { 0 };
};

class Rectangle inherits Shape {
   width: Int <- 5;
   height: Int <- 10;
   
   get_width(): Int { width };
   get_height(): Int { height };
   
   default_area(): Int { width * height };  (* 重写 *)
};

class Circle inherits Shape {
   radius: Int <- 7;
   
   get_radius(): Int { radius };
   
   default_area(): Int { 3 * radius * radius };  (* 重写 *)
};

class ShapeList {
   head: ShapeNode;
   count: Int <- 0;
   
   add_shape(shape: Shape): SELF_TYPE {
      {
         let new_node: ShapeNode <- new ShapeNode in
            {
               new_node.set_shape(shape);
               new_node.set_next(head);
               head <- new_node;
               count <- count + 1;
               self;
            }
      }
   };
   
   total_area(): Int {
      let current: ShapeNode <- head,
          total: Int <- 0 in
         {
            while not isvoid current do
               {
                  total <- total + current.get_shape().default_area();
                  current <- current.get_next();
               }
            pool;
            total;
         }
   };
};

class ShapeNode {
   shape: Shape;
   next: ShapeNode;
   
   set_shape(s: Shape): SELF_TYPE {
      {
         shape <- s;
         self;
      }
   };
   
   get_shape(): Shape { shape };
   
   set_next(n: ShapeNode): SELF_TYPE {
      {
         next <- n;
         self;
      }
   };
   
   get_next(): ShapeNode { next };
};

class AdvancedFeatures {
   factorial(n: Int): Int {
      if n <= 1 then
         1
      else
         n * self.factorial(n - 1)
      fi
   };
   
   fibonacci(n: Int): Int {
      if n <= 1 then
         n
      else
         self.fibonacci(n - 1) + self.fibonacci(n - 2)
      fi
   };
   
   complex_calculation(x: Int, y: Int): Int {
      let intermediate: Int <- 
         if x > y then
            x * self.factorial(y)
         else
            y * self.factorial(x)
         fi in
         {
            let doubled: Int <- intermediate * 2 in
               case intermediate of
                  i: Int => if i > 100 then doubled else intermediate fi;
                  _: Object => 0;
               esac
         }
   };
};
