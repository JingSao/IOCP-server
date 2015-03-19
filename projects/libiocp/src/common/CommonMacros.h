#ifndef _COMMON_MACROS_H_
#define _COMMON_MACROS_H_

#include <string.h>

#define DECLEAR_NO_COPY_CLASS(_class_)                  \
    private:                                            \
        _class_(const _class_ &) = delete;              \
        _class_(_class_ &&) = delete;                   \
        _class_ &operator=(const _class_ &) = delete;   \
        _class_ &operator=(_class_ &&) = delete

// synthesize
#define LAZY_SYNTHESIZE(varType, varName, funName)                  \
    protected: varType varName;                                     \
    public: varType get##funName(void) const { return varName; }    \
    public: void set##funName(varType var) { varName = var; }

#define LAZY_SYNTHESIZE_PASS_BY_REF(varType, varName, funName)          \
    protected: varType varName;                                         \
    public: const varType &get##funName(void) const { return varName; } \
    public: void set##funName(const varType &var) { varName = var; }

#define LAZY_SYNTHESIZE_READONLY(varType, varName, funName)     \
    protected: varType varName;                                 \
    public: varType get##funName(void) const { return varName; }

#define LAZY_SYNTHESIZE_READONLY_PASS_BY_REF(varType, varName, funName) \
    protected: varType varName;                                         \
    public: const varType &get##funName(void) const { return varName; }

#define LAZY_SYNTHESIZE_BOOL(varName, funName)                  \
    protected: bool varName;                                    \
    public: bool is##funName(void) const { return varName; }    \
    public: void set##funName(bool var) { varName = var; }

#define LAZY_SYNTHESIZE_BOOL_READONLY(varName, funName)     \
    protected: bool varName;                                \
    public: bool is##funName(void) const { return varName; }

#define LAZY_SYNTHESIZE_C_ARRAY(varType, num, varName, funName)         \
    protected: varType varName[num];                                    \
    public: const varType *get##funName(void) const { return varName; } \
    public: void set##funName(const varType *var) { memmove(varName, var, sizeof(varType [num]); }

#define LAZY_SYNTHESIZE_C_ARRAY_READONLY(varType, num, varName, funName)    \
    protected: varType varName[num];                                        \
    public: const varType *get##funName(void) const { return varName; }

// property
#define LAZY_PROPERTY(varType, varName, funName)    \
    protected: varType varName;                     \
    public: varType get##funName(void);             \
    public: void set##funName(varType var);

#define LAZY_PROPERTY_PASS_BY_REF(varType, varName, funName)    \
    protected: varType varName;                                 \
    public: const varType &get##funName(void);                  \
    public: void set##funName(const varType &var);

#define LAZY_PROPERTY_READONLY(varType, varName, funName)   \
    protected: varType varName;                             \
    public: varType get##funName(void);

#define LAZY_PROPERTY_READONLY_PASS_BY_REF(varType, varName, funName)   \
    protected: varType varName;                                         \
    public: const varType &get##funName(void);

#define LAZY_PROPERTY_BOOL(varName, funName)    \
    protected: bool varName;                    \
    public: bool is##funName(void);             \
    public: void set##funName(bool var);

#define LAZY_PROPERTY_BOOL_READONLY(varName, funName)   \
    protected: bool varName;                            \
    public: bool is##funName(void);

#define LAZY_PROPERTY_C_ARRAY(varType, num, varName, funName)   \
    protected: varType varName[num];                            \
    public: const varType *get##funName(void);                  \
    public: void set##funName(const varType *var);

#define LAZY_PROPERTY_C_ARRAY_READONLY(varType, num, varName, funName)  \
    protected: varType varName[num];                                    \
    public: const varType *get##funName(void);

#endif
