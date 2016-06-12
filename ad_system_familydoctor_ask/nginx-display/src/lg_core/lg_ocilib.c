
#include <ocilib.h>
#include <ocilib_internal.h>
#include "lg_ocilib.h"

int lg_ocilib_init()
{

    setenv("ORACLE_HOME", "/oracle/10.2/", 1);
    if (setenv("NLS_LANG","AMERICAN_AMERICA.UTF8",1))
	return -1;

    if (!OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT | OCI_ENV_THREADED))
	return -1;

    return 0;
}

int lg_ocilib_conn(lg_ocilib_t *p, char *db, char *user, char *pass)
{

    p->conn = OCI_ConnectionCreate(db, user, pass, OCI_SESSION_DEFAULT);
    if (!p->conn) return -1;

    p->statement = OCI_StatementCreate(p->conn);
    if (!p->statement) {

	OCI_ConnectionFree(p->conn);
	return -1;
    }

    return 0;
}

void lg_ocilib_free(lg_ocilib_t *p)
{

    OCI_StatementFree(p->statement);
    OCI_ConnectionFree(p->conn);
}

int lg_ocilib_find(lg_ocilib_t *p, char *sql)
{

    if (!(OCI_ExecuteStmt(p->statement, sql)))
	return -1;

    p->resultset = OCI_GetResultset(p->statement);
    if (!p->resultset)
	return -1;

    return 0;
}

int lg_ocilib_fetch_next(lg_ocilib_t *p)
{
    return (OCI_FetchNext(p->resultset));
}

int lg_ocilib_find_one(lg_ocilib_t *p, char *sql)
{

    if (lg_ocilib_find(p, sql))
	return -1;

    if (!lg_ocilib_fetch_next(p))
	return -1;

    return 0;
}

int lg_ocilib_runing(lg_ocilib_t *p, char *sql)
{

    if (!(OCI_ExecuteStmt(p->statement, sql))) {

	OCI_Rollback(p->conn);
	return -1;
    }

    OCI_Commit(p->conn);
    return 0;
}

int lg_ocilib_execute(lg_ocilib_t *p, char *sql)
{

    if (!(OCI_ExecuteStmt(p->statement, sql))) 
	return -1;
    return 0;
}

void lg_ocilib_commit(lg_ocilib_t *p)
{
    OCI_Commit(p->conn);
}

void lg_ocilib_rollback(lg_ocilib_t *p)
{
    OCI_Rollback(p->conn);
}

int lg_ocilib_get_number(lg_ocilib_t *p, int index)
{
    return OCI_GetInt(p->resultset, index);
}

double lg_ocilib_get_double(lg_ocilib_t *p, int index)
{
    return OCI_GetDouble(p->resultset, index);
}

char *lg_ocilib_get_string(lg_ocilib_t *p, int index)
{
    char *str = (char *)OCI_GetString(p->resultset, index);
    if (!str) return "null";
    return str;
}





