
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <threads.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <math.h>

int global_array[100] = {-1};
volatile int *p = 0;
const char *ptr;
int *gl_ptr;

typedef struct tagPOINT
{
    long x;
    long y;
} POINT;

// CWE-123: Write-what-where Condition  https://cwe.mitre.org/data/definitions/123.html
void test_CWE_123(char *data)
{
    printf("=====================test_CWE_123\n");
    char *buff1 = (char *)malloc(256);
    char *buff2 = (char *)malloc(256);
    strcpy(buff1, data);
    free(buff2);
    free(buff1);
}

// CWE-170: Improper Null Termination   https://cwe.mitre.org/data/definitions/170.html
void test_CWE_170(char *data, int size)
{
    printf("=====================test_CWE_170\n");
    if (strlen(data) > 1024) // data 并没有NULL字符结尾，strlen 就会出错
    {
        printf("string is too long!\n");
    }
}

// CWE-786: Access of Memory Location Before Start of Buffer   https://cwe.mitre.org/data/definitions/786.html
// CWE-124: Buffer Underwrite ('Buffer Underflow')
char *trimTrailingWhitespace(char *strMessage, int length)
{
    char *retMessage;
    // char *message = malloc(sizeof(char) * (length + 1));

    // copy input string to a temporary string
    char message[length + 1];
    int index;
    for (index = 0; index < length; index++)
    {
        message[index] = strMessage[index];
    }
    message[index] = '\0';

    // trim trailing whitespace
    int len = index - 1;
    while (isspace(message[len]))
    {
        message[len] = '\0';
        len--;
    }

    // return string without trailing whitespace
    retMessage = message;
    return retMessage;
}

void test_CWE_786(char *data, int size)
{
    printf("=====================test_CWE_786\n");
    trimTrailingWhitespace(data, size);
}

// CWE-788: Access of Memory Location After End of Buffer   https://cwe.mitre.org/data/definitions/788.html         易恒可测
char *copy_input(char *user_supplied_string, int size)
{
    int i, dst_index;
    char *dst_buf = (char *)malloc(4 * sizeof(char) * size);
    if (1024 * 10 <= size)
    {
        printf("user string too long, die evil hacker!");
        return dst_buf;
    }
    dst_index = 0;
    for (i = 0; i < size; i++)
    {
        if ('&' == user_supplied_string[i])
        {
            dst_buf[dst_index++] = '&';
            dst_buf[dst_index++] = 'a';
            dst_buf[dst_index++] = 'm';
            dst_buf[dst_index++] = 'p';
            dst_buf[dst_index++] = ';';
        }
        else if ('<' == user_supplied_string[i])
        {

            /* encode to &lt; */
        }
        else
            dst_buf[dst_index++] = user_supplied_string[i];
    }
    return dst_buf;
}

void test_CWE_788(char *data, int size)
{
    printf("=====================test_CWE_788\n");
    char *copystr = copy_input(data, size);
    free(copystr);
}

// CWE-805: Buffer Access with Incorrect Length Value   https://cwe.mitre.org/data/definitions/805.html     易恒可测
#define LOG_INPUT_SIZE 40

// saves the file name to a log file
int outputFilenameToLog(char *filename, int length)
{
    int success;

    // buffer with size set to maximum size for input to log file
    char buf[LOG_INPUT_SIZE];

    // copy filename to buffer
    strncpy(buf, filename, length);

    // save to log file
    // success = saveToLogFile(buf);
    printf("log file = %s\n", buf);

    return success;
}

void test_CWE_805(char *data, int size)
{
    printf("=====================test_CWE_805\n");
    int len = outputFilenameToLog(data, size);
}

// CWE-825: Expired Pointer Dereference    https://cwe.mitre.org/data/definitions/825.html
// CWE-672: Operation on a Resource after Expiration or Release
void test_CWE_825(char *data, int size)
{
    printf("=====================test_CWE_825\n");
    int abrt = 0;
    int len = 128;
    char *ptr = (char *)malloc(len);
    if (size < 5)
    {
        abrt = 1;
        free(ptr);
    }
    else
    {
        len = (size > len) ? len : size;
        memcpy(ptr, data, len);
    }
    if (abrt == 1)
    {
        // logError("operation aborted before commit", ptr);
        printf("operation aborted before commit, data=%s\n", ptr);
        return;
    }
    free(ptr);
}

// CWE-129: Improper Validation of Array Index                  https://cwe.mitre.org/data/definitions/129.html
// CWE-839: Numeric Range Comparison Without Minimum Check      https://cwe.mitre.org/data/definitions/839.html
// CWE-125: Out-of-bounds Read                                  https://cwe.mitre.org/data/definitions/125.html
// 这个例子包含了三个 CWE                                                                                               易恒可测

