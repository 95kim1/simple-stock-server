#include "csapp.h"
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define bool char

struct item {
  int ID;
  int left_stock;
  int price;
  int readcnt;
  sem_t mutex_read;
  sem_t mutex_write;
};

typedef struct stock_node_ {
  struct item item;
  struct stock_node_ *left;
  struct stock_node_ *right;
} stock_node;

typedef struct stock_list_ {
  stock_node *head;
  stock_node *tail;
  int length;
} stock_list;

typedef stock_node* stock_root;

extern stock_root root;

/* struct item functions */
void copy_item(struct item* dest, struct item* src);
stock_node* create_stock_node(struct item* item);
stock_node* create_stock_node_protected(struct item* item);
void fill_item(struct item* item, int ID, int left_stock, int price);
bool nill_item(struct item* item);

/* linked list functions */
void init_list(stock_list* sl);
bool empty_list(stock_list *sl);
struct item* front_list(stock_list *sl);
void push_list(stock_list *sl, struct item *item);
void push_list_protected(stock_list *sl, struct item *item);
void pop_list(stock_list *sl);
void clear_list(stock_list *sl);

/* BST functions */
void init_BST(stock_root *root);
void fill_BST_node(int left, int right, struct item* arr, stock_node* cur_node, struct item *item_nill);
void fill_BST(struct item* arr, int size, stock_root *rootptr);
void free_BST(stock_node* node);
void backup(stock_root root, char *buf, FILE* fp);
void backup_BST(stock_root root, char *filepath);
struct item* find_item(stock_root root, int ID);

/* to sort functions */
bool comp_item(struct item* a, struct item* b);
void merge(int left_, int right_, struct item* arr);
void divide(int left, int right, struct item* arr);
void sort_items(struct item* arr, int n);

/* to make stock tree */
int stoi(char *str_num);
void parse_stock(int *temp, char *stock);
//
stock_root read_stock_list(char *filepath);

/* to operate user command */
void fill_stock_list(stock_list *sl, stock_node* node);
struct item* show_stocks(stock_root root, int *size);
int buy_stock(stock_root root, int ID, int cnt);
int sell_stock(stock_root root, int ID, int cnt);
void show(int connfd);
void buy(int connfd, int ID, int cnt);
void sell(int connfd, int ID, int cnt);
//
int stock(int connfd);