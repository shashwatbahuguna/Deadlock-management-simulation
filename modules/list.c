#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct num_node num_node;
typedef struct num_list num_list;

struct num_node
{
    int data;

    num_node *prev;
    num_node *next;
};

num_node *newNumNode(int val)
{
    num_node *Nnode = (num_node *)malloc(sizeof(num_node));
    Nnode->prev = Nnode->next = NULL;
    Nnode->data = val;

    return Nnode;
}

struct num_list
{
    num_node *head, *tail;
    int cnt;
};

num_list *newNumList()
{
    num_list *h = (num_list *)malloc(sizeof(num_list));
    h->cnt = 0;
    h->tail = h->head = NULL;
    return h;
}

void insertNode(int val, num_list *li)
{
    num_node *newNode = newNumNode(val);

    if (li->tail)
    {
        li->tail->next = newNode;
        newNode->prev = li->tail;
    }
    else
        li->head = newNode;

    li->tail = newNode;
    li->cnt++;
}

num_node *removeNode(num_list *li, num_node *node)
{
    if (!li->cnt)
        return NULL;

    if (node->prev)
        node->prev->next = node->next;

    if (node->next)
        node->next->prev = node->prev;

    if (node == li->head)
        li->head = li->head->next;

    li->cnt--;
    num_node *next = node->next;
    node->next = node->prev = NULL;
    free(node);
    return next;
}

void printList(num_list *li)
{
    num_node *temp = li->head;
    while (temp)
    {
        printf("%d->", temp->data);
        temp = temp->next;
    }
    printf("\n");
}

/* List Testing
int main()
{
    num_list *li = newNumList();
    insertNode(4, li);
    insertNode(10, li);
    insertNode(3, li);
    insertNode(5, li);
    insertNode(4, li);
    insertNode(8, li);
    printList(li);

    num_node *temp = li->head;
    while (li->cnt)
    {
        temp->data--;
        printf("%d, ", temp->data);
        if (temp->data)
            temp = temp->next;
        else
            temp = removeNode(li, temp);
        if (!temp)
            temp = li->head, printf("\n");
    }
}
*/