#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "queue.h"

/*
 * Struct element represents every single element in the queue.
 * It stores the pointer of current element as well as the pointer of the next element. 	
 */
struct element {
	void* cur_ptr;
	struct element* next_ele;
	};


/* 
 * Struct queue stores the pointer of the first and the last element.
 * Also the size of queue is stored	
 */
struct queue {
	struct element* first;
	struct element* last;
	int size;
};


/*
 * Initialization of the queue.	
 */
queue_t queue_create(void)
{
	queue_t my_queue;
	my_queue = malloc(sizeof(struct queue));
	my_queue->size = 0;
	return my_queue;
}

/* 
 * The queue can only be freed when it is empty:
 * Free the elements first.	
 */
int queue_destroy(queue_t queue)
{	
	int* ptr;
	while(queue->size != 0) queue_dequeue(queue, (void**)&ptr);
	free(queue);
	return 0;
}


/*
 * To put an element into a queue.
 * Two situations are considered. 1: An empty queue. 2: The queue has some members.	
 */
int queue_enqueue(queue_t queue, void *data)
{
	struct element* ele_new = malloc(sizeof(struct element));
	assert(ele_new);
	
	if (queue->size == 0){
			ele_new->cur_ptr = data;
			queue->first = ele_new;
			queue->last = ele_new;
	} else {
			ele_new->cur_ptr = data;
			queue->last->next_ele = ele_new;
			queue->last = ele_new;
	}
	queue->size++;
	
	return 0;
}


/*
 * Delete the very first element.	
 * Return -1 if the queue is empty already
 */
int queue_dequeue(queue_t queue, void **data)
{
	if (queue->size == 0) return -1;
	*data = queue->first->cur_ptr;
	if (queue->size == 1) free(queue->first);
	else {
		struct element* temp = queue->first;
		queue->first = temp->next_ele;
		free(temp);
	}
	queue->size--;
	return 0;
}


/*
 * In this part, we delete the data called in other functions.
 * Return -1 if the data is not found in the queue.	
 */
int queue_delete(queue_t queue, void *data)
{
	int* ptr;
	struct element* itr = malloc(sizeof(struct element));
	itr->next_ele = queue->first;
	
	for (int i = 0; i < queue->size; i++) {
		if (itr->next_ele->cur_ptr == data) {
			if (i == 0) {
				free(itr);
				queue_dequeue(queue, (void**)&ptr);
			} else if (i == queue->size - 1) {
				queue->last = itr;
				free(itr->next_ele);
				queue->size--;
			} else {
				struct element* temp = itr->next_ele;
				itr->next_ele = itr->next_ele->next_ele;
				free(temp);
				queue->size--;
			}
			return 0;
		}
		else { //free temp?
			itr = itr->next_ele;
		}
	}
	return -1;
}

/*
 * Iterate the queue only there are members in the queue, return once exection is found	'
 */
int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	if (queue == NULL || func == NULL) return -1;
    if (queue->size == 0) return 0;
	
	int state =0;
    struct element *itr = queue->first;
    
    for (int i = 0; i < queue->size; i++) {
        state = func(itr->cur_ptr, arg);
        if (state == 1) {
            *data = itr->cur_ptr;
            break;
        }
        itr = itr->next_ele;
    }
    return 0;
}

/* 
 * Return the length or the size of the queue	
 */
int queue_length(queue_t queue)
{
	return queue->size;
}

