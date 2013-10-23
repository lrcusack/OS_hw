#define BUFFER_LENGTH 256
#include <stdio.h>
#include <stdlib.h>


struct ring{
	int head;
	int tail;
	int in;
	int out;
	void* values[BUFFER_LENGTH];
	
};

static inline struct ring* rb_create(void){
	//allocate an array of ring structs for the ring buffer
	struct ring* rb=(struct ring*)malloc(sizeof(struct ring));
	if(rb==NULL){
		printf("can't create ring buffer\n");
		return NULL;
	}
	else{
	// set head of ring buffer to first element and tail of ring
	// buffer to second element
		rb->head=0;
		rb->tail=1;
		rb->in=0;
		rb->out=0;
		return rb;
	}
}

static inline void rb_delete(struct ring* rb){
	//free memory allocated
	free(rb);
}

static inline int rb_isempty(struct ring* rb){
	//return 0 if ring buffer is non-empty and -1 if buffer is empty
	if(rb->tail==rb->head+1){
		return -1;
	}
	else{
		return 0;
	}
}

static inline int rb_isfull(struct ring* rb){
	//returns 0 if the ring buffer is non-full and returns -1 if the 		//ring buffer is full
	if(rb->head==rb->tail){
		printf("ring buffer is full\n");
		return -1;	
	}
	else{
		return 0;
	}
}

static inline int rb_enqueue(struct ring* rb, void* value){
	
	if(rb_isfull(rb)) return 0;
	else{
	rb->values[rb->tail-1]=value;
	rb->in++;
	rb->tail=rb->in%BUFFER_LENGTH+1;

	return -1;
	}

}

static inline void* rb_dequeue(struct ring* rb){
	if(rb_isempty(rb)==-1){
		printf("ring buffer is empty\n");
		return NULL;
	}
	else {
		void* ptr;
		ptr=rb->values[rb->head];
		rb->out++;
		rb->head=rb->out%BUFFER_LENGTH;
		return ptr;
	}
}
