#ifndef SEMANT_H
#define SEMANT_H

#include "cool-tree.h"
#include "symtab.h"
#include "stringtab.h"
#include "utilities.h"

class ClassTable;
typedef ClassTable *ClassTableP;

// 全局变量
extern Classes global_classes;
extern int semant_errors;
extern bool semant_debug;

// 语义分析器主类
class ClassTable {
private:
    int semant_errors;
    void install_basic_classes();
    void build_inheritance_graph();
    void check_inheritance();
    void type_check();

public:
    ClassTable(Classes);
    Classes classes;
    SymbolTable<Symbol, Class_> *class_table;
    
    // 基本类成员变量（重要：必须用成员变量而非局部变量）
    Class_ Object_class;
    Class_ IO_class;
    Class_ Int_class;
    Class_ Bool_class;
    Class_ Str_class;
    
    // 类型检查相关方法
    Symbol type_check_expression(Expression expr, Symbol current_class, 
                                SymbolTable<Symbol, Symbol> *&object_env, 
                                const char *filename);
    Symbol lub(Symbol type1, Symbol type2);
    bool is_subtype(Symbol child, Symbol parent);
    Class_ get_class(Symbol name);
    method_class* find_method(Symbol class_name, Symbol method_name);
    void check_method_override(Class_ cls);
    void semant_error(Class_ c);
    void semant_error(Class_ c, const char *msg);
    void semant_error(const char *filename, tree_node *t, const char *msg);
};

#endif
