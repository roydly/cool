#include "semant.h"
#include "cool-tree.handcode.h"
#include <iostream>
#include <vector>
#include <set>

// 全局变量定义
Classes global_classes;
int semant_errors = 0;
bool semant_debug = false;
static ClassTable *class_table = NULL;

// 构造函数
ClassTable::ClassTable(Classes cs) : classes(cs) {
    semant_errors = 0;
    class_table = new SymbolTable<Symbol, Class_>();
    
    install_basic_classes();
    build_inheritance_graph();
    check_inheritance();
    type_check();
}

// 安装基本类
void ClassTable::install_basic_classes() {
    // Object 类
    Object_class = class_(Object, 
                         No_class,
                         append_Features(new Features(0),
                         append_Features(new Features(0),
                         new Features(0))),
                         String, No_class);
    class_table->addid(Object, &Object_class);
    
    // IO 类
    IO_class = class_(IO,
                     Object,
                     append_Features(new Features(0),
                     append_Features(new Features(0),
                     new Features(0))),
                     String, No_class);
    class_table->addid(IO, &IO_class);
    
    // Int 类
    Int_class = class_(Int,
                      Object,
                      new Features(0),
                      String, No_class);
    class_table->addid(Int, &Int_class);
    
    // Bool 类  
    Bool_class = class_(Bool,
                       Object,
                       new Features(0),
                       String, No_class);
    class_table->addid(Bool, &Bool_class);
    
    // String 类
    String_class = class_(Str,
                         Object,
                         append_Features(new Features(0),
                         new Features(0)),
                         String, No_class);
    class_table->addid(Str, &String_class);
}

// 构建继承图
void ClassTable::build_inheritance_graph() {
    for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
        Class_ c = classes->nth(i);
        Symbol name = c->get_name();
        
        // 检查是否已定义
        if (class_table->probe(name) != NULL) {
            semant_error(c) << "Class " << name << " was previously defined." << endl;
        } else {
            // 使用 new 分配内存，避免悬空指针
            Class_ *class_ptr = new Class_(c);
            class_table->addid(name, class_ptr);
        }
    }
}

// 检查继承关系
void ClassTable::check_inheritance() {
    // 检查基本类型的继承限制
    for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
        Class_ c = classes->nth(i);
        Symbol name = c->get_name();
        Symbol parent = c->get_parent();
        
        if (parent == Int || parent == Bool || parent == Str || parent == SELF_TYPE) {
            semant_error(c) << "Class " << name << " cannot inherit from basic class " << parent << endl;
        }
    }
    
    // 检查继承循环
    for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
        Class_ c = classes->nth(i);
        Symbol name = c->get_name();
        Symbol parent = c->get_parent();
        
        if (parent != No_class) {
            Symbol current = name;
            std::set<Symbol> visited;
            visited.insert(current);
            
            Symbol ancestor = parent;
            while (ancestor != No_class) {
                if (visited.find(ancestor) != visited.end()) {
                    semant_error(c) << "Class " << name 
                                   << ", or an ancestor of " << name 
                                   << ", is involved in an inheritance cycle." << endl;
                    break;
                }
                visited.insert(ancestor);
                
                Class_ *ancestor_class_ptr = class_table->lookup(ancestor);
                if (ancestor_class_ptr == NULL) break;
                Class_ ancestor_class = *ancestor_class_ptr;
                ancestor = ancestor_class->get_parent();
            }
        }
    }
}

