#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

struct tailq_entry
{
    int value;

    TAILQ_ENTRY(tailq_entry) entries;
};

TAILQ_HEAD(, taild_entry) my_tailq_head;

int main(int argc, char** argv)
{
    struct tailq_entry *item;
    struct tailq_entry *tp_item;

    int i;
    TAILQ_INIT(&my_tailq_head);

    for (i = 0; i < 10; i++)
    {
        item = malloc(sizeof(*item));
        if (item == NULL)
        {
            perror("malloc failed!\n");
            exit(EXIT_FAILURE);
        }

        item->value = i;
        TAILQ_INSERT_TAIL(&my_tailq_head, item, entries);
    }

    TAILQ_FOREACH(item, &my_tailq_head, entries)
    {
        printf("%d\n", item->value);
    }

    printf("Adding new item after 5: \n");
    TAILQ_FOREACH(item, &my_tailq_head, entries)
    {
        if (item->value == 5)
        {
        struct tailq_entry *new_item = malloc(sizeof(*new_item));
        if (new_item == NULL)
        {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }

        new_item->value = 10;

        TAILQ_INSERT_AFTER(&my_tailq_head, item, new_item, entries);
        break;
        }
    }

    TAILQ_FOREACH(item, &my_tailq_head, entries)
    {
        printf("%d\n", item->value);
    }

    while (item = TAILQ_FIRST(&my_tailq_head))
    {
        TAILQ_REMOVE(&my_tailq_head, item, entries);
        free(item);
    }

    if (!TAILQ_EMPTY(&my_tailq_head))
        printf("tail queue is not empty\n");
    
    return 0;
}
