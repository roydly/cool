(* Stack.cl - 测试SELF_TYPE和复杂数据结构 *)

class Main {
   main(): Object {{ 
      {
         let stack: Stack <- new Stack in
            {
               stack.push(new StackNode);
               out_string("Stack test completed\n");
               0;
            }
      }
   }};
};

class Stack {
   top: StackNode;
   
   push(node: StackNode): SELF_TYPE {
      {
         node.set_next(top);
         top <- node;
         self;
      }
   };
   
   pop(): StackNode {
      {
         let result: StackNode <- top in
            {
               top <- top.get_next();
               result;
            }
      }
   };
   
   is_empty(): Bool {
      isvoid top
   };
};

class StackNode {
   data: Int;
   next: StackNode;
   
   set_next(n: StackNode): SELF_TYPE {
      {
         next <- n;
         self;
      }
   };
   
   get_next(): StackNode { next };
   
   get_data(): Int { data };
   
   set_data(d: Int): SELF_TYPE {
      {
         data <- d;
         self;
      }
   };
};

class StackWithValue {
   content: Int;
   
   constructor(val: Int): SELF_TYPE {
      {
         content <- val;
         self;
      }
   };
   
   add_value(x: Int): SELF_TYPE {
      {
         content <- content + x;
         self;
      }
   };
   
   get_content(): Int { content };
   
   create_copy(): StackWithValue {
      (new StackWithValue).constructor(content)
   };
};

class Container {
   item: Object;
   
   store(obj: Object): SELF_TYPE {
      {
         item <- obj;
         self;
      }
   };
   
   retrieve(): Object { item };
};

class GenericContainer inherits Container {
   get_item_type(): SELF_TYPE {
      self  (* 返回当前容器实例的类型 *)
   };
};

class MethodChaining {
   value: Int <- 0;
   
   increment(): SELF_TYPE {
      {
         value <- value + 1;
         self;
      }
   };
   
   decrement(): SELF_TYPE {
      {
         value <- value - 1;
         self;
      }
   };
   
   double(): SELF_TYPE {
      {
         value <- value * 2;
         self;
      }
   };
   
   get_result(): Int { value };
   
   example_chain(): Int {
      {
         (self.increment().double().increment()).decrement();
         value;
      }
   };
};

class SelfTypeTest {
   process(obj: SELF_TYPE): SELF_TYPE { obj };
   
   create_and_process(): SELF_TYPE {
      {
         let temp: SelfTypeTest <- new SelfTypeTest in
            self.process(temp)
      }
   };
   
   recursive_self(): SELF_TYPE {
      if (isvoid self.create_and_process()) then
         self
      else
         self.recursive_self()
      fi
   };
};