// 类型检查主函数
void ClassTable::type_check() {
    // 对每个类进行类型检查
    for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
        Class_ c = classes->nth(i);
        Symbol class_name = c->get_name();
        
        // 检查方法重写
        check_method_override(c);
        
        // 检查特性
        Features features = c->get_features();
        for (int j = features->first(); features->more(j); j = features->next(j)) {
            Feature f = features->nth(j);
            
            if (auto attr = dynamic_cast<attr_class*>(f)) {
                // 属性类型检查
                Symbol attr_type = attr->get_type();
                if (attr_type == SELF_TYPE) {
                    semant_error(c) << "Attribute " << attr->get_name() 
                                   << " cannot have type SELF_TYPE" << endl;
                }
            } else if (auto method = dynamic_cast<method_class*>(f)) {
                // 方法类型检查
                Symbol return_type = method->get_return_type();
                Expression body = method->get_body();
                
                // 创建对象环境（包含self和参数）
                SymbolTable<Symbol, Symbol> *object_env = new SymbolTable<Symbol, Symbol>();
                object_env->enterscope();
                
                // 添加self变量，类型为SELF_TYPE
                object_env->addid(self, new Symbol(SELF_TYPE));
                
                // 添加方法参数到环境
                Formals formals = method->get_formals();
                for (int k = formals->first(); formals->more(k); k = formals->next(k)) {
                    Formal formal = formals->nth(k);
                    object_env->addid(formal->get_name(), new Symbol(formal->get_type()));
                }
                
                // 检查方法体类型
                Symbol body_type = type_check_expression(body, class_name, object_env, "");
                
                // 清理环境
                delete object_env;
                
                // 检查返回类型兼容性
                if (return_type == SELF_TYPE) {
                    if (body_type != SELF_TYPE) {
                        semant_error(c) << "Method " << method->get_name() 
                                       << " has return type SELF_TYPE but returns " << body_type << endl;
                    }
                } else {
                    if (!is_subtype(body_type, return_type)) {
                        semant_error(c) << "Method " << method->get_name() 
                                       << " returns " << body_type 
                                       << " but should return " << return_type << endl;
                    }
                }
            }
        }
    }
}

