#include "data.h"
#include "csapp.h"

BLOB *blob_create(char *content, size_t size){

}

BLOB *blob_ref(BLOB *bp, char *why){

}

void blob_unref(BLOB *bp, char *why){

}

int blob_compare(BLOB *bp1, BLOB *bp2){

}

int blob_hash(BLOB *bp){

}

KEY *key_create(BLOB *bp){

}

void key_dispose(KEY *kp){

}

int key_compare(KEY *kp1, KEY *kp2){

}

VERSION *version_create(TRANSACTION *tp, BLOB *bp){

}

void version_dispose(VERSION *vp){
	
}
