#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new_head = malloc(sizeof(struct list_head));
    if (!new_head)
        return NULL;

    INIT_LIST_HEAD(new_head);
    return new_head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list)
        q_release_element(entry);
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = malloc(sizeof(element_t));

    if (!node)
        return false;

    node->value = strdup(s);

    if (!node->value) {
        q_release_element(node);
        return false;
    }

    list_add(&node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = malloc(sizeof(element_t));

    if (!node)
        return false;

    node->value = strdup(s);

    if (!node->value) {
        q_release_element(node);
        return false;
    }

    list_add_tail(&node->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_first_entry(head, element_t, list);

    if (!entry)
        return NULL;

    if (!sp) {
        list_del(&entry->list);
        return entry;
    }

    size_t len = strlen(entry->value);
    if (len <= bufsize - 1) {
        strncpy(sp, entry->value, len);
        sp[len] = '\0';
    } else {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(&entry->list);
    return entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_last_entry(head, element_t, list);

    if (!entry)
        return NULL;

    if (!sp) {
        list_del(&entry->list);
        return entry;
    }

    size_t len = strlen(entry->value);
    if (len <= bufsize - 1) {
        strncpy(sp, entry->value, len);
        sp[len] = '\0';
    } else {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(&entry->list);
    return entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return NULL;

    element_t *entry;

    if (list_is_singular(head)) {
        entry = list_entry(head->next, element_t, list);
        list_del(head->next);
    } else {
        struct list_head *front = head->next, *rear = head->prev;
        while (front != rear && rear->next != front) {
            front = front->next;
            rear = rear->prev;
        }

        // Now the front pointer points to the middle node
        entry = list_entry(front, element_t, list);
        list_del(front);
    }
    free(entry->value);
    free(entry);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return NULL;

    bool same = 0;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        bool next = safe != list_entry(head, element_t, list) &&
                    !strcmp(entry->value, safe->value);
        if (next || same) {
            list_del(&entry->list);
            free(entry->value);
            free(entry);
        }
        same = next;
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *iterator, *safe;
    list_for_each_safe (iterator, safe, head) {
        list_move(iterator, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;

    if (k < 2)
        return;

    struct list_head ans, temp;
    INIT_LIST_HEAD(&ans);
    INIT_LIST_HEAD(&temp);

    while (!list_empty(head)) {
        struct list_head *current = head->next;
        for (int i = 1; i < k && (current != head); i++)
            current = current->next;

        list_cut_position(&temp, head,
                          (current == head) ? current->prev : current);
        if (current != head)
            q_reverse(&temp);

        list_splice_tail(&temp, &ans);
    }
    list_splice(&ans, head);
}

static void merge_sort_conquer(struct list_head *dest,
                               struct list_head *victim,
                               bool descend)
{
    int mask = descend ? 0 : INT_MIN;
    LIST_HEAD(result);
    struct list_head *insert;
    while (!list_empty(dest) && !list_empty(victim)) {
        element_t *e_victim = list_first_entry(victim, element_t, list),
                  *e_dest = list_first_entry(dest, element_t, list);
        insert = strcmp(e_victim->value, e_dest->value) & mask ? victim->next
                                                               : dest->next;
        list_move_tail(insert, &result);
    }

    insert = list_empty(dest) ? victim : dest;
    list_splice_tail_init(insert, &result);

    list_splice(&result, dest);
    return;
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_is_singular(head) || list_empty(head))
        return;

    struct list_head *right = head->next, *left = head->prev;
    while (true) {
        if (right == left)
            break;
        left = left->prev;
        if (right == left)
            break;
        right = right->next;
    }

    LIST_HEAD(mid);
    list_cut_position(&mid, head, right);

    q_sort(head, descend);
    q_sort(&mid, descend);

    merge_sort_conquer(head, &mid, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    q_reverse(head);
    element_t *entry, *safe;
    element_t *min = list_first_entry(head, element_t, list);
    list_for_each_entry_safe (entry, safe, head, list) {
        if (strcmp(entry->value, min->value) > 0) {
            list_del(&entry->list);
            q_release_element(entry);
        } else
            min = entry;
    }
    q_reverse(head);
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    q_reverse(head);
    element_t *entry, *safe;
    element_t *max = list_first_entry(head, element_t, list);
    list_for_each_entry_safe (entry, safe, head, list) {
        if (strcmp(entry->value, max->value) < 0) {
            list_del(&entry->list);
            q_release_element(entry);
        } else
            max = entry;
    }
    q_reverse(head);
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;

    struct list_head ans;
    INIT_LIST_HEAD(&ans);

    queue_contex_t *entry;
    list_for_each_entry (entry, head, chain) {
        // cppcheck-suppress uninitvar
        merge_sort_conquer(&ans, entry->q, descend);
        INIT_LIST_HEAD(entry->q);
    }

    list_splice(&ans, list_first_entry(head, queue_contex_t, chain)->q);
    return q_size(list_first_entry(head, queue_contex_t, chain)->q);
}
