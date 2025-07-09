#include "hstring.h"
#include "hthread.h"
#include "htime.h"
#include "ifconfig.h"
#include "hdir.h"
#include <thread>

#include "hobjectpool.h"
#include <unistd.h>

int test_hstring() {
    /*
    char str1[] = "a1B2*C3d4==";
    char str2[] = "a1B2*C3d4==";
    printf("strupper %s\n", strupper(str1));
    printf("strlower %s\n", strlower(str2));
    char str3[] = "abcdefg";
    printf("strreverse %s\n", strreverse(str3));

    char str4[] = "123456789";
    printf("strstartswith=%d\nstrendswith=%d\nstrcontains=%d\n",
        (int)strstartswith(str4, "123"),
        (int)strendswith(str4, "789"),
        (int)strcontains(str4, "456"));
    */

    std::string str5 = asprintf("%s%d", "hello", 5);
    printf("asprintf %s\n", str5.c_str());

    std::string str6("123,456,789");
    StringList strlist = split(str6, ',');
    printf("split %s\n", str6.c_str());
    for (auto& str : strlist) {
        printf("%s\n", str.c_str());
    }

    std::string str7("user=admin&pswd=123456");
    hv::KeyValue kv = splitKV(str7, '&', '=');
    for (auto& pair : kv) {
        printf("%s=%s\n", pair.first.c_str(), pair.second.c_str());
    }

    std::string str8("<stdio.h>");
    std::string str9 = trim_pairs(str8);
    printf("trim_pairs %s\n", str9.c_str());

    std::string str10("<title>{{title}}</title>");
    std::string str11 = replace(str10, "{{title}}", "Home");
    printf("replace %s\n", str11.c_str());

    std::string filepath("/mnt/share/image/test.jpg");
    std::string base = basename(filepath);
    std::string dir = dirname(filepath);
    std::string file = filename(filepath);
    std::string suffix = suffixname(filepath);
    printf("filepath %s\n", filepath.c_str());
    printf("basename %s\n", base.c_str());
    printf("dirname %s\n", dir.c_str());
    printf("filename %s\n", file.c_str());
    printf("suffixname %s\n", suffix.c_str());

    return 0;
}

HTHREAD_ROUTINE(test_thread1) {
    int cnt = 10;
    while (cnt-- > 0) {
        printf("tid=%ld time=%llums\n", hv_gettid(), gettimeofday_ms());
        hv_delay(100);
    }
    return 0;
}

class TestThread2 : public HThread {
protected:
    virtual void run() {
        int cnt = 10;
        while (cnt-- > 0) {
            printf("tid=%ld time=%llums\n", hv_gettid(), gettimeofday_ms());
            hv_delay(100);
        }
    }
};

class TestThread3 : public HThread {
protected:
    virtual bool doPrepare() {
        printf("doPrepare\n");
        return true;
    }

    virtual void doTask() { printf("tid=%ld time=%llums\n", hv_gettid(), gettimeofday_ms()); }

    virtual bool doFinish() {
        printf("doFinish\n");
        return true;
    }
};

int test_hthread() {
    printf("c-style hthread_create\n");
    hthread_t thread1 = hthread_create(test_thread1, NULL);
    hthread_join(thread1);

    printf("cpp-style override HThread::run\n");
    TestThread2 thread2;
    thread2.start();
    thread2.stop();

    printf("cpp-style override HThread::doTask\n");
    TestThread3 thread3;
    thread3.setSleepPolicy(HThread::SLEEP_UNTIL, 100);
    thread3.start();
    sleep(1);
    thread3.stop();

    return 0;
}

int test_ifconfig() {
    std::vector<ifconfig_t> ifcs;
    ifconfig(ifcs);
    for (auto& item : ifcs) {
        printf("%s\nip: %s\nmask: %s\nbroadcast: %s\nmac: %s\n\n", item.name, item.ip, item.mask, item.broadcast, item.mac);
    }
    return 0;
}

