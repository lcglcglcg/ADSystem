
#include <stdio.h>
#include "lg_redis.h"

int main()
{

    lg_redis_conn_t conn;
    if (lg_redis_conn(&conn, "127.0.0.1", 6379)) return -1;

    lg_redis_value_t *v = lg_redis_get_value(&conn, "lcg", 3);
    fprintf(stdout, "%s\n", v->str);

    return 0;
}




