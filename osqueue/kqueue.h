#include "chardev.h"
#include <linux/slab.h>

struct node{
	char[10] val;
	struct node* next;
};

typedef struct kqueue{
	struct node* head;
	struct node* tail;
	int length;
} kqueue;

static inline struct kqueue* kq_create(void){
	struct kqueue* kq;
	kq=(struct kqueue*) kmalloc(sizeof(struct kqueue), GFP_KERNEL);
	
	if(kq==NULL){
		printk("can't allocate memory\n");
		return NULL;
	}
	
	return kq;
}

static inline void kq_delete(struct kqueue *kq){
	struct node* curr = kq->head;
	int ii;
	for(ii=0; ii < (kq->length-1); ii++){

		kq->head=kq->head->next;
		kfree(curr);
		curr=kq->head;
	}
	
	if(kq->head==kq->tail){
		kfree(kq->head);
	}
	
	kfree(kq);
	
	return;
}

static inline int kq_enqueue(struct kqueue *kq, char *string[0]){

	struct node* new;
	new=(struct node*) kmalloc(sizeof(struct node),GFP_KERNEL);
	//struct node* tracker=kq->head;
	
	if(!new){
		printk("can't create node\n");
		return 0;
	}
	
	strcpy(new->val,value);
	
	if(kq->length==0){
		new->next=NULL;
		kq->head=new;
		kq->tail=new;
		kq->head->next=NULL;
		kq->tail->next=NULL;
	}
	
	else{
		kq->tail->next=new;
		kq->tail=new;
		kq->tail->next=NULL;
	}
	
	return 1;
}

static inline char[10] kq_dequeue(struct kqueue *kq){
	
	struct node* pop =kq->head;
	char[10] p;
	
	strcpy(p,kq->head->val);
	kq->head=kq->head->next;
	
	kq->length--;
	kfree(pop);

	return p;
}