char getValueFromArray(char *array, int len, int index)
{

    char value;
    // check that the array index is less than the maximum
    // length of the array
    if (index < len)
    {
        // get the value at the specified index of the array
        value = array[index];
    }
    // if array index is invalid then output error message
    // and return value indicating error
    else
    {
        printf("Value is: %d\n", array[index]);
        value = -1;
    }

    return value;
}
/*
However, this method only verifies that the given array index is less than the maximum length of the array but does not check for the minimum value (CWE-839).
This will allow a negative value to be accepted as the input array index, which will result in a out of bounds read (CWE-125) and may allow access to sensitive memory.
The input array index should be checked to verify that is within the maximum and minimum range required for the array (CWE-129).
In this example the if statement should be modified to include a minimum range check, as shown below.
*/
void test_CWE_129(char *data, int size)
{
    printf("=====================test_CWE_129\n");

    char value = getValueFromArray(data, size, size - 10); // 当size-10 小于0时，会发生错误
    printf("the read out char is %c\n", value);
}

// https://cwe.mitre.org/data/definitions/252.html
// CWE-252: Unchecked Return Value
// CWE-195: Signed to Unsigned Conversion Error
// CWE-787: Out-of-bounds Write
// CWE-788: Access of Memory Location After End of Buffer

int returnChunkSize(char *data, int size)
{
    /* if chunk info is valid, return the size of usable memory,
     * else, return -1 to indicate an error
     */
    if (size >= 5)
    {
        return size;
    }
    else
    {
        return -1;
    }
}

void test_CWE_252(char *data, int size)
{
    printf("=====================test_CWE_252\n");
    char *destBuf;
    destBuf = malloc(size + 1);
    memcpy(destBuf, data, (returnChunkSize(data, size) - 1));
    printf("destBuf=%s\n", destBuf);
    free(destBuf);
}

// CWE-390: Detection of Error Condition Without Action     https://cwe.mitre.org/data/definitions/390.html

void test_CWE_390(char *data, int size)
{
    printf("=====================test_CWE_390\n");
    char *foo = malloc(sizeof(char)); // the next line checks to see if malloc failed
    if (foo == NULL)
    {
        // We do nothing so we just ignore the error.
    }
    free(foo);
}

// CWE-404: Improper Resource Shutdown or Release           https://cwe.mitre.org/data/definitions/404.html
// CWE-772: Missing Release of Resource after Effective Lifetime
// CWE-775: Missing Release of File Descriptor or Handle after Effective Lifetime
int decodeFile(char *fName)
{
    int BUF_SZ = 32;
    char buf[BUF_SZ];
    FILE *f = fopen(fName, "r");
    if (!f)
    {
        printf("cannot open %s\n", fName);
        // 没有关闭文件，直接返回
        return -1;
    }
    else
    {
        // while (fgets(buf, BUF_SZ, f))
        // {
        //     if (!checkChecksum(buf))
        //     {
        //         return -1;
        //     }
        //     else
        //     {
        //         decodeBlock(buf);
        //     }
        // }
    }
    fclose(f);
    return 0;
}

void test_CWE_404(char *data, int size)
{
    printf("=====================test_CWE_404\n");
    decodeFile("test.c");
}

// CWE-401: Missing Release of Memory after Effective Lifetime      https://cwe.mitre.org/data/definitions/401.html
// The following C function leaks a block of allocated memory if the call to read() does not return the expected number of bytes:       易恒可测

char *getBlock(int fd)
{
    int BLOCK_SIZE = 16;
    char *buf = (char *)malloc(BLOCK_SIZE);
    if (!buf)
    {
        return NULL;
    }
    if (read(fd, buf, BLOCK_SIZE) != BLOCK_SIZE)
    {
        // 没有释放buf
        return NULL;
    }
    return buf;
}

void test_CWE_401(char *data, int size)
{
    printf("=====================test_CWE_401\n");
    char *buff = getBlock(32);
    if (buff != NULL)
    {
        free(buff);
    }
}

