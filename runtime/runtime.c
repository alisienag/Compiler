// runtime.c
void* get_memory(unsigned long len);   // defined in runtime.s

#define HEADER_SIZE 8

typedef struct Block {
    //always used
    unsigned long size; //8bytes
    //only used when free so can use these next 2 parts during a malloc call
    struct Block* next;
} Block;

static char* heap_ptr = 0;
static char* heap_end = 0;
static Block* free_list = 0;

static void* bump(unsigned long total) {
    if (heap_ptr + total > heap_end) {
        unsigned long chunk = total > 65536 ? total : 65536;
        char* mem = (char*)get_memory(chunk);
        heap_ptr = mem;
        heap_end = mem + chunk;
    }
    void* result = heap_ptr;
    heap_ptr += total;
    return result;
}

void* malloc(unsigned long size) {
    size = (size + 15) & ~15UL;
    Block** list = &free_list;
    Block* cur = *list;

    while (cur != 0) {
        if (cur->size >= size) {
            *list = cur->next;
            return (char*)cur + HEADER_SIZE;
        }
        list = &cur->next;
        cur = cur->next;
    }

    Block* b = (Block*)bump(HEADER_SIZE + size);
    b->size = size;
    return (char*)b + HEADER_SIZE;
}

void free(void* ptr) {
    if (!ptr) return;
    Block* b = (Block*)((char*)ptr - HEADER_SIZE);
    b->next = free_list;
    free_list = b;
}

/*void* malloc(long size) {
    size = (size + 15) / 16 * 16;
    if (heap_ptr + size > heap_end) {
        long chunk = 65536;
        if (size > chunk) chunk = size;
        char* mem = (char*)get_memory(chunk);
        heap_ptr = mem;
        heap_end = mem + chunk;
    }
    char* result = heap_ptr;
    heap_ptr = heap_ptr + size;
    return result;
}*/
