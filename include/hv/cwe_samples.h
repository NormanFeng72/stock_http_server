extern "C"
{

    // CWE-561: Dead Code   https://cwe.mitre.org/data/definitions/561.html
    void test_CWE_561(char *data, int size);
    // CWE-788: Access of Memory Location After End of Buffer   https://cwe.mitre.org/data/definitions/788.html         易恒可测
    void test_CWE_788(char *data, int size);
    // CWE-476: NULL Pointer Dereference            易恒可测
    void test_CWE_476(char *data, int size);
    // CWE-129: Improper Validation of Array Index                  https://cwe.mitre.org/data/definitions/129.html
    void test_CWE_129(char *data, int size);
    // CWE-252: Unchecked Return Value
    void test_CWE_252(char *data, int size);
    // CWE-401: Missing Release of Memory after Effective Lifetime      https://cwe.mitre.org/data/definitions/401.html
    void test_CWE_401(char *data, int size);
    // CWE-805: Buffer Access with Incorrect Length Value   https://cwe.mitre.org/data/definitions/805.html     易恒可测
    void test_CWE_805(char *data, int size);
    // CWE-131: Incorrect Calculation of Buffer Size    https://cwe.mitre.org/data/definitions/131.html
    void test_CWE_131(char *data, int size);
    // CWE-667: Improper Locking                        https://cwe.mitre.org/data/definitions/667.html
    void test_CWE_667(char *data, int size);
    // CWE-366: Race Condition within a Thread          https://cwe.mitre.org/data/definitions/366.html
    void test_CWE_366(char *data, int size);
    // CWE-820: Missing Synchronization             https://cwe.mitre.org/data/definitions/820.html
    int test_CWE_820(char *data, int size);
    // CWE-833: Deadlock                                            https://cwe.mitre.org/data/definitions/833.html
    int test_CWE_833(char *data, int size);
    // CWE-835: Loop with Unreachable Exit Condition ('Infinite Loop')
    void test_CWE_835(char *data, int size);
    // CWE-606: Unchecked Input for Loop Condition      https://cwe.mitre.org/data/definitions/606.html
    void test_CWE_606(char *data, int size);
    // CWE-562: Return of Stack Variable Address    https://cwe.mitre.org/data/definitions/562.html
    void test_CWE_562(char *data, int size);
    // CWE-786: Access of Memory Location Before Start of Buffer   https://cwe.mitre.org/data/definitions/786.html
    void test_CWE_786(char *data, int size);
    // CWE-194: Unexpected Sign Extension               https://cwe.mitre.org/data/definitions/194.html
    void test_CWE_194(char *data, int size);
    // CWE-789: Memory Allocation with Excessive Size Value         https://cwe.mitre.org/data/definitions/789.html
    void test_CWE_789(char *data, int size);
    // 运算符优先级逻辑错误
    void test_CWE_783(char *data, int size);
    // // 解引用失效的指针
    // void test_CWE_805(char *data, int size);
    // 未处理检测到的错误
    void test_CWE_390(char *data, int size);
    // 资源关闭或释放不当
    void test_CWE_404(char *data, int size);
    // 使用废弃的函数
    void test_CWE_477(char *data, int size);
    // Switch语句块缺少Default语句
    void test_CWE_478(char *data, int size);
    // 运算符错误
    void test_CWE_480(char *data, int size);
    // 运算符错误
    void test_CWE_480_case2(char *data, int size);
    // Switch语句块缺少Break语句
    void test_CWE_484(char *data, int size);
    // 在任意地方写任意内容的条件    易恒 - 偶尔crash    read-memory-access      #6 0x5641ee885abf in test_CWE_123 bugs.c:26
    void test_CWE_123(char *data);
    // 表达式总为False
    void test_CWE_570(char *data, int size);
    // 表达式总为True
    void test_CWE_571(char *data, int size);
    // 初始化不当
    int test_CWE_665(char *data, int size);
    // 变量未初始化
    int test_CWE_456(char *data, int size);
    // 使用变量前未初始化
    int test_CWE_457(char *data, int size);
    // 数值截断错误
    void test_CWE_197(char *data, int size);
    // 关键资源的权限分配不正确
    void test_CWE_732(char *data, int size);
    // 字符串或数组终止不当          易恒 - 偶尔crash    heap-buffer-overflow    #1 0x55c3044d9b7f in test_CWE_170 bugs.c:35
    void test_CWE_170(char *data, int size);
    // 硬编码凭证
    void test_CWE_798(char *data, int size);
    // 使用未初始化的资源
    void test_CWE_908(char *data, int size);
    // 冗余代码（复制粘贴）
    void test_CWE_1041(char *data, int size);
    // CWE-787: Out-of-bounds Write	            https://cwe.mitre.org/data/definitions/787.html
    void test_CWE_787();
    // CWE-125: Out-of-bounds Read
    void test_CWE_125();
    // CWE-191: Integer Underflow
    void test_CWE_191();
    // CWE-416: Use After Free
    void test_CWE_416();
    // CWE-190: Integer Overflow or Wraparound
    void test_CWE_190();
    // CWE-120: Buffer Copy without Checking Size
    void test_CWE_120();
    // CWE-369: Divide By Zero
    void test_CWE_369();
    // CWE-468: Incorrect Pointer Scaling
    void test_CWE_468();
    // CWE-681: Incorrect Conversion between Numeric Types
    void test_CWE_681();
    // CWE-415 双重释放
    void test_CWE_415();
    // CWE-590 释放并不在堆上的内存
    void test_CWE_590();
    // CWE-193 Off-by-one错误
    void test_CWE_193();
    // CWE-464 对数据结构哨兵域的增加
    void test_CWE_464();
    // CWE-88 参数注入或修改
    void test_CWE_88(char *data, int size);
    // CWE-482 错误将赋值符号写成比较符号
    void test_CWE_482();
    // stack-buffer-underflow
    void stackBufferUnderflow();
    // memory leak
    void memLeak();
    // array index underflow
    void arrayIndexUnderflow();
    // CWE-467: Use of sizeof() on a Pointer Type
    void test_CWE_467();
    // Modify string literal         https://riptutorial.com/c/example/6644/modify-string-literal
    void modifyStringLiteral();
    // global_buffer_over_flow
    void globalBufferOverflow();
    // use after scope
    void useAfterScope();
    // Pointer to object with ended lifetime, test 3
    void pointer2ObjectWithEndedLifetime();
    // AddressSanitizerExampleUseAfterReturn     https://github.com/google/sanitizers/wiki/AddressSanitizerExampleUseAfterReturn
    void useAfterReturn();
    // Conversion between two pointer types
    void conversionBetweenTwoPointerTtypes();
    // Function call throught pointer
    void functionCallThroughtPointer();
    // Pointer addition/subtraction,
    void pointerSubtraction();
    // Pointer addition/subtraction,
    void pointerAddition();
    // ArrayOutofBoundRead
    void arrayOutofBoundRead();
    // Subtracting two pointers
    void substractingTwoPointers();
    // Copying overlapping memory
    void copyingOverlappingMemory();
    // Pointer used after free
    void pointUsedAfterFree();
    // Library function with invalid value
    void libFunctionWithInvalidValue();
    // Bit shifting - If an expression is shifted by a negative number or by an amount greater than or equal to the width of the promoted expression, the behavior is undefined.
    // export AFL_USE_UBSAN=1, 才能发现bug，但是bug详情不能显示
    void bitShiftingWithNegativeNumber();
    // Size expression in an array declaration
    void sizeExpression();
    // Data race             https://riptutorial.com/c/example/2622/data-race
    int dataRace();
    // Dereferencing a null pointer  https://riptutorial.com/c/example/1230/dereferencing-a-null-pointer
    void dereferNullPointer();
    // Modifying any object more than once between two sequence points   https://riptutorial.com/c/example/1231/modifying-any-object-more-than-once-between-two-sequence-points
    void modifyObjTwice();
}