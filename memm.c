#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include "memm.h"


static size_t SYSTEM_PAGE_SIZE = 0;

/* pointer to the head of page_families list */
vm_page_for_families_t *first_vm_page_for_families = NULL;

/*
 * Initizlization function
 * has to be called as the first one
 */
void
memm_init()
{
	SYSTEM_PAGE_SIZE = getpagesize();
}

/*
 * Get new memory page from the kernel
 */
static void *
memm_get_new_vm_page_from_kernel(int num_pages)
{
	char *vm_page = mmap(
			0,
			num_pages * SYSTEM_PAGE_SIZE,
			PROT_READ|PROT_WRITE|PROT_EXEC,
			MAP_ANON|MAP_PRIVATE,
			0, 0);

	if(vm_page == MAP_FAILED)
	{
		printf("Error: VM Page allocation failed\n");
		return NULL;
	}

	return (void*)vm_page;
}

/*
 * Return allocated page to the kernel
 */
static void
memm_return_vm_pages_to_kernel(void *addr, int num_pages)
{
	if(munmap(addr, num_pages * SYSTEM_PAGE_SIZE))
		printf("Error: Could not munmap VM pages to kernel!\n");
}

/*
 * Add new page_family for the goven struct
 */
void
memm_instantiate_new_page_family(char *struct_name,
				 uint32_t struct_size)
{
        vm_page_family_t *vm_page_family_curr = NULL;

	if(struct_size > SYSTEM_PAGE_SIZE)
	{
		fprintf(stderr, "Error: %s() Structure %s size exceeds system page size\n",
			__FUNCTION__, struct_name);
		return;
	}

	/* There is no page families yet */
	if(!first_vm_page_for_families)
	{
		first_vm_page_for_families =
			(vm_page_for_families_t *)memm_get_new_vm_page_from_kernel(1);
		first_vm_page_for_families->next = NULL;
                first_vm_page_for_families->page_ctr = 1;

                strncpy(first_vm_page_for_families->vm_page_family[0].struct_name,
                        struct_name, MEMM_MAX_STRUCT_NAME);
                first_vm_page_for_families->vm_page_family[0].struct_size = struct_size;
                return;
	}

        /* find last family page */
        vm_page_for_families_t *tmp_vm_page_for_families = first_vm_page_for_families;
        while(tmp_vm_page_for_families->next)
        {
                tmp_vm_page_for_families = tmp_vm_page_for_families->next;
        }
        if(tmp_vm_page_for_families->page_ctr <  MAX_FAMILIES_PER_VM_PAGE)
        {
                strncpy(tmp_vm_page_for_families->vm_page_family[tmp_vm_page_for_families->page_ctr].struct_name,
                        struct_name, MEMM_MAX_STRUCT_NAME);
                tmp_vm_page_for_families->vm_page_family[tmp_vm_page_for_families->page_ctr++].struct_size = struct_size;
        }
        else
        {
                tmp_vm_page_for_families->next =
                        (vm_page_for_families_t *)memm_get_new_vm_page_from_kernel(1);
                tmp_vm_page_for_families = tmp_vm_page_for_families->next;
                tmp_vm_page_for_families->next = NULL;
                tmp_vm_page_for_families->page_ctr = 1;
        }
}

/*
 * Displaying all registered page_family one by one
 */
void memm_print_registered_page_families()
{
    vm_page_for_families_t *tmp_vm_page_for_families = first_vm_page_for_families;
    while(tmp_vm_page_for_families)
    {
        for(int i = 0; i < tmp_vm_page_for_families->page_ctr; i++)
        {
            printf("Page Family : %s, Size = %d\n", tmp_vm_page_for_families->vm_page_family[i].struct_name, 
                                                    tmp_vm_page_for_families->vm_page_family[i].struct_size);
        }

        tmp_vm_page_for_families = tmp_vm_page_for_families->next;
    }

}

/*
 * Function used to search for paga_family with a specific structure name,
 * if not found, returns a NULL pointer
 */
vm_page_family_t *
lookup_page_family_by_name(char *struct_name_search)
{
    vm_page_for_families_t *tmp_vm_page_for_families = first_vm_page_for_families;
    while(tmp_vm_page_for_families)
    {
        for(int i = 0; i < tmp_vm_page_for_families->page_ctr; i++)
        {
            if(strcmp(tmp_vm_page_for_families->vm_page_family[i].struct_name, struct_name_search))
                return  &tmp_vm_page_for_families->vm_page_family[i];

        }

        tmp_vm_page_for_families = tmp_vm_page_for_families->next;
    }

    return NULL;
}