// 表达式类型检查
Symbol ClassTable::type_check_expression(Expression expr, Symbol current_class, 
                                        SymbolTable<Symbol, Symbol> *&object_env, 
                                        const char *filename) {
    if (auto int_const = dynamic_cast<int_const_class*>(expr)) {
        return Int;
    }
    else if (auto bool_const = dynamic_cast<bool_const_class*>(expr)) {
        return Bool;
    }
    else if (auto string_const = dynamic_cast<string_const_class*>(expr)) {
        return Str;
    }
    else if (auto var = dynamic_cast<var_class*>(expr)) {
        // 变量查找
        Symbol *type_ptr = object_env->lookup(var->get_name());
        if (type_ptr == NULL) {
            semant_error(filename, expr) << "Undefined variable " << var->get_name() << endl;
            return Object;
        }
        return *type_ptr;
    }
    else if (auto assign = dynamic_cast<assign_class*>(expr)) {
        // 赋值表达式
        Symbol var_name = assign->get_name();
        Symbol *var_type_ptr = object_env->lookup(var_name);
        
        if (var_type_ptr == NULL) {
            semant_error(filename, expr) << "Assignment to undefined variable " << var_name << endl;
            return Object;
        }
        
        Symbol var_type = *var_type_ptr;
        Symbol expr_type = type_check_expression(assign->get_expr(), current_class, object_env, filename);
        
        if (!is_subtype(expr_type, var_type)) {
            semant_error(filename, expr) << "Type " << expr_type 
                                       << " is not subtype of " << var_type << endl;
        }
        
        return expr_type;
    }
    else if (auto dispatch = dynamic_cast<dispatch_class*>(expr)) {
        // 方法调用
        Symbol expr_type = current_class;  // 默认是self
        
        if (dispatch->get_expr() != nullptr) {
            expr_type = type_check_expression(dispatch->get_expr(), current_class, object_env, filename);
            // 处理SELF_TYPE
            if (expr_type == SELF_TYPE) {
                expr_type = current_class;
            }
        }
        
        Symbol method_name = dispatch->get_name();
        method_class *method = find_method(expr_type, method_name);
        
        if (method == NULL) {
            semant_error(filename, expr) << "Dispatch to undefined method " << method_name << endl;
            return Object;
        }
        
        // 检查参数
        Expressions actuals = dispatch->get_actuals();
        Formals formals = method->get_formals();
        
        if (actuals->len() != formals->len()) {
            semant_error(filename, expr) << "Method " << method_name 
                                       << " called with wrong number of arguments" << endl;
        } else {
            for (int i = actuals->first(), j = formals->first(); 
                 actuals->more(i) && formals->more(j); 
                 i = actuals->next(i), j = formals->next(j)) {
                Expression actual = actuals->nth(i);
                Formal formal = formals->nth(j);
                
                Symbol actual_type = type_check_expression(actual, current_class, object_env, filename);
                Symbol formal_type = formal->get_type();
                
                if (!is_subtype(actual_type, formal_type)) {
                    semant_error(filename, actual) << "Actual type " << actual_type 
                                                  << " does not match formal type " << formal_type << endl;
                }
            }
        }
        
        // 处理方法返回类型
        Symbol result_type = method->get_return_type();
        Symbol original_expr_type = expr_type;  // 保存原始类型用于SELF_TYPE处理
        
        if (result_type == SELF_TYPE) {
            if (dispatch->get_expr() != nullptr && 
                type_check_expression(dispatch->get_expr(), current_class, object_env, filename) == SELF_TYPE) {
                result_type = SELF_TYPE;
            } else {
                result_type = current_class;
            }
        }
        
        return result_type;
    }
    else if (auto cond = dynamic_cast<cond_class*>(expr)) {
        // 条件表达式
        Symbol pred_type = type_check_expression(cond->get_pred(), current_class, object_env, filename);
        if (pred_type != Bool) {
            semant_error(filename, cond->get_pred()) << "Predicate of 'if' must have type Bool" << endl;
        }
        
        Symbol then_type = type_check_expression(cond->get_then_exp(), current_class, object_env, filename);
        Symbol else_type = type_check_expression(cond->get_else_exp(), current_class, object_env, filename);
        
        return lub(then_type, else_type);
    }
    else if (auto loop = dynamic_cast<loop_class*>(expr)) {
        // 循环表达式
        Symbol pred_type = type_check_expression(loop->get_pred(), current_class, object_env, filename);
        if (pred_type != Bool) {
            semant_error(filename, loop->get_pred()) << "Predicate of 'while' must have type Bool" << endl;
        }
        
        type_check_expression(loop->get_body(), current_class, object_env, filename);
        return Object;  // while循环返回Object
    }
    else if (auto block = dynamic_cast<block_class*>(expr)) {
        // 块表达式
        Symbol result_type = No_type;
        Expressions body = block->get_body();
        
        for (int i = body->first(); body->more(i); i = body->next(i)) {
            Expression e = body->nth(i);
            result_type = type_check_expression(e, current_class, object_env, filename);
        }
        
        return result_type;
    }
    else if (auto let = dynamic_cast<let_class*>(expr)) {
        // Let表达式
        Symbol var_type = let->get_type_decl();
        if (var_type == SELF_TYPE) {
            semant_error(filename, expr) << "Let variable cannot have type SELF_TYPE" << endl;
            var_type = Object;
        }
        
        // 进入新的作用域
        object_env->enterscope();
        
        // 添加变量到环境
        if (let->get_init() != nullptr) {
            Symbol init_type = type_check_expression(let->get_init(), current_class, object_env, filename);
            if (!is_subtype(init_type, var_type)) {
                semant_error(filename, let->get_init()) << "Initialization type mismatch" << endl;
            }
        }
        
        object_env->addid(let->get_identifier(), new Symbol(var_type));
        
        // 检查in表达式
        Symbol body_type = type_check_expression(let->get_body(), current_class, object_env, filename);
        
        // 退出作用域
        object_env->exitscope();
        
        return body_type;
    }
    else if (auto typcase = dynamic_cast<typcase_class*>(expr)) {
        // Case表达式
        type_check_expression(typcase->get_expr(), current_class, object_env, filename);
        
        std::vector<Symbol> branch_types;
        Cases cases = typcase->get_cases();
        
        for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
            Branch branch = cases->nth(i);
            Symbol branch_type = branch->get_type_decl();
            
            // 检查分支类型唯一性
            for (Symbol prev_type : branch_types) {
                if (prev_type == branch_type) {
                    semant_error(filename, branch) << "Duplicate branch type " << branch_type << endl;
                    break;
                }
            }
            branch_types.push_back(branch_type);
            
            // 检查分支表达式
            object_env->enterscope();
            object_env->addid(branch->get_name(), new Symbol(branch_type));
            
            Symbol case_type = type_check_expression(branch->get_expr(), current_class, object_env, filename);
            branch_types.back() = case_type;  // 使用实际表达式类型
            
            object_env->exitscope();
        }
        
        // 返回所有分支类型的LUB
        Symbol result_type = branch_types[0];
        for (size_t i = 1; i < branch_types.size(); i++) {
            result_type = lub(result_type, branch_types[i]);
        }
        return result_type;
    }
    else if (auto new_ = dynamic_cast<new__class*>(expr)) {
        // New表达式
        Symbol type_name = new_->get_type_name();
        if (type_name != SELF_TYPE && get_class(type_name) == NULL) {
            semant_error(filename, expr) << "new: undefined type " << type_name << endl;
        }
        return type_name;
    }
    else if (auto isvoid = dynamic_cast<isvoid_class*>(expr)) {
        // IsVoid表达式
        type_check_expression(isvoid->get_expr(), current_class, object_env, filename);
        return Bool;
    }
    else if (auto plus = dynamic_cast<plus_class*>(expr) ||
               auto minus = dynamic_cast<minus_class*>(expr) ||
               auto times = dynamic_cast<times_class*>(expr) ||
               auto divide = dynamic_cast<divide_class*>(expr)) {
        // 算术运算
        Symbol left_type = type_check_expression(plus->get_left(), current_class, object_env, filename);
        Symbol right_type = type_check_expression(plus->get_right(), current_class, object_env, filename);
        
        if (left_type != Int || right_type != Int) {
            semant_error(filename, expr) << "Arithmetic operation on non-integer operands" << endl;
        }
        
        return Int;
    }
    else if (auto lt = dynamic_cast<lt_class*>(expr) ||
               auto leq = dynamic_cast<leq_class*>(expr)) {
        // 比较运算
        Symbol left_type = type_check_expression(lt->get_left(), current_class, object_env, filename);
        Symbol right_type = type_check_expression(lt->get_right(), current_class, object_env, filename);
        
        if (left_type != Int || right_type != Int) {
            semant_error(filename, expr) << "Comparison operation on non-integer operands" << endl;
        }
        
        return Bool;
    }
    else if (auto eq = dynamic_cast<eq_class*>(expr)) {
        // 相等运算
        Symbol left_type = type_check_expression(eq->get_left(), current_class, object_env, filename);
        Symbol right_type = type_check_expression(eq->get_right(), current_class, object_env, filename);
        
        // 基本类型之间可以比较
        if ((left_type == Int || left_type == Bool || left_type == Str) && 
            left_type != right_type) {
            semant_error(filename, expr) << "Equality comparison between different basic types" << endl;
        }
        
        return Bool;
    }
    else if (auto comp = dynamic_cast<comp_class*>(expr)) {
        // 逻辑非
        Symbol type = type_check_expression(comp->get_expr(), current_class, object_env, filename);
        if (type != Bool) {
            semant_error(filename, comp->get_expr()) << "'not' operand must have type Bool" << endl;
        }
        return Bool;
    }
    else if (auto neg = dynamic_cast<neg_class*>(expr)) {
        // 算术取负
        Symbol type = type_check_expression(neg->get_expr(), current_class, object_env, filename);
        if (type != Int) {
            semant_error(filename, neg->get_expr()) << "'~' operand must have type Int" << endl;
        }
        return Int;
    }
    
    return Object;  // 默认返回Object
}

