/*this struct holds data entered by the user and a pointer to the next struct*/
struct request{
void* num;
struct request* next;
};


/*this struct holds data of the overall list, where the head is, where the tail is and the current length*/
struct ll{
struct request* head;
struct request* tail;
int length;
};

/*this function allocates and returns a pointer to a linked list and returns NULL if it cannot allocate one*/
struct ll* ll_create(void){
/*create a pointer to a struct ll and allocate space for that struct*/
struct ll* ll;
ll=(struct ll*) malloc(sizeof(struct ll));
/*return an error message if unable to allocate space*/
if(ll==NULL){
printf("unable to allocate memory\n");
return NULL;
}
return ll;
}

//delete the linked list, i don't think this entirely works
void ll_delete(struct ll *ll){
	struct request* tracker=ll->head;
	int counter=0;
	//printf("where am I seg faulting? %p %d\n", tracker->next, tracker->num);
	int i;
	for(i=0; i<ll->length; i++){
	counter++;
	ll->head=ll->head->next;
	free(tracker);
	tracker=ll->head;
	//printf("I have freed request %d \n", counter);
	}
	if(ll->head==ll->tail){
	counter++;
	free(ll->head);
	//printf("I have freed request %d \n", counter);
	}
	

}

/*add to the linked list under the given insertion rules and returns -1 (logical true) on success and returns 0 on failure (logical false)*/


int ll_enqueue(struct ll *ll, void *value){
/*declare a pointer to a request so that it can persist in memory after the function has exited*/
	struct request* item;
/*allocate memory for the request*/
/*can also be written as struct request* item=malloc(sizeof(struct request))*/
	item=(struct request*)malloc(sizeof(struct request));
/*if the memmory cannot be allocated for the request, tell the user that an error has occured and exit the program*/
	if(!item){
	printf("failed to create request\n");
	return 0;
	}
	item->num=value;
	
/*Set the next pointer to NULL if this is the only request. Set both the pointer to the head AND tail of the list to the same request if it is the only item in the queue*/

	if(ll->length==0){
	item->next=NULL;
	ll->head=item;
	ll->tail=item;
	ll->head->next=NULL;
	ll->tail->next=NULL;
	ll->length++;
	//printf("list length: %d\n", ll->length);
	}
	
	else{
	//struct request* tracker=ll->head;
	item->next=ll->head;
	ll->head=item;
	ll->length++;
	//printf("list length: %d\n", ll->length);
	
	}
	return -1;
	}

//dequeue the first item in the list
void* ll_dequeue(struct ll *ll){
	if(ll->length==1){
		struct request* tracker=ll->head;
		void* datdat;
		datdat=ll->head->num;
		//printf("check the new head %d\n", ll->head->num);
		free(tracker);
		//printf("memory freed\n");
		ll->length--;
		//printf("list length: %d\n", ll->length);
		return datdat;	
	}
	else if(ll->length==0){
		return (void*) -1;
	}
	else{	
		struct request* tracker=ll->head;
		void* datdat;
		datdat=ll->head->num;
		ll->head=ll->head->next;
		//printf("check the new head %d\n", ll->head->num);
		free(tracker);
		//printf("memory freed\n");
		ll->length--;
		//printf("list length: %d\n", ll->length);
		return datdat;
	}
}
