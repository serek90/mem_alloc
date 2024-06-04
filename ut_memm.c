#include <stdio.h>
#include "api_memm.h"

typedef struct emp_ {
    char name[32];
    uint32_t emp_id;
} emp_t;

typedef struct student_ {
    char name[32];
    uint32_t rollno;
    uint32_t marks_phys;
    uint32_t marks_chem;
    uint32_t marks_math;
    struct student_ *next;
} student_t;

int main(int argc, char **argv){

    memm_init();

    MEMM_REG_STRUCT(emp_t);

    MEMM_REG_STRUCT(student_t);

    memm_print_registered_page_families();

    return 0;
}

