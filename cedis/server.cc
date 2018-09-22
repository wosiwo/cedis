//
// Created by onceme on 2018/9/10.
//

#include "ty_server.h"
#include "kvdb.h"
#include <stdlib.h>  // exit
#include <string.h> //memset.h
#include <assert.h>

#define TEST_VERBOSE 1
#define TEST_EXIT exit(1)

#define TDB_SIZE SMALL
#define TDB_KEYSIZE 30
#define TDB_VALSIZE 31

#define ZEROLENVAL_KEY  "Zerolen Value's key%d"
#define ZEROLENKEY_VAL  "zero length key's value"
#define UPD_ZEROLENKEY_VAL  "Updated zerolength key'svalue"
#define ZLVSIZE 5

#define exit_if(r, ...) if(r) {printf(__VA_ARGS__); printf("error no: %d error msg %s\n", errno, strerror(errno)); exit(1);}




#include "../handy/handy.h"
using namespace handy;


char* g_name = NULL;
char *data[6];
char *resp;
kvdb_s* test_create_database(char* name){
    if (name == NULL){
        g_name = "./kv001.db";
    }else{
        g_name = name;
    }
    kvdb_s* kdb = create_kvdb(g_name, TDB_SIZE,
                              TDB_KEYSIZE, TDB_VALSIZE);
    assert(kdb != NULL);
    //printf("TEST: Succesfully created db\n");
//	if(TEST_VERBOSE) printf("Test step: %d completed\n", ++g_teststep);
    return kdb;
}


kvdb_s* test_load_database(char* name){
    if (name == NULL){
        g_name = "./kv001.db";
    }else{
        g_name = name;
    }
    kvdb_s* kdb = load_kvdb(g_name);
    assert(kdb != NULL);
    //printf("TEST: Succesfully loaded db\n");
//    if(TEST_VERBOSE) printf("Test step: %d completed\n", ++g_teststep);
    return kdb;
}

/**
 * 字符串分割
 * @param buf
 * @return
 */
char** split(char* buf,char *d){
    char *p;

    p = strtok(buf,d);


    data[0] = p;
    int i = 1;
    while(p)
    {
        p=strtok(NULL,d);
        if(p!= NULL){
            printf("\n n %lu i %d \n",sizeof(p),i);
            printf(" p %s \n",p);

            data[i] = p;
            i++;
        }
    }
    return data;
}
char* parseCmd(char* buf, kvdb_s* kdb){
    char *d = "\r\n";
//	printf("read2 %s \n", buf);

    char **ret = split(buf,d);
//	int paramNum = ret[0];
    char *cmd = ret[2];
    printf("\n cmd %s \n",cmd);
    printf("\n cmd n %lu \n", sizeof(cmd));
//	cmd = strtolower(cmd);
//	printf("\n cmd2 %s \n",cmd);

    //TODO 兼容linux与mac strcasecmp stricmp

    if (strcasecmp("get", cmd) == 0){
        printf("\n get \n");

        char *key = ret[4];
        printf("\n get key %s\n",key);

        char* val = get_kvdb(kdb, key);
        printf("\n get val %s\n",val);

        if(val!=NULL){
            int len = (int)strlen(val);
            printf("\n get val2 %s len %d \n",val,len);
            char resp2[100];
            sprintf(resp2, "$%d\r\n%s\r\n", len, val);
            printf("\n get resp %s\n",resp);
            resp = resp2;

        }else{
            resp = "$4\r\nfalse\r\n";
        }


        return resp;

    }
    if (strcasecmp("set", cmd) == 0){
        printf("\n set \n");

        char *key = ret[4];
        char *val = ret[6];
        add_kvdb(kdb, key, val); //int ret =
        return "+OK";
    }
    if (strcasecmp("hget", cmd) == 0){
        printf("\n hget \n");
    }
    if (strcasecmp("hset", cmd) == 0){
        printf("\n hset \n");
    }
    return "+Error";
}


int main(int argc, const char* argv[]) {

    char *kvname = "./kv002.db";
//	kvdb_s* kdb = test_create_database(kvname);
    kvdb_s* kdb = test_load_database(kvname);

    EventBase base;
    Signal::signal(SIGINT, [&]{ base.exit(); });
    TcpServerPtr svr = TcpServer::startServer(&base, "", 2099);
    exitif(svr == NULL, "start tcp server failed");
    svr->onConnRead([](const TcpConnPtr& con) {
        char* resp = parseCmd(con->getInput());
        con->send(resp);
    });
    base.loop();
}