// 辅助方法实现
bool ClassTable::is_subtype(Symbol child, Symbol parent) {
    if (child == parent) return true;
    if (child == SELF_TYPE && parent == SELF_TYPE) return true;
    if (child == SELF_TYPE) return true;  // SELF_TYPE 可赋值给任何类型
    if (parent == SELF_TYPE) return false; // 任何类型不能赋值给SELF_TYPE
    if (child == No_type) return true;    // No_type 是所有类型的子类型
    
    // 常规继承检查
    if (child == Object) return false;
    
    Class_ child_class = get_class(child);
    if (child_class == NULL) return false;
    
    Symbol parent_of_child = child_class->get_parent();
    return parent_of_child != No_class && is_subtype(parent_of_child, parent);
}

Symbol ClassTable::lub(Symbol type1, Symbol type2) {
    if (type1 == type2) return type1;
    if (type1 == No_type) return type2;
    if (type2 == No_type) return type1;
    if (type1 == SELF_TYPE && type2 == SELF_TYPE) return SELF_TYPE;
    if (type1 == SELF_TYPE || type2 == SELF_TYPE) return Object;
    
    // 如果一个是另一个的子类型，返回父类型
    if (is_subtype(type1, type2)) return type2;
    if (is_subtype(type2, type1)) return type1;
    
    // 寻找共同祖先
    Class_ c1 = get_class(type1);
    Class_ c2 = get_class(type2);
    
    if (c1 == NULL || c2 == NULL) return Object;
    
    // 收集所有祖先
    std::vector<Symbol> ancestors1, ancestors2;
    Symbol current = type1;
    while (current != No_class) {
        ancestors1.push_back(current);
        Class_ cls = get_class(current);
        if (cls == NULL) break;
        current = cls->get_parent();
    }
    
    current = type2;
    while (current != No_class) {
        ancestors2.push_back(current);
        Class_ cls = get_class(current);
        if (cls == NULL) break;
        current = cls->get_parent();
    }
    
    // 从最具体到最通用寻找共同祖先
    for (int i = 0; i < ancestors1.size(); i++) {
        for (int j = 0; j < ancestors2.size(); j++) {
            if (ancestors1[i] == ancestors2[j]) {
                return ancestors1[i];
            }
        }
    }
    
    return Object;  // 最坏情况下返回Object
}

