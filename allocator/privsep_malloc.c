#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define SYSCALL_TRACEMALLOC 353
#define SIZE_ALIGN (4*sizeof(size_t))
#define SIZE_MASK (-SIZE_ALIGN)
#define PAGE_LENGTH sysconf(_SC_PAGESIZE)
#define align4(x) (((((x)-1)>>2)<<2)+4)
/* Define the block size since the sizeof will be wrong */
#define BLOCK_SIZE 20

void *base = NULL;
void *heaps[100] = {NULL};

/*block struct */
struct ps_chunk {
    size_t           size;
    struct ps_chunk *next;
    struct ps_chunk *prev;
    void            *ptr;
};

/* Page struct with list of empty and used chunks */
struct ps_page {
    struct ps_chunk *free;
    struct ps_chunk *used;
    struct ps_page *next;
    struct ps_page *prev;
    size_t size;
};

void print_heap_metadata() {
    int i;
    for (i=0;i<100;++i){
        if(heaps[i] != NULL){
            struct ps_page *page = (struct ps_page *) heaps[i];
            int k = 0;
            printf ("\n\nHEAP N %d\n\n", i);
            while (page != NULL) {
                struct ps_chunk *free = page->free;
                struct ps_chunk *used = page->used;
                printf ("=====================\n");
                printf ("PAGE %d, ADDRESS = %p\n", k,page);
                printf ("SIZE = %d, NEXT = %p, PREV = %p, FREE = %p, USED = %p\n\n\n", page->size,
                                                                                   page->next,
                                                                                   page->prev,
                                                                                   page->free,
                                                                                   page->used);
                int j = 0;
                while(free != NULL) {
                    printf ("~~~~~~~~~~~~~~~~~~\n");
                    printf ("FREE N %d, ADDRESS = %p\n", j, free);
                    printf ("SIZE = %d, NEXT = %p, PREV = %p, PTR = %p\n", free->size,
                                                                           free->next,
                                                                           free->prev,
                                                                           free->ptr);
                    printf ("~~~~~~~~~~~~~~~~~~\n\n");
                    free = free->next;
                    j++;
                }
                j=0;
                while(used != NULL) {
                    printf ("~~~~~~~~~~~~~~~~~~\n");
                    printf ("USED N %d, ADDRESS = %p\n", j, used);
                    printf ("SIZE = %d, NEXT = %p, PREV = %p, PTR = %p\n", used->size,
                                                                           used->next,
                                                                           used->prev,
                                                                           used->ptr);
                    printf ("~~~~~~~~~~~~~~~~~~\n\n");
                    used = used->next;
                    j++;
                }
                page = page->next;
                k++;
            }
            printf("=====================\n\n\n");
        }
    }
}
/* Add a new block at the top of the heap. Return NULL if things go wrong */
void *
extend_heap(size_t s,
            unsigned int privlev) {

    int times = s/PAGE_LENGTH;
    if (s%PAGE_LENGTH != 0) times++;
    if (privlev < 0 || privlev > 100) return NULL;

    unsigned long ret_sys = syscall(SYSCALL_TRACEMALLOC,
                                    NULL,
                                    times*PAGE_LENGTH,
                                    PROT_READ|PROT_WRITE,
                                    MAP_PRIVATE|MAP_ANONYMOUS,
                                    "MMAP",
                                    privlev);

    if (ret_sys == -EINVAL) return NULL;

    void *chunk = (void *) ret_sys;

    if (chunk == MAP_FAILED) return NULL;

    if (heaps[privlev] != NULL) {
        /*scorro pagine e aggiungo una pagina*/
        struct ps_page *page = heaps[privlev];
        while (page->next != NULL) page = page->next;
        /* create the new page for this privilege level */
        page->next = (struct ps_page *) malloc(sizeof(struct ps_page));
        /*setting prev pointer */
        page->next->prev = page;
        /*moving to the new created page */
        page = page->next;
        /*keep track of the page size for freeing correctly */
        page->size = times*PAGE_LENGTH;
        /*creation of used first chunk */
        page->used = (struct ps_chunk *)malloc (sizeof(struct ps_chunk));
        /*initialization of first used chunk */
        page->used->size = s;
        page->used->next = NULL;
        page->used->prev = NULL;
        page->used->ptr = chunk;

        if ((times*PAGE_LENGTH -s) > 0) {
            /*creation of free first chunk */
            page->free = (struct ps_chunk *)malloc (sizeof(struct ps_chunk));
            /*initialization of second used chunk */
            page->free->size = (times*PAGE_LENGTH) - s;
            page->free->next = NULL;
            page->free->prev = NULL;
            page->free->ptr = chunk+s;
        }
        else
            page->free = NULL;
        /*setting next and prev pointer to null */
        page->next = NULL;

    }
    else {
        /*genero la prima pagina*/
        struct ps_page *page = (struct ps_page *) malloc(sizeof(struct ps_page));
        /*Keep track of the page size for freeing correctly */
        page->size = times*PAGE_LENGTH;
        /*creation of used first chunk */
        page->used = (struct ps_chunk *)malloc (sizeof(struct ps_chunk));
        /*initialization of first used chunk */
        page->used->size = s;
        page->used->next = NULL;
        page->used->prev = NULL;
        page->used->ptr = chunk;
        if ((times*PAGE_LENGTH -s) > 0) {
            /*creation of free first chunk */
            page->free = (struct ps_chunk *)malloc (sizeof(struct ps_chunk));
            /*initialization of second used chunk */
            page->free->size = (times*PAGE_LENGTH)- s;
            page->free->next = NULL;
            page->free->prev = NULL;
            page->free->ptr = chunk+s;
        }
        else
            page->free = NULL;
        /*setting next and prev pointer to null */
        page->next = NULL;
        page->prev = NULL;
        /*stick new page to head of the heap for the requeste privlev */
        heaps[privlev] = page;
    }
    return (void *)chunk;
}

