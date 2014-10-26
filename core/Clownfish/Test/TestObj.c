/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>

#define CFISH_USE_SHORT_NAMES
#define TESTCFISH_USE_SHORT_NAMES

#include "charmony.h"

#include "Clownfish/Test/TestObj.h"

#include "Clownfish/String.h"
#include "Clownfish/Err.h"
#include "Clownfish/Test.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/Class.h"

TestObj*
TestObj_new() {
    return (TestObj*)Class_Make_Obj(TESTOBJ);
}

static Obj*
S_new_testobj() {
    StackString *class_name = SSTR_WRAP_UTF8("TestObj", 7);
    Obj *obj;
    Class *klass = Class_fetch_class((String*)class_name);
    if (!klass) {
        klass = Class_singleton((String*)class_name, OBJ);
    }
    obj = Class_Make_Obj(klass);
    return Obj_init(obj);
}

static void
test_refcounts(TestBatchRunner *runner) {
    Obj *obj = S_new_testobj();

    TEST_INT_EQ(runner, Obj_Get_RefCount(obj), 1,
                "Correct starting refcount");

    obj = Obj_Inc_RefCount(obj);
    TEST_INT_EQ(runner, Obj_Get_RefCount(obj), 2, "Inc_RefCount");

    Obj_Dec_RefCount(obj);
    TEST_INT_EQ(runner, Obj_Get_RefCount(obj), 1, "Dec_RefCount");

    DECREF(obj);
}

static void
test_To_String(TestBatchRunner *runner) {
    Obj *testobj = S_new_testobj();
    String *string = Obj_To_String(testobj);
    TEST_TRUE(runner, Str_Find_Utf8(string, "TestObj", 7) >= 0, "To_String");
    DECREF(string);
    DECREF(testobj);
}

static void
test_Equals(TestBatchRunner *runner) {
    Obj *testobj = S_new_testobj();
    Obj *other   = S_new_testobj();

    TEST_TRUE(runner, Obj_Equals(testobj, testobj),
              "Equals is true for the same object");
    TEST_FALSE(runner, Obj_Equals(testobj, other),
               "Distinct objects are not equal");

    DECREF(testobj);
    DECREF(other);
}

static void
test_Hash_Sum(TestBatchRunner *runner) {
    Obj *testobj = S_new_testobj();
    int64_t address64 = CHY_PTR_TO_I64(testobj);
    int32_t address32 = (int32_t)address64;
    TEST_TRUE(runner, (Obj_Hash_Sum(testobj) == address32),
              "Hash_Sum uses memory address");
    DECREF(testobj);
}

static void
test_Is_A(TestBatchRunner *runner) {
    String *string     = Str_new_from_trusted_utf8("", 0);
    Class  *str_class  = Str_Get_Class(string);
    String *class_name = Str_Get_Class_Name(string);

    TEST_TRUE(runner, Str_Is_A(string, STRING), "String Is_A String.");
    TEST_TRUE(runner, Str_Is_A(string, OBJ), "String Is_A Obj.");
    TEST_TRUE(runner, str_class == STRING, "Get_Class");
    TEST_TRUE(runner, Str_Equals(Class_Get_Name(STRING), (Obj*)class_name),
              "Get_Class_Name");

    DECREF(string);
}

static void
S_attempt_init(void *context) {
    Obj_init((Obj*)context);
}

static void
S_attempt_Clone(void *context) {
    Obj_Clone((Obj*)context);
}

static void
S_attempt_Compare_To(void *context) {
    Obj_Compare_To((Obj*)context, (Obj*)context);
}

static void
S_attempt_To_I64(void *context) {
    Obj_To_I64((Obj*)context);
}

static void
S_attempt_To_F64(void *context) {
    Obj_To_F64((Obj*)context);
}

static void
S_attempt_Mimic(void *context) {
    Obj_Mimic((Obj*)context, (Obj*)context);
}

static void
S_verify_abstract_error(TestBatchRunner *runner, Err_Attempt_t routine,
                        void *context, const char *name) {
    char message[100];
    sprintf(message, "%s() is abstract", name);
    Err *error = Err_trap(routine, context);
    TEST_TRUE(runner, error != NULL
              && Err_Is_A(error, ERR) 
              && Str_Find_Utf8(Err_Get_Mess(error), "bstract", 7) != -1,
              message);
    DECREF(error);
}

static void
test_abstract_routines(TestBatchRunner *runner) {
    Obj *blank = Class_Make_Obj(OBJ);
    S_verify_abstract_error(runner, S_attempt_init, blank, "init");

    Obj *obj = S_new_testobj();
    S_verify_abstract_error(runner, S_attempt_Clone,      obj, "Clone");
    S_verify_abstract_error(runner, S_attempt_Compare_To, obj, "Compare_To");
    S_verify_abstract_error(runner, S_attempt_To_I64,     obj, "To_I64");
    S_verify_abstract_error(runner, S_attempt_To_F64,     obj, "To_F64");
    S_verify_abstract_error(runner, S_attempt_Mimic,      obj, "Mimic");
    DECREF(obj);
}

void
TestObj_Run_IMP(TestObj *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 17);
    test_refcounts(runner);
    test_To_String(runner);
    test_Equals(runner);
    test_Hash_Sum(runner);
    test_Is_A(runner);
    test_abstract_routines(runner);
}

/*********************************************************************/

String*
AliasTestObj_Aliased_IMP(AliasTestObj *self) {
    UNUSED_VAR(self);
    return Str_newf("C");
}

String*
AliasTestObj_Call_Aliased_From_C_IMP(AliasTestObj *self) {
    UNUSED_VAR(self);
    return AliasTestObj_Aliased(self);
}