int test_hdir() {
    const char* dir = ".";
    std::list<hdir_t> dirs;
    listdir(dir, dirs);
    for (auto& item : dirs) {
        printf("%c%c%c%c%c%c%c%c%c%c\t", item.type, item.mode & 0400 ? 'r' : '-', item.mode & 0200 ? 'w' : '-', item.mode & 0100 ? 'x' : '-',
               item.mode & 0040 ? 'r' : '-', item.mode & 0020 ? 'w' : '-', item.mode & 0010 ? 'x' : '-', item.mode & 0004 ? 'r' : '-',
               item.mode & 0002 ? 'w' : '-', item.mode & 0001 ? 'x' : '-');
        float hsize;
        if (item.size < 1024) {
            printf("%lu\t", item.size);
        }
        else if ((hsize = item.size / 1024.0f) < 1024.0f) {
            printf("%.1fK\t", hsize);
        }
        else if ((hsize /= 1024.0f) < 1024.0f) {
            printf("%.1fM\t", hsize);
        }
        else {
            hsize /= 1024.0f;
            printf("%.1fG\t", hsize);
        }
        struct tm* tm = localtime(&item.mtime);
        printf("%04d-%02d-%02d %02d:%02d:%02d\t", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        printf("%s%s\n", item.name, item.type == 'd' ? "/" : "");
    }
    return 0;
}

class Task {
public:
    Task() { printf("Task()\n"); }
    ~Task() { printf("~Task()\n"); }

    void Do() {
        printf("%p start do...\n", this);
        msleep(500);
        printf("%p end do\n", this);
    }
};

HObjectPool<Task> task_pool(1, 5);

void task_thread(int id) {
    printf("thread %d run...\n", id);
    HPoolObject<Task> pTask(task_pool);
    if (pTask) {
        pTask->Do();
    }
    else {
        printf("No available task in pool\n");
    }
    printf("thread %d exit\n", id);
}

int test_objectpool() {
    for (int i = 0; i < 2; ++i) {
        new std::thread(task_thread, i);
    }
    msleep(500);
    for (int i = 10; i < 12; ++i) {
        new std::thread(task_thread, i);
    }
    msleep(500);
    return 0;
}

#include "hthread.h"
#include "hmutex.h"

#define THREAD_NUM 3
std::mutex g_mutex;

HTHREAD_ROUTINE(test_synchronized) {
    synchronized(g_mutex) {
        hv_delay(100);
        printf("tid=%ld time=%llus\n", hv_gettid(), (unsigned long long)time(NULL));
    }
    return 0;
}

int test_sync() {
    hthread_t threads[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; ++i) {
        threads[i] = hthread_create(test_synchronized, NULL);
    }
    for (int i = 0; i < THREAD_NUM; ++i) {
        hthread_join(threads[i]);
    }
    return 0;
}

#include "base64.h"
#include "sha1.h"

int test_base64(char* data) {
    if (data == NULL) return -1;
    char magic[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    unsigned char out[1024] = {0};
    unsigned char digest[20] = {0};
    SHA1_CTX ctx;
    SHA1Init(&ctx);
    SHA1Update(&ctx, (unsigned char*)magic, strlen(magic));
    SHA1Final(digest, &ctx);
    base64_encode((unsigned char*)data, strlen(data), (char*)out);
    base64_decode((char*)out, strlen((char*)out), (unsigned char*)out);
    return 0;
}

#include "hv.h"
#include "hmain.h"
#include "iniparser.h"

typedef struct conf_ctx_s {
    IniParser* parser;
    int loglevel;
    int worker_processes;
    int worker_threads;
    int port;
} conf_ctx_t;
conf_ctx_t g_conf_ctx;

inline void conf_ctx_init(conf_ctx_t* ctx) {
    ctx->parser = new IniParser;
    ctx->loglevel = LOG_LEVEL_DEBUG;
    ctx->worker_processes = 0;
    ctx->worker_threads = 0;
    ctx->port = 0;
}

static void print_version();
static int parse_confile(const char* confile);

void print_version() {
    printf("%s version %s\n", g_main_ctx.program_name, hv_compile_version());
}

int parse_confile(const char* confile) {
    int ret = g_conf_ctx.parser->LoadFromFile(confile);
    if (ret != 0) {
        printf("Load confile [%s] failed: %d\n", confile, ret);
        exit(-40);
    }

    // logfile
    string str = g_conf_ctx.parser->GetValue("logfile");
    if (!str.empty()) {
        strncpy(g_main_ctx.logfile, str.c_str(), sizeof(g_main_ctx.logfile));
    }
    hlog_set_file(g_main_ctx.logfile);
    // loglevel
    str = g_conf_ctx.parser->GetValue("loglevel");
    if (!str.empty()) {
        hlog_set_level_by_str(str.c_str());
    }
    // log_filesize
    str = g_conf_ctx.parser->GetValue("log_filesize");
    if (!str.empty()) {
        hlog_set_max_filesize_by_str(str.c_str());
    }
    // log_remain_days
    str = g_conf_ctx.parser->GetValue("log_remain_days");
    if (!str.empty()) {
        hlog_set_remain_days(atoi(str.c_str()));
    }
    // log_fsync
    str = g_conf_ctx.parser->GetValue("log_fsync");
    if (!str.empty()) {
        logger_enable_fsync(hlog, getboolean(str.c_str()));
    }
    // first log here
    hlogi("%s version: %s", g_main_ctx.program_name, hv_compile_version());
    hlog_fsync();

    // worker_processes
    int worker_processes = 0;
    str = g_conf_ctx.parser->GetValue("worker_processes");
    if (str.size() != 0) {
        if (strcmp(str.c_str(), "auto") == 0) {
            worker_processes = get_ncpu();
            hlogd("worker_processes=ncpu=%d", worker_processes);
        }
        else {
            worker_processes = atoi(str.c_str());
        }
    }
    g_conf_ctx.worker_processes = LIMIT(0, worker_processes, MAXNUM_WORKER_PROCESSES);
    // worker_threads
    int worker_threads = g_conf_ctx.parser->Get<int>("worker_threads");
    g_conf_ctx.worker_threads = LIMIT(0, worker_threads, 16);

    // port
    int port = 0;
    const char* szPort = get_arg("p");
    if (szPort) {
        port = atoi(szPort);
    }
    if (port == 0) {
        port = g_conf_ctx.parser->Get<int>("port");
    }
    if (port == 0) {
        printf("Please config listen port!\n");
        exit(-10);
    }
    g_conf_ctx.port = port;

    hlogi("parse_confile('%s') OK", confile);
    return 0;
}

int test_parse_confile() {
    print_version();
    conf_ctx_init(&g_conf_ctx);
    parse_confile("./etc/hmain_test.conf");
    return 0;
}
