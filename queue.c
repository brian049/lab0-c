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

    int same = 0;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (!strcmp(entry->value, safe->value) && (&safe->list != head)) {
            same = 1;
            list_del(&entry->list);
            free(entry->value);
            free(entry);
        } else if (same) {
            same = 0;
            list_del(&entry->list);
            free(entry->value);
            free(entry);
        }
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

struct list_head *merge(struct list_head *l1, struct list_head *l2)
{
    struct list_head *head = NULL, **ptr = &head, **node = NULL;

    while (l1 && l2) {
        node = strcmp(list_entry(l1, element_t, list)->value,
                      list_entry(l2, element_t, list)->value) > 0
                   ? &l2
                   : &l1;
        *ptr = *node;
        ptr = &(*ptr)->next;
        *node = (*node)->next;
    }

    *node = (struct list_head *) ((uintptr_t) l1 | (uintptr_t) l2);
    *ptr = *node;
    return head;
}

struct list_head *merge_sort(struct list_head *head)
{
    if (!head || !head->next)
        return head;

    struct list_head *fast = head->next;
    struct list_head *slow = head;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    fast = slow->next;
    slow->next = NULL;

    struct list_head *L1 = merge_sort(head);
    struct list_head *L2 = merge_sort(fast);

    return merge(L1, L2);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return;

    head->prev->next = NULL;
    head->next->prev = NULL;
    head->next = merge_sort(head->next);

    struct list_head *prev, *current;

    for (prev = head, current = head->next; current;
         prev = current, current = current->next)
        current->prev = prev;

    head->prev = prev;
    prev->next = head;

    if (descend)
        q_reverse(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    struct list_head *next;
    struct list_head *min = head->prev;
    struct list_head *node = min->prev;

    if (node == head)
        return 1;

    int count = 1;
    for (next = node->prev; node != head;
         node = node->prev, next = next->prev) {
        if (strcmp(list_entry(min, element_t, list)->value,
                   list_entry(node, element_t, list)->value) < 0) {
            list_del(node);
            q_release_element(list_entry(node, element_t, list));
        } else {
            min = node;
            count++;
        }
    }
    return count;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    struct list_head *next;
    struct list_head *max = head->prev;
    struct list_head *node = max->prev;

    if (node == head)
        return 1;

    int count = 1;
    for (next = node->prev; node != head;
         node = node->prev, next = next->prev) {
        if (strcmp(list_entry(max, element_t, list)->value,
                   list_entry(node, element_t, list)->value) > 0) {
            list_del(node);
            q_release_element(list_entry(node, element_t, list));
        } else {
            max = node;
            count++;
        }
    }
    return count;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
