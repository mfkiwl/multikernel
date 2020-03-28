/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <nanvix/runtime/rmem.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/ulib.h>
#include <posix/stddef.h>

#define NUM_NODES 100
#define NONE 0

/*
 * Dijkstra routine from MiBench Benchmark.
 */

/* Foward definitions. */
extern void *nanvix_malloc(size_t size);
extern void nanvix_free(void *ptr);

/* Node struct definition. */
struct node
{
	int dist;
	int prev;
};

/* Item struct definition. */
struct item
{
	struct item *next;
	int node;
	int dist;
	int prev;
};

/* Queue head. */
struct item *head = NULL;

/* Print path for comparison purposes. */
void print_path (struct node *path_nodes, int node)
{
	if (path_nodes[node].prev != NONE)
		print_path(path_nodes, path_nodes[node].prev);

	uprintf (" %d", node);
}

/* Global queue size. */
int queue_count = 0;

/* Node path. */
struct node path_nodes[NUM_NODES];

/*
 * @brief Enqueues a node with it's distance and previous element.
 *
 * @details Enqueue a new element with the value for node and more information
 * to make a new item. Each item know about the elements next to itself.
 * Moreover, each new item is inserted in the end of the queue by iterating over
 * all known nodes.
 */
int enqueue (int node, int dist, int prev)
{
	struct item *new = (struct item*) nanvix_malloc(sizeof(struct item));
	struct item *last = head;

	if (!new)
		return (-1);

	new->node = node;
	new->dist = dist;
	new->prev = prev;
	new->next = NULL;

	if (!last)
		head = new;
	else
	{
		while (last->next) last = last->next;
		last->next = new;
	}
	queue_count++;
	return 0;
}


/*
 * @brief Dequeues a node with it's distance and previous element.
 *
 * @details Dequeues a node by eliminating the head. Information about the
 * deleted node will be passed by reference to posterior use.
 */
int dequeue (int *dequeue_node, int *dequeue_dist, int *dequeue_prev)
{
	struct item *kill = head;

	if (head)
	{
		*dequeue_node = head->node;
		*dequeue_dist = head->dist;
		*dequeue_prev = head->prev;
		head = head->next;
		nanvix_free(kill);
		queue_count--;
		return 0;
	}
	return (-1);
}

/*
 * @brief Executes the dijkstra algorithm.
 *
 * @details The algorithm is responsible to initalize the node path and insert
 * the first node in the queue. After this first stage, each node of the queue
 * will be analysed to see if its path will be the lowest possible. The analysis
 * will be done for each node adjacent to queue nodes.
 */
int nanvix_dijkstra(int* adj_matrix, int node_start, int node_end)
{
	/* Item infromation returned from dequeue. */
	int dequeue_prev, dequeue_node, dequeue_cost, dequeue_dist;
	int err;

	/* Initial path. */
	for (int i = 0; i < NUM_NODES; i++)
	{
		path_nodes[i].dist = NONE;
		path_nodes[i].prev = NONE;
	}

	if (node_start == node_end)
	{
#ifdef NDEBUG
		uprintf("Shortest path is 0 in cost. Just stay where you are.\n");
#endif
	}

	else
	{

		path_nodes[node_start].dist = 0;
		path_nodes[node_start].prev = NONE;

		if ((err = enqueue (node_start, 0, NONE)) == -1) return -1;

		while (queue_count > 0)
		{
			if ((err = dequeue (&dequeue_node, &dequeue_dist, &dequeue_prev)) == -1) return -1;

			for (int i = 0; i < NUM_NODES; i++)
				if ((dequeue_cost = adj_matrix[dequeue_node*NUM_NODES+i]) != NONE)
					if ((NONE == path_nodes[i].dist) || (path_nodes[i].dist > (dequeue_cost + dequeue_dist)))
					{
						path_nodes[i].dist = dequeue_dist + dequeue_cost;
						path_nodes[i].prev = dequeue_node;
						if ((err = enqueue (i, dequeue_dist + dequeue_cost, dequeue_node)) == -1) return -1;
					}
		}

#ifdef NDEBUG
		uprintf("Shortest path is %d in cost. ", path_nodes[node_end].dist);
		uprintf("Path is: ");
		print_path(path_nodes, node_end);
		uprintf("\n");
#endif
	}
	return 0;
}