#include "data.h"
#include "csapp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

BLOB *blob_create(char *content, size_t size){
	BLOB *blob = (BLOB*)Malloc(sizeof(BLOB));
	if(blob == NULL) return NULL;

	//blob->content = strdup(content);//allocate the size of the content
	// blob->content = Malloc(size + 1);
	// if(blob->content == NULL) return NULL;
	// memcpy(blob->content, content, size);
	// blob->content[size] = '\0';

	blob->content = strdup(content);
	blob->content[size] = '\0';
	
	blob->size = strlen(blob->content);
	//blob->size = size;
	blob->refcnt = 1; //reference count initially is 1

	blob->prefix = Malloc(1);
	blob->prefix[0] = '\0';

	// blob->prefix = Malloc(size + 1);
	// if(blob->prefix == NULL) return NULL;
	// memcpy(blob->prefix, content, size);
	// blob->prefix[size] = '\0';

	//blob->prefix = content;

	int initRes = pthread_mutex_init(&blob->mutex, NULL); //initialize mutex
	if(initRes != 0){ //error
		Free(blob->content);
		Free(blob->prefix);
		Free(blob);
		return NULL;
	}

	return blob;

}

BLOB *blob_ref(BLOB *bp, char *why){
	if(bp != NULL){
		pthread_mutex_lock(&bp->mutex);
		bp->refcnt++;
		pthread_mutex_unlock(&bp->mutex);
	}

	return bp;
}

void blob_unref(BLOB *bp, char *why){
	if(bp != NULL){
		pthread_mutex_lock(&bp->mutex);
		bp->refcnt--;
		if(bp->refcnt == 0){
			pthread_mutex_unlock(&bp->mutex);
			Free(bp->content);
			Free(bp->prefix);
			Free(bp);
		} else{
			pthread_mutex_unlock(&bp->mutex);
		}
	}
	

}

int blob_compare(BLOB *bp1, BLOB *bp2){
	if(strcmp(bp1->content, bp2->content) == 0){
		return 0;
	}
	
	return -1;
}

int blob_hash(BLOB *bp){
	int hashBrown = 0;
	char* contentFrog = bp->content; //content is string field of BLOB struct

	while(*contentFrog != '\0'){
		hashBrown = (hashBrown * 31) + *contentFrog;
		contentFrog++;

	}

	return hashBrown;
}

KEY *key_create(BLOB *bp){
	KEY *key = Malloc(sizeof(KEY));
	if(key == NULL) return NULL;

	key->blob = blob_ref(bp, "Key created from blob and inherits caller ref");
	key->hash = blob_hash(bp);
	return key;
}

void key_dispose(KEY *kp){
	blob_unref(kp->blob, "Disposing a key");
	free(kp);

}

int key_compare(KEY *kp1, KEY *kp2){
	if(kp1->hash != kp2->hash) return -1;

	int blobRes = blob_compare(kp1->blob, kp2->blob);

	if(blobRes == -1) return -1;

	return 0;
}

VERSION *version_create(TRANSACTION *tp, BLOB *bp){
	VERSION *version = malloc(sizeof(VERSION));
	if(version == NULL) return NULL;

	version->creator = tp;
	//version->blob = blob_ref(bp, "Version created from this blob");
	version->blob = bp;
	version->next = NULL;
	version->prev = NULL;

	trans_ref(tp, "Increase ref count of transaction after creating Version");

	return version;
}

void version_dispose(VERSION *vp){
	trans_unref(vp->creator, "Disposing a version");
	blob_unref(vp->blob, "Disposing a version");
	Free(vp);

}
