
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

int rsa_public_encrypt(const char *path, char *dest, char *src)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "r")))
	return -1;

    RSA *rsa = NULL;
    if (!(rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL))) {

	fclose(fp);
	return -1;
    }

    int size = RSA_public_encrypt(RSA_size(rsa), 
	    (unsigned char *)src, 
	    (unsigned char *)dest, 
	    rsa, 
	    RSA_NO_PADDING);
    if (size == -1) {
	RSA_free(rsa);
	fclose(fp);
	return -1;
    }

    RSA_free(rsa);
    fclose(fp);
    return size;
}

int rsa_public_decrypt(const char *path, char *dest, char *src)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "r")))
	return -1;

    RSA *rsa = NULL;
    if (!(rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL))) {
	return -1;

	fclose(fp);
	return -1;
    }

    int size = RSA_public_decrypt(RSA_size(rsa), 
	    (unsigned char *)src, 
	    (unsigned char *)dest, 
	    rsa, 
	    RSA_NO_PADDING);
    if (size == -1) {
	RSA_free(rsa);
	fclose(fp);
	return -1;
    }

    RSA_free(rsa);
    fclose(fp);
    return size;
}

int rsa_private_encrypt(const char *path, char *dest, char *src)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "r")))
	return -1;

    RSA *rsa = NULL;
    if (!(rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL))) {
	return -1;

	fclose(fp);
	return -1;
    }

    int size = RSA_private_encrypt(RSA_size(rsa), 
	    (unsigned char *)src, 
	    (unsigned char *)dest, 
	    rsa, 
	    RSA_NO_PADDING);
    if (size == -1) {

	RSA_free(rsa);
	fclose(fp);
	return -1;
    }

    RSA_free(rsa);
    fclose(fp);
    return size;
}

int rsa_private_decrypt(const char *path, char *dest, char *src)
{

    FILE *fp = NULL;
    if (!(fp = fopen(path, "r")))
	return -1;

    RSA *rsa = NULL;
    if (!(rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL))) {

	fclose(fp);
	return -1;
    }

    int size = RSA_private_decrypt(RSA_size(rsa), 
	    (unsigned char *)src, 
	    (unsigned char *)dest, 
	    rsa, 
	    RSA_NO_PADDING);
    if (size == -1) {
	RSA_free(rsa);
	fclose(fp);
	return -1;
    }

    RSA_free(rsa);
    fclose(fp);
    return size;
}

int rsa_main(void)
{

    char str1[1024] = "{\"a\":\"abc123\",\"b\":123}";
    char str2[1024] = {0};
    char str3[1024] = {0};

    rsa_private_encrypt("/make/nginx-1.7.12/rsa.key", str2, str1);
    rsa_public_decrypt("/make/nginx-1.7.12/rsa_pub.key", str3, str2);

    fprintf(stdout, "[1]%s\n", str1);
    fprintf(stdout, "[2]%s\n", str3);
    return 0;
}



