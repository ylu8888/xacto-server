#include "csapp.h"
#include "transaction.h"
#include <pthread.h>

TRANSACTION trans_list;

void trans_init(void){
  
}

void trans_fini(void){
  
}

TRANSACTION *trans_create(void){
  
}

TRANSACTION *trans_ref(TRANSACTION *tp, char *why){
  
}

void trans_unref(TRANSACTION *tp, char *why){
  
}

void trans_add_dependency(TRANSACTION *tp, TRANSACTION *dtp){
  
}

TRANS_STATUS trans_commit(TRANSACTION *tp){
  
}

TRANS_STATUS trans_abort(TRANSACTION *tp){
  
}

TRANS_STATUS trans_get_status(TRANSACTION *tp){
  
}


void trans_show(TRANSACTION *tp){
  
}

void trans_show_all(void){
  
}

