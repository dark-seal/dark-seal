#include "headers.h"

char* md5(char* data) {
	DWORD cb_hash = 16;
	int i = 0;
	HCRYPTPROV crypt_prov;
	HCRYPTHASH crypt_hash;
	BYTE hash[16];
	char *hex = "0123456789abcdef";
	char *str_hash;
	str_hash = malloc(sizeof(char) * 33);
	memset(str_hash, 0, 33);
	if (!CryptAcquireContext(&crypt_prov, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		return NULL;
	}
	if (!CryptCreateHash(crypt_prov, CALG_MD5, 0, 0, &crypt_hash)) {
		CryptReleaseContext(crypt_prov, 0);
		return NULL;
	}
	if (!CryptHashData(crypt_hash, (BYTE*)data, strlen(data), 0)) {
		CryptReleaseContext(crypt_prov, 0);
		CryptDestroyHash(crypt_hash);
		return NULL;
	}
	if (!CryptGetHashParam(crypt_hash, HP_HASHVAL, hash, &cb_hash, 0)) {
		CryptReleaseContext(crypt_prov, 0);
		CryptDestroyHash(crypt_hash);
		return NULL;
	}
	for (i = 0; i < cb_hash; i++) {
		str_hash[i * 2] = hex[hash[i] >> 4];
		str_hash[(i * 2) + 1] = hex[hash[i] & 0xF];
	}
	CryptReleaseContext(crypt_prov, 0);
	CryptDestroyHash(crypt_hash);

	return str_hash;
}

void random_string(unsigned char *buffer, int len) {

	srand(time(NULL));
	static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

	for (int i = 0; i < len; i++) {
		buffer[i] = alphanum[rand() % (sizeof(alphanum)-1)];
	}
}

uint8_t *generate_aes_key(char *string) {

	uint8_t *key = malloc(sizeof(uint8_t) * 32);
	char *md5_hash = NULL;
	md5_hash = md5(string);

	for(int i=0; i < 32; i++) {
		key[i] = md5_hash[i];
	}

	return key;
}

char *string2hex(BUFFER *buffer) {

	int j = 0;
	char *hex = malloc(sizeof(char) * (2 * buffer->size + 1));

	for(int i = 0; i < buffer->size; i++) {
		sprintf(hex + j, "%02x", buffer->content[i] & 0xff);
		j+=2;
	}

	free(buffer->content);
	free(buffer);
	return hex;
}

char *hex2string(BUFFER *buffer) {

	char *string = malloc(sizeof(char) * (buffer->size/2+1));
	int j = 0;

	for (int i = 0; j < buffer->size; i++) {
		int val[1];
		sscanf(buffer->content + j, "%02x", val);
		string[i] = val[0];
		string[i+1] = '\0';
		j += 2;
	}

	string[buffer->size/2] = '\0';

	free(buffer->content);
	free(buffer);
	return string;
}

BUFFER *aes_encrypt(BUFFER *buffer, uint8_t *key) {

	struct AES_ctx ctx;

	uint8_t iv[16];
	random_string(iv, sizeof(iv));


	AES_init_ctx_iv(&ctx, key, iv);
	AES_CTR_xcrypt_buffer(&ctx, (uint8_t *)buffer->content, buffer->size);

	buffer->content = realloc(buffer->content, buffer->size+16 + 1);
	memcpy(buffer->content + buffer->size, iv, 16);
	buffer->size += 16;
	buffer->content[buffer->size] = '\0';

	BUFFER* encrypted = malloc(sizeof(BUFFER));
	encrypted->content = string2hex(buffer);
	encrypted->size = strlen(encrypted->content);

	return encrypted;
}

BUFFER *aes_decrypt(BUFFER *buffer, uint8_t *key) {

	struct AES_ctx ctx;

	uint8_t iv[16];

	BUFFER *decrypted = malloc(sizeof(BUFFER));
	decrypted->size = buffer->size/2;
	decrypted->content = hex2string(buffer);

	memcpy(iv, decrypted->content + decrypted->size - 16, 16);
	decrypted->content[decrypted->size - 16] = 0;
	decrypted->size -= 16;

	AES_init_ctx_iv(&ctx, key, iv);
	AES_CTR_xcrypt_buffer(&ctx, (uint8_t *)decrypted->content, decrypted->size);

	return decrypted;
}
