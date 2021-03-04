#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "list.h"
//#include "quick-sort_non_recursive.h"

static uint16_t values[256];  // 256

static void list_qsort(struct list_head *head)
{
    struct list_head list_less, list_greater;
    struct listitem *pivot;
    struct listitem *item = NULL, *is = NULL;

    if (list_empty(head) || list_is_singular(head))
        return;

    INIT_LIST_HEAD(&list_less);
    INIT_LIST_HEAD(&list_greater);

    pivot = list_first_entry(head, struct listitem, list);
    list_del(&pivot->list);

    list_for_each_entry_safe (item, is, head, list) {
        if (cmpint(&item->i, &pivot->i) < 0)
            list_move_tail(&item->list, &list_less);
        else
            list_move(&item->list, &list_greater);
    }

    list_qsort(&list_less);
    list_qsort(&list_greater);

    list_add(&pivot->list, head);
    list_splice(&list_less, head);
    list_splice_tail(&list_greater, head);
}

typedef struct stack_ele STACK_ELE;
struct stack_ele {
    struct list_head temp_head;
    // bool is_right_child;
    int merged;  // defualt false
    int level;
};

static void list_qsort_non_recursive(struct list_head *head)
{
    struct listitem *item = NULL, *is = NULL;

    struct listitem *pivot[256];
    struct stack_ele stack[512];
    int stack_top = -1, stack_bottum = -1, stack_current = -1, pivot_top = -1,
        pivot_bottum = -1, pivot_current = -1;  // 256 as ARRAY_SIZE(values)

    if (list_empty(head) || list_is_singular(head))
        return;

    INIT_LIST_HEAD(&stack[0].temp_head);  // list_less
    INIT_LIST_HEAD(&stack[1].temp_head);  // list_large

    stack[0].level = 1;
    stack[1].level = 1;

    pivot[0] = list_first_entry(head, struct listitem, list);
    list_del(&pivot[0]->list);

    list_for_each_entry_safe (item, is, head, list) {
        if (cmpint(&item->i, &pivot[0]->i) < 0)
            list_move_tail(&item->list, &stack[0].temp_head);
        else
            list_move(&item->list, &stack[1].temp_head);
    }

    stack_top += 2;
    stack_bottum++;
    stack_current++;
    pivot_top += 1;
    pivot_bottum++;

    while (stack_top >= stack_current) {
        if (!list_empty(&stack[stack_current].temp_head) &&
            !list_is_singular(&stack[stack_current].temp_head)) {
            stack[stack_current].merged = 1;
            pivot_top++;
            pivot[pivot_top] = list_first_entry(&stack[stack_current].temp_head,
                                                struct listitem, list);
            list_del(&pivot[pivot_top]->list);

            INIT_LIST_HEAD(&stack[stack_top + 1].temp_head);
            INIT_LIST_HEAD(&stack[stack_top + 2].temp_head);

            list_for_each_entry_safe (item, is, &stack[stack_current].temp_head,
                                      list) {
                if (cmpint(&item->i, &pivot[pivot_top]->i) < 0)
                    list_move_tail(&item->list,
                                   &stack[stack_top + 1].temp_head);
                else
                    list_move(&item->list, &stack[stack_top + 2].temp_head);
            }
            stack[stack_top + 1].level = stack[stack_current].level + 1;
            stack[stack_top + 2].level = stack[stack_current].level + 1;
            stack_top += 2;
        } else
            stack[stack_current].merged = 0;
        stack_current++;
    }

    int less_index, greater_index = stack_top, merge_index;
    pivot_current = pivot_top;

    while (greater_index > stack_bottum + 1) {
        less_index = greater_index - 1;
        merge_index = less_index - 1;

        while (stack[merge_index].level != stack[less_index].level - 1)
            merge_index--;
        while (stack[merge_index].merged != 1)
            merge_index--;

        list_add(&pivot[pivot_current]->list, &stack[merge_index].temp_head);
        list_splice(&stack[less_index].temp_head,
                    &stack[merge_index].temp_head);
        list_splice_tail(&stack[greater_index].temp_head,
                         &stack[merge_index].temp_head);

        pivot_current--;
        greater_index = less_index - 1;
        stack[merge_index].merged = 0;
    }
    less_index = greater_index - 1;
    list_add(&pivot[pivot_current]->list, head);
    list_splice(&stack[less_index].temp_head, head);
    list_splice_tail(&stack[greater_index].temp_head, head);
}

int main(void)
{
    struct list_head testlist;
    struct listitem *item, *is = NULL;
    size_t i;

    random_shuffle_array(values, (uint16_t) ARRAY_SIZE(values));

    INIT_LIST_HEAD(&testlist);

    assert(list_empty(&testlist));

    for (i = 0; i < ARRAY_SIZE(values); i++) {
        item = (struct listitem *) malloc(sizeof(*item));
        assert(item);
        item->i = values[i];
        list_add_tail(&item->list, &testlist);
        printf("%d ", item->i);
    }
    printf("\n");

    assert(!list_empty(&testlist));

    qsort(values, ARRAY_SIZE(values), sizeof(values[0]), cmpint);
    // list_qsort(&testlist);
    list_qsort_non_recursive(&testlist);

    i = 0;
    list_for_each_entry_safe (item, is, &testlist, list) {
        assert(item->i == values[i]);
        printf("%d ", item->i);
        list_del(&item->list);
        free(item);
        i++;
    }
    printf("\n");

    assert(i == ARRAY_SIZE(values));
    assert(list_empty(&testlist));

    return 0;
}
