#include <gtest/gtest.h>
#include <openssl/aes.h>
#define AES_BITS 128
#define MSG_LEN 128

// http://yuanshuilee.blog.163.com/blog/static/21769727520140942826137/
int aes_encrypt(char* in, char* key, char* out)
{
    if(!in || !key || !out) return 0;
    unsigned char iv[AES_BLOCK_SIZE];//加密的初始化向量
    for(int i=0; i<AES_BLOCK_SIZE; ++i)//iv一般设置为全0,可以设置其他，但是加密解密要一样就行
    	iv[i]=0;
    AES_KEY aes;
    if(AES_set_encrypt_key((unsigned char*)key, 128, &aes) < 0){
        return 0;
    }
    int len=strlen(in);
    AES_cbc_encrypt((unsigned char*)in, (unsigned char*)out, len, &aes, iv, AES_ENCRYPT);
    return 1;
}

TEST(openssl, aes) {
	char sourceStringTemp[MSG_LEN];
    char dstStringTemp[MSG_LEN];
    memset((char*)sourceStringTemp, 0 ,MSG_LEN);
    memset((char*)dstStringTemp, 0 ,MSG_LEN);
    strcpy((char*)sourceStringTemp, "123456789 123456789 123456789 12a");

    char key[AES_BLOCK_SIZE];
    for(int i = 0; i < 16; i++) {//可自由设置密钥
        key[i] = 32 + i;
    }

    ASSERT_EQ(aes_encrypt(sourceStringTemp,key,dstStringTemp), 1);

    printf("enc %lu:",strlen((char*)dstStringTemp));
    for(int i= 0; dstStringTemp[i]; i+=1){
        printf("%x",(unsigned char)dstStringTemp[i]);
    }
    printf("\n");
}


TEST(simple, simple) {
  ASSERT_EQ(1 + 1, 2);
}
