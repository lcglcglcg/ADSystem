
#ifndef __LG_RSA_H__
#define __LG_RSA_H__


#ifdef __cplusplus
extern "C" { 
#endif

    extern int rsa_public_encrypt(const char *path, char *dest, char *src);
    extern int rsa_public_decrypt(const char *path, char *dest, char *src);
    extern int rsa_private_encrypt(const char *path, char *dest, char *src);
    extern int rsa_private_decrypt(const char *path, char *dest, char *src);

#ifdef __cplusplus
}
#endif
#endif


