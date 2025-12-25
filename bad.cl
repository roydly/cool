(* Bad.cl - 测试各种语义错误 *)

class Main {
   main(): Object {{ abort() }};  (* undefined function *)
};

(* 重复定义类 *)
class A { x: Int; };
class A { y: Int; };  (* ERROR: duplicate class *)

(* 继承基本类型 *)
class B inherits Int {  (* ERROR: cannot inherit from Int *)
   x: Int;
};

(* 继承未定义类 *)
class C inherits UndefinedClass {  (* ERROR: undefined parent *)
   x: Int;
};

(* 循环继承 *)
class D inherits E { };
class E inherits D { };  (* ERROR: inheritance cycle *)

(* 未定义变量 *)
class F {
   test(): Int { undefined_var + 5 };  (* ERROR: undefined variable *)
};

(* 类型不匹配 *)
class G {
   x: Int;
   y: String;
   
   test(): Int { x + y };  (* ERROR: type mismatch in arithmetic *)
   
   compare(): Bool { x = "hello" };  (* ERROR: equality type mismatch *)
};

(* 属性类型错误 *)
class H {
   self_attr: SELF_TYPE;  (* ERROR: attribute cannot have SELF_TYPE *)
};

(* 方法重写错误 - 参数数量不同 *)
class Parent {
   method(x: Int): Int { x };
};

class Child inherits Parent {
   method(): Int { 0 };  (* ERROR: different number of parameters *)
};

(* 方法重写错误 - 参数类型不同 *)
class Parent2 {
   method2(x: Int): Int { x };
};

class Child2 inherits Parent2 {
   method2(x: String): Int { 0 };  (* ERROR: parameter type mismatch *)
};

(* 方法重写错误 - 返回类型不同 *)
class Parent3 {
   method3(): Int { 0 };
};

class Child3 inherits Parent3 {
   method3(): String { "" };  (* ERROR: return type mismatch *)
};

(* 方法调用参数数量错误 *)
class I {
   foo(x: Int, y: Int): Int { x + y };
   
   test(): Int { foo(1) };  (* ERROR: wrong number of arguments *)
};

(* 方法调用参数类型错误 *)
class J {
   bar(x: Int): Int { x };
   
   test2(): Int { bar("hello") };  (* ERROR: argument type mismatch *)
};

(* 未定义方法调用 *)
class K {
   test3(): Int { undefined_method() };  (* ERROR: undefined method *)
};

(* 非法new表达式 *)
class L {
   test4(): Int { (new UndefinedType).some_method() };  (* ERROR: undefined type *)
};

(* 控制流类型错误 *)
class M {
   test5(): Object {
      if 5 then  (* ERROR: predicate not Bool *)
         1
      else
         2
      fi
   };
   
   test6(): Object {
      while "hello" do  (* ERROR: predicate not Bool *)
         {} 
      pool
   };
};