int count_page (struct ps_page *p) {
    int i=0;
    while (p != NULL) {
        p = p->next;
        i++;
    }
    return i;
}

void *
find_block(size_t size,
           unsigned int privlev) {

    struct ps_page *head = heaps[privlev];

    while (head != NULL) {
        struct ps_chunk *free = head->free;
        struct ps_chunk *used = head->used;
        while (free != NULL) {
            if (free->size == size) {
                /*case where I should move the whole chunk*/
                if (free->prev == NULL && free->next == NULL){
                    /* case free is head of the list */
                    if (used == NULL) {
                        head->used = head->free;
                        head->free = NULL;
                        return (void *)head->used->ptr;
                    }
                    while (used->next != NULL) used = used->next;
                    used->next = free;
                    free->prev = used;
                    free->next = NULL;
                    head->free = NULL;
                    return (void *)free->ptr;
                }
                else if (free->prev == NULL) {
                    /* first element of the list to be removed */
                    head->free = free->next;
                    free->next->prev = free->prev;
                    if (used == NULL) {
                        head->used = free;
                        free->prev = NULL;
                        free->next = NULL;
                        return (void *)head->used->ptr;
                    }
                    while (used->next != NULL) used = used->next;
                    used->next = free;
                    free->prev = used;
                    free->next = NULL;
                    return (void *)free->ptr;
                }
                else {
                    /* element in the middle of the list */
                    struct ps_chunk *pre_free = free->prev;

                    if (free->next != NULL){
                        pre_free->next = free->next;
                        free->next->prev = pre_free;
                    }
                    else
                        pre_free->next = NULL;

                    if (used == NULL) {
                        head->used = free;
                        head->used->next = NULL;
                        head->used->prev = NULL;
                        return (void *) head->used->ptr;
                    }
                    while(used->next != NULL) used = used->next;
                    used->next = free;
                    free->prev = used;
                    free->next = NULL;
                    return (void *)free->ptr;
                }
            }
            else if (free->size > size) {
                /*case where I should reduce the free chunk and create a new one into the used*/
                struct ps_chunk *occ = (struct ps_chunk *) malloc(sizeof(struct ps_chunk));
                occ->ptr    = free->ptr;
                occ->size   = size;
                free->size -= size;
                free->ptr  += size;
                if (used == NULL){
                    head->used = occ;
                    occ->prev = NULL;
                    occ->next = NULL;
                    return (void *) head->used->ptr;
                }
                while (used->next != NULL) used = used->next;
                used->next = occ;
                occ->prev  = used;
                occ->next  = NULL;
                return (void *)occ->ptr;
            }
            free = free->next;
        }
        head = head->next;
    }
    return NULL;
}
static int adjust_size(size_t *n) {
    if (*n-1 > PTRDIFF_MAX -SIZE_ALIGN - PAGE_LENGTH) {
        if(*n) {
            return -1;
        }
        else {
            *n = SIZE_ALIGN;
            return 0;
        }
    }
    *n = (*n + SIZE_ALIGN -1) & SIZE_MASK;
    return 0;
}