Class_ ClassTable::get_class(Symbol name) {
    if (name == Object) return &Object_class;
    if (name == IO) return &IO_class;
    if (name == Int) return &Int_class;
    if (name == Bool) return &Bool_class;
    if (name == Str) return &Str_class;
    
    Class_ *class_ptr = class_table->lookup(name);
    return class_ptr ? *class_ptr : NULL;
}

void ClassTable::check_method_override(Class_ cls) {
    Symbol class_name = cls->get_name();
    Features features = cls->get_features();
    
    for (int i = features->first(); features->more(i); i = features->next(i)) {
        Feature f = features->nth(i);
        if (auto method = dynamic_cast<method_class*>(f)) {
            Symbol method_name = method->get_name();
            
            // 在父类中查找同名方法
            Class_ current_class = cls;
            Symbol parent = current_class->get_parent();
            
            while (parent != No_class) {
                Class_ *parent_class_ptr = class_table->lookup(parent);
                if (parent_class_ptr == NULL) break;
                
                Class_ parent_class = *parent_class_ptr;
                Features parent_features = parent_class->get_features();
                
                for (int j = parent_features->first(); parent_features->more(j); j = parent_features->next(j)) {
                    Feature pf = parent_features->nth(j);
                    if (auto parent_method = dynamic_cast<method_class*>(pf)) {
                        if (parent_method->get_name() == method_name) {
                            // 检查方法签名兼容性
                            Formals parent_formals = parent_method->get_formals();
                            Formals child_formals = method->get_formals();
                            
                            // 检查参数数量
                            if (parent_formals->len() != child_formals->len()) {
                                semant_error(cls) << "Method " << method_name 
                                                << " overrides method with different number of parameters" << endl;
                                return;
                            }
                            
                            // 检查参数类型
                            bool params_compatible = true;
                            for (int k = parent_formals->first(), l = child_formals->first();
                                 parent_formals->more(k) && child_formals->more(l);
                                 k = parent_formals->next(k), l = child_formals->next(l)) {
                                Formal parent_f = parent_formals->nth(k);
                                Formal child_f = child_formals->nth(l);
                                
                                if (parent_f->get_type() != child_f->get_type()) {
                                    params_compatible = false;
                                    break;
                                }
                            }
                            
                            // 检查返回类型
                            Symbol parent_return = parent_method->get_return_type();
                            Symbol child_return = method->get_return_type();
                            
                            if (!params_compatible) {
                                semant_error(cls) << "Method " << method_name 
                                                << " overrides method with incompatible parameter types" << endl;
                                return;
                            }
                            
                            if (parent_return != child_return) {
                                semant_error(cls) << "Method " << method_name 
                                                << " overrides method with different return type" << endl;
                                return;
                            }
                            
                            return;  // 找到并重写了方法，退出
                        }
                    }
                }
                
                parent = parent_class->get_parent();
            }
        }
    }
}

method_class* ClassTable::find_method(Symbol class_name, Symbol method_name) {
    Class_ cls = get_class(class_name);
    if (cls == NULL) return NULL;
    
    // 在当前类中查找
    Features features = cls->get_features();
    for (int i = features->first(); features->more(i); i = features->next(i)) {
        Feature f = features->nth(i);
        if (auto method = dynamic_cast<method_class*>(f)) {
            if (method->get_name() == method_name) {
                return method;
            }
        }
    }
    
    // 在父类中递归查找
    Symbol parent = cls->get_parent();
    if (parent != No_class) {
        return find_method(parent, method_name);
    }
    
    return NULL;
}

// 错误报告方法
void ClassTable::semant_error(Class_ c) {
    semant_error() << "In class " << c->get_name() << ": ";
}

void ClassTable::semant_error(Class_ c, const char *msg) {
    semant_error(c) << msg;
}

void ClassTable::semant_error(const char *filename, tree_node *t, const char *msg) {
    semant_error() << filename << ":" << t->get_line_number() << ": " << msg;
}

std::ostream& semant_error() {
    semant_errors++;
    cool::cerr << "ERROR: ";
    return cool::cerr;
}

// 主函数（由semant-phase.cc调用）
void program_class::semant() {
    global_classes = classes;
    class_table = new ClassTable(classes);
    
    if (semant_errors > 0) {
        cool::cerr << semant_errors << " semantic errors." << endl;
    }
    
    delete class_table;
}