// CWE-476: NULL Pointer Dereference            易恒可测
// CWE-252, 119
static void validate_addr_form(char *v)
{
    // naive, simplistic validation
    if (strspn(v, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-0123456789.") < strlen(v))
    {
        fprintf(stderr, "hostname contains invalid characters");
        exit(1);
    }
}

void host_lookup(char *user_supplied_addr)
{
    struct hostent *hp;
    in_addr_t *addr;
    char hostname[64];
    in_addr_t inet_addr(const char *cp);

    /*routine that ensures user_supplied_addr is in the right format for conversion */

    validate_addr_form(user_supplied_addr);
    addr = inet_addr(user_supplied_addr);
    hp = gethostbyaddr(addr, sizeof(struct in_addr), AF_INET);
    strcpy(hostname, hp->h_name);
}

void test_CWE_476(char *data, int size)
{
    printf("=====================test_CWE_476\n");
    host_lookup("192.168.5.2");
}

// CWE-477: Use of Obsolete Function        https://cwe.mitre.org/data/definitions/477.html
void test_CWE_477(char *data, int size)
{
    printf("=====================test_CWE_477\n");
    char *pwdline;
    getpw(1, pwdline);
}

// CWE-478: Missing Default Case in Multiple Condition Expression   https://cwe.mitre.org/data/definitions/478.html
void test_CWE_478(char *data, int size)
{
    printf("=====================test_CWE_478\n");
    switch (data[0])
    {
    case 0:
        printf("check failed!\n");
        exit(0);
        // Break never reached because of exit()
        break;
    case 1:
        printf("check passed.\n");
        break;
    }
}

// CWE-480: Use of Incorrect Operator       https://cwe.mitre.org/data/definitions/480.html
int isValid(int value)
{
    if (value = 100) // 应该是 ==
    {
        printf("Value is valid\n");
        return (1);
    }
    printf("Value is not valid\n");
    return (0);
}

void test_CWE_480(char *data, int size)
{
    printf("=====================test_CWE_480\n");
    isValid(data[0]);
}

// CWE-484: Omitted Break Statement in Switch   https://cwe.mitre.org/data/definitions/484.html
void printMessage(int month)
{
    switch (month)
    {
    case 1:
        printf("January");
    case 2:
        printf("February");
    case 3:
        printf("March");
    case 4:
        printf("April");
    case 5:
        printf("May");
    case 6:
        printf("June");
    case 7:
        printf("July");
    case 8:
        printf("August");
    case 9:
        printf("September");
    case 10:
        printf("October");
    case 11:
        printf("November");
    case 12:
        printf("December");
    }
    printf(" is a great month");
}

void test_CWE_484(char *data, int size)
{
    printf("=====================test_CWE_484\n");
    printMessage(data[0]);
}

// CWE-561: Dead Code   https://cwe.mitre.org/data/definitions/561.html
void test_CWE_561(char *data, int size)
{
    printf("=====================test_CWE_561\n");
    char s = NULL;
    if (s == NULL)
    {
        s = "Y";
        return;
    }

    if (s != NULL)
    {
        printf("the dead code here!\n");
    }
}

// CWE-562: Return of Stack Variable Address    https://cwe.mitre.org/data/definitions/562.html
char *getName()
{
    char name[128];
    sprintf(name, "tester");
    return name;
}

void test_CWE_562(char *data, int size)
{
    printf("=====================test_CWE_562\n");
    printf("Get stack variable address: %s\n", getName());
}

// CWE-570: Expression is Always False          https://cwe.mitre.org/data/definitions/570.html
#define BIT_READ 0x0001                   // 00000001
#define BIT_WRITE 0x0010                  // 00010000
unsigned int mask = BIT_READ & BIT_WRITE; /* intended to use "|" */
// using "&", mask = 00000000
// using "|", mask = 00010001
// determine if user has read and write access
int hasReadWriteAccess(unsigned int userMask)
{
    // if the userMask has read and write bits set
    // then return 1 (true)
    if (userMask & mask)
    {
        return 1;
    }
    // otherwise return 0 (false)
    return 0;
}

void test_CWE_570(char *data, int size)
{
    printf("=====================test_CWE_570\n");
    hasReadWriteAccess(data[0]);
}

// CWE-571: Expression is Always True          https://cwe.mitre.org/data/definitions/571.html
int alwaysTrue(unsigned int userMask)
{
    // if the userMask has read and write bits set
    // then return 1 (true)
    if (userMask & mask)
    {
        return 0;
    }
    // otherwise return 1 (true)
    return 1;
}

void test_CWE_571(char *data, int size)
{
    printf("=====================test_CWE_571\n");
    alwaysTrue(data[0]);
}

// CWE-606: Unchecked Input for Loop Condition      https://cwe.mitre.org/data/definitions/606.html
void iterate(unsigned int n)
{
    unsigned int i;
    for (i = 0; i < n; i++)
    {
        printf("iteration, i=%d\n", i);
    }
}
void test_CWE_606(char *data, int size)
{
    printf("=====================test_CWE_606\n");
    iterate(data[0]);
}

// CWE-366: Race Condition within a Thread          https://cwe.mitre.org/data/definitions/366.html
int foo = 0;
int storenum(int num)
{
    static int counter = 0;
    counter++;
    if (num > foo)
        foo = num;
    return foo;
}

void test_CWE_366(char *data, int size)
{
    printf("=====================test_CWE_366\n");
    foo = storenum(data[0]);
}

// CWE-667: Improper Locking                        https://cwe.mitre.org/data/definitions/667.html
pthread_mutex_t mutex;
void *threadFunction(void *arg)
{
    pthread_mutex_lock(&mutex);
    // if (0 != result)         //不检测是否锁成功
    //     return;
    /* access shared resource */
    foo = foo++;
    usleep(100);
    pthread_mutex_unlock(&mutex);
}

void test_CWE_667(char *data, int size)
{
    printf("=====================test_CWE_667\n");
    pthread_t id1;
    pthread_t id2;
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&id1, NULL, threadFunction, NULL);
    pthread_create(&id2, NULL, threadFunction, NULL);
    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
    pthread_mutex_destroy(&mutex);
    printf("foo = %d\n", foo);
}

// CWE-820: Missing Synchronization             https://cwe.mitre.org/data/definitions/820.html
static void printMsg(char *string)
{
    char *word;
    int counter;
    for (word = string; counter = *word++;)
    {
        putc(counter, stdout);
        fflush(stdout);
        /* Make timing window a little larger... */
        // sleep(1);
    }
}

int test_CWE_820(char *data, int size)
{
    printf("=====================test_CWE_820\n");
    pid_t pid;
    pid = fork();
    if (pid == -1)
    {
        exit(-2);
    }
    else if (pid == 0)
    {
        printMsg("child\n");
    }
    else
    {
        printMsg("PARENT\n");
    }
    exit(0);
}

// CWE-665: Improper Initialization             https://cwe.mitre.org/data/definitions/665.html
int test_CWE_665(char *data, int size)
{
    printf("=====================test_CWE_665\n");
    char str[20];
    strcat(str, "hello world");
    printf("%s", str);
}

// CWE-456: Missing Initialization of a Variable        https://cwe.mitre.org/data/definitions/456.html
void parse_data(char *untrusted_input)
{
    int m, n, error;
    error = sscanf(untrusted_input, "%d:%d", &m, &n);
    if (EOF == error)
    {
        printf("Did not specify integer value. Die evil hacker!\n");
    }
    /* proceed assuming n and m are initialized correctly */
    m = m + n;
}

int test_CWE_456(char *data, int size)
{
    printf("=====================test_CWE_456\n");
    parse_data(data);
}

// CWE-457: Use of Uninitialized Variable           https://cwe.mitre.org/data/definitions/457.html
int test_CWE_457(char *data, int size)
{
    printf("=====================test_CWE_457\n");
    char *test_string;
    if (size > 50)
    {
        test_string = "Hello World!";
    }
    printf("%s", test_string);
}

// CWE-194: Unexpected Sign Extension               https://cwe.mitre.org/data/definitions/194.html
int GetUntrustedInt()
{
    return (0x0000FFFF);
}

void test_CWE_194(char *data, int size)
{
    printf("=====================test_CWE_194\n");
    char path[256];
    char input[256] = "pathname";
    int i;
    short s;
    unsigned int sz;

    i = GetUntrustedInt();
    s = i;
    /* s is -1 so it passes the safety check - CWE-697 */
    if (s > 256)
    {
        printf("go away!\n");
    }

    /* s is sign-extended and saved in sz */
    sz = s;

    /* output: i=65535, s=-1, sz=4294967295 - your mileage may vary */
    printf("i=%d, s=%d, sz=%u\n", i, s, sz);

    // input = GetUserInput("Enter pathname:");
    /* strncpy interprets s as unsigned int, so it's treated as MAX_INT
    (CWE-195), enabling buffer overflow (CWE-119) */
    strncpy(path, input, s);
    path[255] = '\0'; /* don't want CWE-170 */
    printf("Path is: %s\n", path);
}

// CWE-197: Numeric Truncation Error                https://cwe.mitre.org/data/definitions/197.html
void test_CWE_197(char *data, int size)
{
    printf("=====================test_CWE_197\n");
    int intPrimitive;
    short shortPrimitive;
    // intPrimitive = (int)(~((int)0) ^ (1 << (sizeof(int) * 8 - 1)));
    intPrimitive = INTMAX_MAX;
    shortPrimitive = intPrimitive;
    printf("Int MAXINT: %d\nShort MAXINT: %d\n", intPrimitive, shortPrimitive);
}

// CWE-131: Incorrect Calculation of Buffer Size    https://cwe.mitre.org/data/definitions/131.html
void test_CWE_131(char *data, int size)
{
    printf("=====================test_CWE_131\n");
    int *id_sequence;
    /* Allocate space for an array of three ids. */
    id_sequence = (int *)malloc(3); // The malloc() call could have used '3*sizeof(int)' as the value for the size parameter in order to allocate the correct amount of space required to store the three ints.
    if (id_sequence == NULL)
        exit(1);
    /* Populate the id array. */
    id_sequence[0] = 13579;
    id_sequence[1] = 24680;
    id_sequence[2] = 97531;
}

// CWE-732: Incorrect Permission Assignment for Critical Resource   https://cwe.mitre.org/data/definitions/732.html
void test_CWE_732(char *data, int size)
{
    printf("=====================test_CWE_732\n");
    umask(0);
    FILE *out;
    /* Ignore link following (CWE-59) for brevity */
    out = fopen("hello.out", "w");
    if (out)
    {
        fprintf(out, "hello world!\n");
        fclose(out);
    }
}

// CWE-783: Operator Precedence Logic Error     https://cwe.mitre.org/data/definitions/783.html
int AuthenticateUser(char *username, char *password)
{
    if (username != NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int validateUser(char *username, char *password)
{
    int isUser = 0;
    // call method to authenticate username and password
    // if authentication fails then return failure otherwise return success
    if (isUser = AuthenticateUser(username, password) == 0) // 缺少括号
    {
        return isUser;
    }
    else
    {
        isUser = 1;
    }
    return isUser;
}

void test_CWE_783(char *data, int size)
{
    printf("=====================test_CWE_783\n");
    validateUser(data, data);
}

// CWE-789: Memory Allocation with Excessive Size Value         https://cwe.mitre.org/data/definitions/789.html
void test_CWE_789(char *data, int size)
{
    printf("=====================test_CWE_789\n");
    int a = 5, b = 6;
    size_t len = a - b;
    char buf[len]; // Just blows up the stack
    printf("buf:%s\n", buf);
}

// CWE-798: Use of Hard-coded Credentials                       https://cwe.mitre.org/data/definitions/798.html
// CWE-259: Use of Hard-coded Password
// CWE-321: Use of Hard-coded Cryptographic Key
int verifyAdmin(char *password)
{
    if (strcmp(password, "68af404b513073584c4b6f22b6c63e6b"))
    {
        printf("Incorrect Password!\n");
        return (0);
    }
    printf("Entering Diagnostic Mode...\n");
    return (1);
}

void test_CWE_798(char *data, int size)
{
    printf("=====================test_CWE_798\n");
    verifyAdmin(data);
}

// CWE-833: Deadlock                                            https://cwe.mitre.org/data/definitions/833.html
pthread_mutex_t s_mutex_a;
pthread_mutex_t s_mutex_b;
pthread_barrier_t s_barrier;
void lock()
{
    pthread_mutex_lock(&s_mutex_b);
    {
        pthread_barrier_wait(&s_barrier); // 10行
        pthread_mutex_lock(&s_mutex_a);
        pthread_mutex_unlock(&s_mutex_a);
    }

    pthread_mutex_unlock(&s_mutex_b);
}

static void *thread_routine(void *arg)
{
    pthread_mutex_lock(&s_mutex_a);
    {
        pthread_barrier_wait(&s_barrier); // 21行
        pthread_mutex_lock(&s_mutex_b);
        pthread_mutex_unlock(&s_mutex_b);
    }
    pthread_mutex_unlock(&s_mutex_a);
}

int test_CWE_833(char *data, int size)
{
    printf("=====================test_CWE_833\n");
    pthread_t tid;
    pthread_mutex_init(&s_mutex_a, 0);
    pthread_mutex_init(&s_mutex_b, 0);
    pthread_barrier_init(&s_barrier, 0, 2);
    pthread_create(&tid, 0, &thread_routine, 0);
    lock();
    pthread_join(tid, 0);
    pthread_cancel(tid);
    pthread_barrier_destroy(&s_barrier);
    pthread_mutex_destroy(&s_mutex_a);
    pthread_mutex_destroy(&s_mutex_b);
    return 0;
}

// CWE-835: Loop with Unreachable Exit Condition ('Infinite Loop')
// However, the while loop will become an infinite loop if the rateSold input parameter has a value of zero since the inventoryCount will never fall below the minimumCount.
// In this case the input parameter should be validated to ensure that a value of zero does not cause an infinite loop,as in the following code.
int isReorderNeeded(char *bookISBN, int rateSold)
{
    int isReorder = 0;
    int minimumCount = 10;
    int days = 0;
    // get inventory count for book
    // int inventoryCount = inventory.getIventoryCount(bookISBN);
    int inventoryCount = 20;
    // find number of days until inventory count reaches minimum
    while (inventoryCount > minimumCount)
    {
        inventoryCount = inventoryCount - rateSold;
        days++;
    }
    // if number of days within reorder timeframe
    // set reorder return boolean to true
    if (days > 0 && days < 5) //
    {
        isReorder = 1;
    }
    return isReorder;
}

void test_CWE_835(char *data, int size)
{
    printf("=====================test_CWE_835\n");
    isReorderNeeded("new book", 0);
}

// CWE-908: Use of Uninitialized Resource
void test_CWE_908(char *data, int size)
{
    printf("=====================test_CWE_908\n");
    char str[20];
    strcat(str, "hello world");
    printf("%s", str);
}

// CWE-1041: Use of Redundant Code
void test_CWE_1041(char *data, int size)
{
    printf("=====================test_CWE_1041\n");
    double s = 10.0;
    double r = 1.0;
    double pi = 3.14159;
    double surface_area;
    if (r > 0.0)
    {
        // complex math equations
        surface_area = pi * r * s + pi * pow(r, 2);
    }
    if (r > 1.0)
    {
        // a complex set of math
        surface_area = pi * r * s + pi * pow(r, 2);
    }
}

// CWE-480: Use of Incorrect Operator
#define SIZE 50
int *tos, *p1, stack[SIZE];

void push(int i)
{
    p1++;
    if (p1 == (tos + SIZE))
    {
        // Print stack overflow error message and exit
        printf("Print stack overflow error message and exit\n");
        return;
    }
    *p1 == i;
}

int pop(void)
{
    if (p1 == tos)
    {
        // Print stack underflow error message and exit
        printf("Print stack underflow error message and exit\n");
        return -1;
    }
    p1--;
    return *(p1 + 1);
}

void test_CWE_480_case2(char *data, int size)
{
    printf("=====================test_CWE_480_case2\n");
    // initialize tos and p1 to point to the top of stack
    tos = stack;
    p1 = stack;
    // code to add and remove items from stack
    push(1);
    pop();
}

// CWE-570	表达式总为False
// CWE-571	表达式总为True
// void test_CWE_571(char *data, int size)
// {
//     if (1 == 1)
//     {
//         printf("its always true!\n");
//     }
//     if (1 == 2)
//     {
//         printf("its always false!\n");
//     }
// }

// CWE-787: Out-of-bounds Write	            https://cwe.mitre.org/data/definitions/787.html
void outofBoundWrite()
{
    int id_sequence[3];
    id_sequence[3] = 456;
}

void test_CWE_787()
{
    outofBoundWrite();
}

// CWE-125: Out-of-bounds Read		        https://cwe.mitre.org/data/definitions/125.html
int outofBoundRead()
{
    int id_sequence[3] = {1, 1, 1};
    int value = id_sequence[3];
    printf("outofBoundRead, value=%d\n", value);
    return 0;
}

void test_CWE_125()
{
    outofBoundRead();
}

// CWE-191: Integer Underflow		        https://cwe.mitre.org/data/definitions/191.html
int integerUnderflow()
{
    int i;
    i = -2147483648;
    i = i - 1;
    printf("integerUnderflow, %d\n", i);
}

void test_CWE_191()
{
    integerUnderflow();
}

// CWE-416: Use After Free			        https://cwe.mitre.org/data/definitions/416.html
char useAfterFree()
{
    int buff_size = 128;
    char *ptr = (char *)malloc(buff_size);
    memset(ptr, 0, buff_size);
    strcpy(ptr, "abc");
    free(ptr);
    printf("use after free, print the string: %s\n", ptr);
    strcpy(ptr, "use after free!");
    *ptr = 'A';
    *(ptr + 1) = 'A';
    return *ptr;
}

void test_CWE_416()
{
    useAfterFree();
}

// CWE-190: Integer Overflow or Wraparound	https://cwe.mitre.org/data/definitions/190.html
void integerOverflow()
{
    int i;
    i = 2147483648;
    i = i + 1;
    printf("integerOverflow = %d\n", i);
    int b = INT_MAX + 10;
    printf("b+1 = %d\n", b);
}

void test_CWE_190()
{
    integerOverflow();
}

// CWE-120: Buffer Copy without Checking Size 	https://cwe.mitre.org/data/definitions/120.html
void bufferCopyWithoutCheckingSize()
{
    char buf[128] = "Very_very_long_string";
    char dest[5] = "";
    strcpy(dest, buf);
    printf("dest=%s\n", dest);
}

void test_CWE_120()
{
    bufferCopyWithoutCheckingSize();
}

// CWE-369: Divide By Zero			            https://cwe.mitre.org/data/definitions/369.html
int dividebyZero()
{
    int iz = 0;
    float fz = 0.0;
    iz = 100 / iz;
    fz = 100.0 / fz;
    printf("dividebyZero: int %d, float %f\n", iz, fz);
    return 0;
}

void test_CWE_369()
{
    dividebyZero();
}

// CWE-468: Incorrect Pointer Scaling	        https://cwe.mitre.org/data/definitions/468.html
void incorrectPointerScaling()
{
    int *p = (int *)malloc(sizeof(int) * 5);
    char *second_char = (char *)(p + 1);
    printf("second_char=%s\n", second_char);
}

void test_CWE_468()
{
    incorrectPointerScaling();
}

// CWE-681: Incorrect Conversion between Numeric Types	https://cwe.mitre.org/data/definitions/681.html
unsigned int incorrectConversion()
{
    int amount = -1;
    float ft = 3.14;
    int i = (int)ft;
    printf("i=%d \n", i);
    return amount;
}

void test_CWE_681()
{
    unsigned int ret = incorrectConversion();
}

// CWE-415 双重释放		                            http://cve.scap.org.cn/cwe/415
void doubleFree()
{
    int size = 5;
    char *buf = (char *)malloc(size);
    memset(buf, 0, size);
    *buf = 'A';
    *(buf + 1) = 'B';
    *(buf + 2) = 'C';
    printf("buf: %s\n", buf);
    free(buf);
    if (1 == 1)
    {
        free(buf);
    }
}

void test_CWE_415()
{
    doubleFree();
}

// CWE-590 释放并不在堆上的内存	                    http://cve.scap.org.cn/cwe/590
void freeMemNotonHeap()
{
    POINT plist[128];
    for (int i = 0; i < sizeof(plist) / sizeof(plist[0]); i++)
    {
        plist[0].x = 100;
        plist[0].y = 100;
    }
    free(plist);
}

void test_CWE_590()
{
    freeMemNotonHeap();
}

// CWE-193 Off-by-one错误	                        http://cve.scap.org.cn/cwe/193
void offbyOne()
{
    char firstname[10];
    char lastname[10];
    char fullname[20];
    fullname[0] = '\0';
    memset(firstname, 'A', 10);
    memset(lastname, 'B', 10);
    strncat(fullname, firstname, 10);
    strncat(fullname, lastname, 10);
}

void test_CWE_193()
{
    offbyOne();
}

// CWE-464 对数据结构哨兵域的增加	                    http://cve.scap.org.cn/cwe/464
void additionofDataStructureSentinel()
{
    char *foo;
    foo = malloc(sizeof(char) * 5);
    foo[0] = 'a';
    foo[1] = 'b';
    foo[2] = '\0';
    foo[3] = 'd';
    foo[4] = '\0';
    printf("%c %c %c %c %c \n", foo[0], foo[1], foo[2], foo[3], foo[4]);
    printf("%s\n", foo);
    free(foo);
}

void test_CWE_464()
{
    additionofDataStructureSentinel();
}

// CWE-88 参数注入或修改	                            http://cve.scap.org.cn/cwe/88
void argumentInjection(char *data, int size)
{
    char cmd[256] = "/usr/bin/cat ";
    if (size > 1)
    {
        data[size - 1] = '\0';
        int copy_len = sizeof(cmd) - strlen(cmd) - 1;
        if (strlen(data) < copy_len)
        {
            strcat(cmd, data);
            system(cmd);
        }
    }
}

void test_CWE_88(char *data, int size)
{
    argumentInjection(data, size);
}

// CWE-482 错误将赋值符号写成比较符号	                http://cve.scap.org.cn/cwe/482
void comparingInsteadofAssigning()
{
    int p, i;
    i = 100;
    p = 1;
    p++;
    p == i;
    printf("comparingInsteadofAssigning, p=%d \n", p);
}

void test_CWE_482()
{
    comparingInsteadofAssigning();
}

// stack-buffer-underflow
void stackBufferUnderflow()
{
    int cnt = 100;
    int *p = &cnt - 32;
    for (int i = 0; i < 32; i++)
    {
        *(p + i) = 0;
    }
}

// memory leak
void memLeak()
{

    int *p = malloc(7);
    memset(p, 0, 7);
    p = 10; // The memory is leaked here.

    int size = 1024;
    char *buff = (char *)malloc(size);
    memset(buff, 0, size);
    strcpy(buff, "test memory leak...");
}

// array index underflow
void arrayIndexUnderflow()
{
    char buf[2];
    int index = -1;
    printf(buf[index]);
}

// CWE-467: Use of sizeof() on a Pointer Type        https://cwe.mitre.org/data/definitions/467.html
void useofSizeof()
{
    // 该例子取至上飞用例中刻意埋下的错误
    typedef struct list_node_s
    {
        struct list_node_s *next; /* Next node in the list. */
        void *data;               /* Data for this node. */
    } list_node_t;
    list_node_t *node = NULL;
    list_node_t *node1 = NULL;
    node = (list_node_t *)malloc(sizeof(node1));
    node->data = (void *)malloc(sizeof(void *) * 1024);
    strcpy(node->data, "use of sizeof");
    node->next = NULL;
    free(node->data);
    free(node);
}

void test_CWE_467()
{
    useofSizeof();
}

// Modify string literal         https://riptutorial.com/c/example/6644/modify-string-literal
void modifyStringLiteral()
{
    char *p = "hello world";
    p[0] = 'H';
}

// global_buffer_over_flow
void globalBufferOverflow()
{
    printf(global_array[101]); // BOOM
}

// use after scope
void useAfterScope()
{
    {
        int x = 0;
        p = &x;
    }
    *p = 5;
}

void func2()
{
    int a = 23;
    gl_ptr = &a;
}

void func1()
{
    func2();
}
// Pointer to object with ended lifetime, test 3
void pointer2ObjectWithEndedLifetime()
{
    func1();
    printf("gl_ptr = %d \n", *gl_ptr);
}

// AddressSanitizerExampleUseAfterReturn     https://github.com/google/sanitizers/wiki/AddressSanitizerExampleUseAfterReturn
void useAfterReturn()
{
    int local[100] = {6};
    gl_ptr = &local[0];
}

// Conversion between two pointer types
void conversionBetweenTwoPointerTtypes()
{
    uint8_t ip = 9;
    uint8_t *p1 = &ip;
    uint32_t *p2;

    p2 = (uint32_t *)p1; /* undefined behavior , possible incompatible alignment */
    printf("p2 = %d\n", *p2);
}

// Function call throught pointer
char *(*fp)();
void functionCallThroughtPointer()
{
    const char *cr;
    fp = strchr;
    cr = fp('e', " Undefined ");
    printf("%s\n", cr);
}

// Pointer addition/subtraction,
void pointerSubtraction()
{
    char ub[18] = " UndefinedBehavior ";
    char *ptr1 = ub - 1;  /* undefined behavior */
    char *ptr4 = ub + 19; /* undefined behavior */
    printf("(ub -1) = %s\n", ptr1);
    printf("(ub + 19) = %s\n", ptr4);
}

// Pointer addition/subtraction,
void pointerAddition()
{
    char ub[18] = " UndefinedBehavior ";
    char *ptr = ub + 18; /* pointing to just beyond */
    char var = *ptr;     /* undefined behavior */
    printf(" var = %c\n", var);
}

// ArrayOutofBoundRead
void arrayOutofBoundRead()
{
    int arr[4][5] = {{11, 12, 13, 14, 15}, {21, 22, 23, 24, 25}, {31, 32, 33, 34, 35}, {41, 42, 43, 44, 45}};
    int b = arr[0][7]; /* undefined behavior */
    printf("%d\n", b);
}

// Subtracting two pointers
void substractingTwoPointers()
{
    int arr[5] = {1, 2, 3, 4, 5};
    int *ptr1, *ptr2;
    ptr1 = &arr[0];
    ptr2 = &arr[2];
    ptr1 = ptr2 - ptr1; /* undefined behavior */
    printf(" ptr1 = %d \n", *ptr1);
}

// Copying overlapping memory
void copyingOverlappingMemory()
{
    char str[27] = " This is undefined behavior ";

    memcpy(str + 7, str, 10);
    printf(" After memcpy : %s\n", str);
}

// Pointer used after free
void pointUsedAfterFree()
{
    char str[19] = " tutorial ";
    char ftr[19] = " aftertut ";
    int bufsize = strlen(str) + 1;
    char *buf = (char *)malloc(bufsize);
    if (!buf)
    {
        return;
    }
    free(buf);
    strcpy(buf, ftr); /* undefined behavior */
    printf(" buf = %s\n", buf);
}

// Library function with invalid value
void libFunctionWithInvalidValue()
{
    int val;
    char str[20];
    strcpy(str, " 10348593 ");
    val = atoi(str);
    printf(" String =%s, Int =%d\n", str, val);
    strcpy(str, " something else ");
    val = atoi(str);
    printf(" String = %s, Int =%d\n", str, val);
    /* undefined section */
    char *ptr = NULL;
    val = atoi(ptr);
}

// Bit shifting - If an expression is shifted by a negative number or by an amount greater than or equal to the width of the promoted expression, the behavior is undefined.
// export AFL_USE_UBSAN=1, 才能发现bug，但是bug详情不能显示
void bitShiftingWithNegativeNumber()
{
    int x = 5 << -3;
    int y = 5 >> -3;

    printf("x= %d , y = %d\n", x, y);
}

// Size expression in an array declaration
// If the size expression in an array declaration is not a constant expression and evaluates at program execution time to a non-positive value, the behavior is undefined
int func(int n)
{
    int arr[n]; /* undefined behavior */
    return (sizeof(arr));
}

void sizeExpression()
{
    printf(" func (0) = %d\n", func(0));
    printf(" func ( -10) = %d\n", func(-10));
}

int a = 0;

int Function1(void *ignore)
{
    a = 1;
    return 0;
}

// Data race             https://riptutorial.com/c/example/2622/data-race
int dataRace()
{
    thrd_t id1;
    thrd_create(&id1, Function1, NULL);
    int b = a;
    a = 2;
    thrd_join(id1, NULL);
}

// Dereferencing a null pointer  https://riptutorial.com/c/example/1230/dereferencing-a-null-pointer
void dereferNullPointer()
{
    int *pointer = NULL;
    int value = *pointer; /* Dereferencing happens here */
}

// Modifying any object more than once between two sequence points   https://riptutorial.com/c/example/1231/modifying-any-object-more-than-once-between-two-sequence-points
void modifyObjTwice()
{
    int i = 42;
    i = i++; /* Assignment changes variable, post-increment as well */
    int a = i++ + i--;
    printf("%d %d\n", i++, i++); /* commas as separator of function arguments are not comma-operators */
}
