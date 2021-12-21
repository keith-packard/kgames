/*
 * Copyright © 2021 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef _Xkw_list_h
#define _Xkw_list_h

#include <stdbool.h>

struct xkw_list {
    struct xkw_list *prev, *next;
};

static inline void
xkw_list_init(struct xkw_list *list)
{
    list->prev = list->next = list;
}

/* Insert entry at head of list */
static inline void
xkw_list_prepend(struct xkw_list *entry, struct xkw_list *head)
{
    struct xkw_list *next = head->next;

    entry->next = next;
    entry->prev = head;
    head->next = entry;
    next->prev = entry;
}

/* append entry to end of list */
static inline void
xkw_list_append(struct xkw_list *entry, struct xkw_list *head)
{
    struct xkw_list *prev = head->prev;
    entry->prev = prev;
    entry->next = head;
    head->prev = entry;
    prev->next = entry;
}

/* remove entry from list */
static inline void
xkw_list_del(struct xkw_list *entry)
{
    struct xkw_list *prev = entry->prev;
    struct xkw_list *next = entry->next;

    prev->next = next;
    next->prev = prev;
    xkw_list_init(entry);
}

/* check whether list is empty */
static inline bool
xkw_list_is_empty(struct xkw_list *head)
{
    return head->next == head;
}

#define container_of(ptr, type, member) ((type *)((void *) (ptr) - offsetof(type, member)))

#define xkw_foreach(pos, head, member)                                  \
    for (typeof(pos) __tmp =                                            \
             container_of((pos =                                        \
                           container_of((head)->next, typeof(*pos),      \
                                        member))->member.next,          \
                          typeof(*pos), member);                         \
         &pos->member != (head);                                        \
         pos = __tmp, __tmp = container_of(pos->member.next, typeof(*pos), member))

#define xkw_foreach_rev(pos, head, member)                              \
    for (typeof(pos) __tmp =                                            \
             container_of((pos =                                        \
                           container_of((head)->prev, typeof(*pos),      \
                                        member))->member.prev,          \
                          typeof(*pos), member);                         \
         &pos->member != (head);                                        \
         pos = __tmp, __tmp = container_of(pos->member.prev, typeof(*pos), member))

#endif /* _Xkw_list_h */
