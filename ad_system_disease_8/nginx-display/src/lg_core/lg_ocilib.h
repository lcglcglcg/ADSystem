
#ifndef __LG_OCILIB_H__
#define __LG_OCILIB_H__

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct {

	void *conn;
	void *statement;
	void *resultset;

    } lg_ocilib_t;

    // init
    extern int lg_ocilib_init();

    // conn
    extern int lg_ocilib_conn(lg_ocilib_t *p, char *db, char *user, char *pass);
    extern void lg_ocilib_free(lg_ocilib_t *p);

    // find
    extern int lg_ocilib_find(lg_ocilib_t *p, char *sql);
    extern int lg_ocilib_fetch_next(lg_ocilib_t *p);
    extern int lg_ocilib_find_one(lg_ocilib_t *p, char *sql);

    // execute
    extern int lg_ocilib_runing(lg_ocilib_t *p, char *sql);
    extern int lg_ocilib_execute(lg_ocilib_t *p, char *sql);
    extern void lg_ocilib_commit(lg_ocilib_t *p);
    extern void lg_ocilib_rollback(lg_ocilib_t *p);

    // get
    extern int lg_ocilib_get_number(lg_ocilib_t *p, int index);
    extern double lg_ocilib_get_double(lg_ocilib_t *p, int index);
    extern char *lg_ocilib_get_string(lg_ocilib_t *p, int index);

#ifdef __cplusplus
}
#endif

#endif