void *
privsep_malloc (size_t size,
                unsigned int privlev) {
    struct ps_page *page, *last;
    void *ptr;
    size_t s = size;
    //s = align4(size);
    if (privlev < 0 || privlev > 100) return NULL;
    if (adjust_size(&s) < 0 ) return NULL;

    if(heaps[privlev]){
        ptr = find_block(s, privlev);
        if (ptr == NULL) {
            ptr = extend_heap(s, privlev);
        }
    }
    else {
        ptr = extend_heap (s, privlev);
        if (!ptr)
            return NULL;
    }
    /*print_heap_metadata();*/
    return (void *)ptr;
}

struct ps_page *
get_heap_page(void *p, int *heap_page) {
    int i=0;
    for (i = 0; i < 100; i++){
        if (heaps[i] != NULL) {
            struct ps_page  *page = heaps[i];
            while(page != NULL){
                struct ps_chunk *used = page->used;
                while (used != NULL) {
                    if (used->ptr == p) {
                        *heap_page = i;
                        return page;
                    }
                    used = used->next;
                }
                page = page->next;
            }
        }
    }
    return NULL;
}
void fusion_free_chunk(struct ps_chunk *free_list) {
    /* This function should receive an ordered list */
    if (free_list == NULL) return ;
    while (free_list->next != NULL) {
        struct ps_chunk *next = free_list->next;
        if ((free_list->ptr+free_list->size) == next->ptr ) {
            /*here I should merge the two adjacent blocks freeing next */
            free_list->size += next->size;
            free_list->next = next->next;
            free(next);
        }
        else
            free_list = free_list->next;
    }
}

void free_page (struct ps_page *page, int privlev){

    if(page->used != NULL) return;

    //int res = munmap (page->free->ptr, page->size);
    unsigned long res = syscall(SYSCALL_TRACEMALLOC,
                                page->free->ptr,
                                page->size,
                                0,
                                0,
                                "MUNMAP",
                                0);

    if (res == -EINVAL) return;

    free(page->free);

    if(page->next == NULL && page->prev == NULL) {
        /* Last page */
        free(page);
        heaps[privlev] = NULL;
    }
    else if(page->next == NULL) {
        /* Tail element */
        struct ps_page *prev = page->prev;
        prev->next = NULL;
        free(page);
    }
    else if(page->prev == NULL) {
        /* First element */
        heaps[privlev] = page->next;
        page->next->prev = NULL;
        free(page);
    }
    else {
        /*in the middle */
        page->prev->next = page->next;
        page->next->prev = page->prev;
        free(page);
    }
    return;
}

struct ps_chunk *
insert_element_to_list (struct ps_chunk *list, struct ps_chunk *element){
    struct ps_chunk *tmp = list;
    struct ps_chunk *prev = NULL;

    if(list == NULL)  {
        element->prev = NULL;
        element->next = NULL;
        return element;
    }

    if(element == NULL) return list;

    while (tmp != NULL && element->ptr > tmp->ptr ){
        prev = tmp;
        tmp = tmp->next;
    }
    if (tmp == NULL) {
       prev->next = element;
       element->prev = prev;
       element->next = NULL;
    }
    else {
        if (prev !=NULL){
            element->next = prev->next;
            prev->next = element;
            element->prev = prev;
            tmp->prev = element;
        }
        else {
            tmp->prev = element;
            element->next = tmp;
            element->prev = NULL;
            return element;
        }
    }
    return list;
}

void
privsep_free(void *p) {
    int num_heap;
    struct ps_page *page = get_heap_page(p, &num_heap);
    //printf ("\n PAGE = %p, N_HEAP = %d\n", page, num_heap);
    if (page == NULL) return;

    struct ps_chunk *free = page->free;
    struct ps_chunk *used = page->used;

    while (used != NULL && used->ptr != p) used = used->next;

    if (used == NULL) return;
    //printf ("\n USED PAGE = %p", used);
    memset(used->ptr, 0, used->size);

    if (used->next == NULL && used->prev == NULL) {
        /* It is the last used block for this page so*/
        page->used = NULL;
        page->free = insert_element_to_list(page->free, used);
    }
    else if (used->next == NULL) {
        /* It is the tail of the list */
        used->prev->next = NULL;
        page->free = insert_element_to_list(page->free, used);
    }
    else if (used->prev == NULL) {
        /* It is the head */
        page->used = used->next;
        used->next->prev = NULL;
        page->free = insert_element_to_list(page->free, used);
    }
    else {
        /* It is in the middle */
        used->prev->next = used->next;
        page->free = insert_element_to_list(page->free, used);
    }

    fusion_free_chunk(page->free);

    if (page->free != NULL && page->free->size == page->size && count_page(heaps[num_heap]) > 100)
        free_page (page, num_heap);
    /*print_heap_metadata();*/
    return;
}
