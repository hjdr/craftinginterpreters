#ifndef cvox_object_h
#define cvox_object_h

#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "table.h"

#define IS_OBJ(value)           ((value).type == VAL_OBJ)

#define IS_BOOL(value)          ((value).type == VAL_BOOL)
#define IS_BOUND_METHOD(value)  isObjType(value, OBJ_BOUND_METHOD)
#define IS_CLASS(value)         isObjType(value, OBJ_CLASS)
#define IS_CLOSURE(value)       isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value)      isObjType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value)      isObjType(value, OBJ_INSTANCE)
#define IS_NATIVE(value)        isObjType(value, OBJ_NATIVE)
#define IS_NIL(value)           ((value).type == VAL_NIL)
#define IS_NUMBER(value)        isObjType(value, OBJ_NUMBER)
#define IS_STRING(value)        isObjType(value, OBJ_STRING)

#define AS_OBJ(value)           ((value).as.obj)

#define AS_BOOL(value)          ((value).as.boolean)
#define AS_BOUND_METHOD(value)  ((ObjBoundMethod*)AS_OBJ(value))
#define AS_CLASS(value)         ((ObjClass*)AS_OBJ(value))
#define AS_CLOSURE(value)       ((ObjClosure*)AS_OBJ(value))
#define AS_FUNCTION(value)      ((ObjFunction*)AS_OBJ(value))
#define AS_INSTANCE(value)      ((ObjInstance*)AS_OBJ(value))
#define AS_NUMBER(val)          (((ObjNumber*)AS_OBJ(val))->value)
#define AS_NATIVE(value)        (((ObjNative*)AS_OBJ(value))->function)
#define AS_STRING(value)        ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)       (((ObjString*)AS_OBJ(value))->chars)

// TODO: Remove.
#define OBJ_TYPE(value) ((value).as.obj->type)

#define BOOL_VAL(val) ((Value){ VAL_BOOL, { .boolean = val } })
#define NIL_VAL       ((Value){ VAL_NIL, { .obj = NULL } })
#define OBJ_VAL(obj)  objectToValue((Obj*)(obj))

// TODO: Unboxed numbers?

typedef enum {
  OBJ_BOUND_METHOD,
  OBJ_CLASS,
  OBJ_CLOSURE,
  OBJ_FUNCTION,
  OBJ_INSTANCE,
  OBJ_NATIVE,
  OBJ_NUMBER,
  OBJ_STRING,
  OBJ_UPVALUE
} ObjType;

struct sObj {
  ObjType type;
  // TODO: Stuff into low bit of next?
  bool isDark;
  
  struct sObj* next;
};

typedef struct {
  Value* values;
  int capacity;
  int count;
} ValueArray;

typedef struct {
  Obj obj;
  bool value;
} ObjBool;

typedef struct {
  Obj obj;

  int codeCount;
  int codeCapacity;
  uint8_t* code;
  int arity;
  int upvalueCount;
  ValueArray constants;

  // Debug information.
  ObjString* name;
  int* codeLines;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
  Obj obj;
  NativeFn function;
} ObjNative;

typedef struct {
  Obj obj;
  double value;
} ObjNumber;

struct sObjString {
  Obj obj;
  int length;
  const char* chars;
  uint32_t hash;
};

typedef struct sUpvalue {
  Obj obj;
  
  // Pointer to the variable this upvalue is referencing.
  Value* value;
  
  // If the upvalue is closed (i.e. the local variable it was pointing too has
  // been popped off the stack) then the closed-over value is hoisted out of
  // the stack into here. [value] is then be changed to point to this.
  Value closed;
  
  // Open upvalues are stored in a linked list. This points to the next one in
  // that list.
  struct sUpvalue* next;
} ObjUpvalue;

typedef struct {
  Obj obj;
  ObjFunction* function;
  ObjUpvalue** upvalues;
} ObjClosure;

typedef struct sObjClass {
  Obj obj;
  ObjString* name;
  struct sObjClass* superclass;
  Table methods;
} ObjClass;

typedef struct {
  Obj obj;
  ObjClass* klass;
  Table fields;
} ObjInstance;

typedef struct {
  Obj obj;
  Value receiver;
  ObjClosure* method;
} ObjBoundMethod;

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
ObjClass* newClass(ObjString* name, ObjClass* superclass);
ObjClosure* newClosure(ObjFunction* function);
ObjFunction* newFunction();
ObjInstance* newInstance(ObjClass* klass);
ObjNative* newNative(NativeFn function);
ObjNumber* newNumber(double value);
ObjString* takeString(const char* chars, int length);
ObjString* copyString(const char* chars, int length);
ObjUpvalue* newUpvalue(Value* slot);

bool valuesEqual(Value a, Value b);

void initArray(ValueArray* array);
void growArray(ValueArray* array);
void freeArray(ValueArray* array);

// TODO: "obj" -> "object".
// Returns true if [value] is an object of type [type]. Do not call this
// directly, instead use the [IS___] macro for the type in question.
static inline bool isObjType(Value value, ObjType type)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

// Converts the raw object pointer [obj] to a [Value].
static inline Value objectToValue(Obj* obj)
{
  Value value;
  value.type = VAL_OBJ;
  value.as.obj = obj;
  return value;
}

#endif
