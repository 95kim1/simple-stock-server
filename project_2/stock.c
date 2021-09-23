#include "stock.h"
#include "csapp.h"

/* $begin struct item functions */
void copy_item(struct item* dest, struct item* src) {
  dest->ID = src->ID;
  dest->left_stock = src->left_stock;
  dest->price = src->price;
}

/* used for creating a node of stock_BST */
stock_node* create_stock_node(struct item* item) {
  stock_node* sn = (stock_node*)malloc(sizeof(stock_node));
  copy_item(&sn->item, item);

  sn->item.readcnt = 0;
  Sem_init(&sn->item.mutex_read, 0, 1);
  Sem_init(&sn->item.mutex_write, 0, 1);

  sn->left = sn->right = NULL;
  return sn;
}

/* used for a copy of node from stock_BST */
stock_node* create_stock_node_protected(struct item* item) {
  stock_node* sn = (stock_node*)malloc(sizeof(stock_node));
  
  P(&item->mutex_read);
  item->readcnt++;
  if (item->readcnt == 1)
    P(&item->mutex_write);
  V(&item->mutex_read);

  copy_item(&sn->item, item);

  P(&item->mutex_read);
  item->readcnt--;
  if (item->readcnt == 0)
    V(&item->mutex_write);
  V(&item->mutex_read);
  
  sn->left = sn->right = NULL;
  return sn;
}

/* fill struct item of stock_node */
void fill_item(struct item* item, int ID, int left_stock, int price) {
  item->ID = ID;
  item->left_stock = left_stock;
  item->price = price;
  
  item->readcnt = 0;

  Sem_init(&item->mutex_read, 0, 1);
  Sem_init(&item->mutex_write, 0, 1);
}

/* is nill item? */
bool nill_item(struct item* item) {
  if (item->ID == -1)
    return 1;
  return 0;
}
/* $end struct item functions */

/* $begin linked list functions */
void init_list(stock_list* sl) {
  sl->head = sl->tail = NULL;
  sl->length = 0;
}

bool empty_list(stock_list *sl) {
  return (sl->length == 0) ? 1 : 0;
}

struct item* front_list(stock_list *sl) {
  return (sl->head) ? &sl->head->item : NULL;
}

void push_list(stock_list *sl, struct item *item) {
  stock_node* sn = create_stock_node(item);
  
  if (empty_list(sl)) {
    sl->head = sl->tail = sn;
    sl->length = 1;
    return;
  }

  sl->tail->right = sn;
  sl->tail = sn;
  sl->length++;
}

void push_list_protected(stock_list *sl, struct item *item) {
  stock_node* sn = create_stock_node_protected(item);
  
  if (empty_list(sl)) {
    sl->head = sl->tail = sn;
    sl->length = 1;
    return;
  }

  sl->tail->right = sn;
  sl->tail = sn;
  sl->length++;
}

void pop_list(stock_list *sl) {
  if (empty_list(sl))
    return;
  
  stock_node *cur_node = sl->head;

  sl->head = sl->head->right;
  sl->length--;
  free(cur_node);

  if (empty_list(sl))
    sl->tail = NULL;
}

void clear_list(stock_list *sl) {
  if (empty_list(sl))
    return;
  
  while (!empty_list(sl))
    pop_list(sl);
}
/* $end linked list */

/* $begin BST functions */
void init_BST(stock_root *root) {
  struct item item;
  fill_item(&item, -1, 0, 0);
  stock_node *node = create_stock_node(&item);
  *root = node;
}

void fill_BST_node(int left, int right, struct item* arr, stock_node* cur_node, struct item *item_nill) {
  if (left > right)
    return;
  
  int mid = (left + right) / 2;

  /* fill current node */
  copy_item(&cur_node->item, arr + mid);
  
  /* fill left child as a nill node */
  stock_node *node = create_stock_node(item_nill);
  cur_node->left = node;

  /* fill right child as a nill node */
  node = create_stock_node(item_nill);
  cur_node->right = node;

  /* fill childs as a item node */
  fill_BST_node(left, mid-1, arr, cur_node->left, item_nill);
  fill_BST_node(mid+1, right, arr, cur_node->right, item_nill);
}

void fill_BST(struct item* arr, int size, stock_root *rootptr) {
  init_BST(rootptr);
  
  struct item item_nill;
  fill_item(&item_nill, -1, 0, 0);
  
  fill_BST_node(0, size - 1, arr, *rootptr, &item_nill);
}

void free_BST(stock_node* node) {
  if (node == NULL)
    return;

  stock_node *left = node->left;
  stock_node *right = node->right;
  
  free(node);
  
  free_BST(left);
  free_BST(right);
}

void backup(stock_node* node, char *buf, FILE *fp) {
  if (nill_item(&node->item))
    return;

  P(&node->item.mutex_read);
  node->item.readcnt++;
  if (node->item.readcnt == 1)
    P(&node->item.mutex_write);
  V(&node->item.mutex_read);
  
  sprintf(buf, "%d %d %d\n", node->item.ID, node->item.left_stock, node->item.price);
  Fputs(buf, fp);

  P(&node->item.mutex_read);
  node->item.readcnt--;
  if (node->item.readcnt == 0)
    V(&node->item.mutex_write);
  V(&node->item.mutex_read);

  backup(node->left, buf, fp);
  backup(node->right, buf, fp);
}

void backup_BST(stock_root root, char *filepath) {
  FILE* fp = fopen(filepath, "w");
  char buf[MAXLINE];
  backup(root, buf, fp);
  fclose(fp);
}

struct item* find_item(stock_root root, int ID) {
  stock_node* node = root;
  while (!nill_item(&node->item)) {
    if (node->item.ID < ID)
      node = node->right;
    else if (node->item.ID > ID)
      node = node->left;
    else
      return &node->item;
  }
  /* no match node */
  return NULL;
}
/* $end BST functions */

/* $begin sort functions */
bool comp_item(struct item* a, struct item* b) {
  if (a->ID <= b->ID)
    return 1;
  return 0;
}

void merge(int left_, int right_, struct item* arr) {
  int left = left_;
  int left_end = (left_ + right_) / 2;

  int right = left_end + 1;
  int right_end = right_;

  int idx = 0;
  int size = (right_-left_+1);
  struct item* temp_arr = (struct item*)malloc(sizeof(struct item) * size);

  while(left <= left_end && right <= right_end) {
    if (comp_item(arr+left, arr+right)) {
      copy_item(temp_arr + idx, arr + left);
      left++;
    }
    else {
      copy_item(temp_arr + idx, arr + right);
      right++;
    }
    idx++;
  }

  while(left <= left_end) {
    copy_item(temp_arr + idx, arr + left);
    idx++;
    left++;
  }

  while(right <= right_end) {
    copy_item(temp_arr + idx, arr + right);
    idx++;
    right++;
  }

  for (idx = 0; idx < size; idx++)
    copy_item(arr + idx + left_, temp_arr + idx);

  free(temp_arr);
}

void divide(int left, int right, struct item* arr) {
  if (left >= right)
    return;
  
  int mid = (left + right) / 2;

  divide(left, mid, arr);
  divide(mid+1, right, arr);
  
  merge(left, right, arr);
}

void sort_items(struct item* arr, int n) {
  int left = 0;
  int right = n-1;
  divide(left, right, arr);
}
/* $end sort functions */

/* $begin make stock tree */
int stoi(char *str_num) {
  int len = strlen(str_num);
  int num = 0;
  for (int i = 0; i < len; i++)
    num = num * 10 + (str_num[i] - '0');
  return num;
}

void parse_stock(int *temp, char *stock) {
  char str_num[16];
  int i, j, idx;
  
  temp[0] = temp[1] = temp[2] = -1;

  idx = i = j = 0;
  while(1) {
    if (stock[i] == ' ') {
      str_num[j] = '\0';
      temp[idx++] = stoi(str_num);
      j = 0;

      i++;
      while(stock[i] == ' ')
        i++;
    }
    else if (stock[i] == '\0') {
      str_num[j] = '\0';
      temp[idx++] = stoi(str_num);
      j = 0;
      return;
    }
    else
      str_num[j++] = stock[i++];
  }
}

stock_root read_stock_list(char *filepath) {
  FILE* fp = fopen(filepath, "r");
  char stock[255];
  int temp[3];
  struct item item;
  stock_list sl;
  
  /*linked list*/
  init_list(&sl);
  
  while(fgets(stock, 255, fp)) {
    if (stock[strlen(stock)-1] == '\n')
      stock[strlen(stock)-1] = '\0';

    parse_stock(temp, stock);
    
    if (temp[0] == -1 || temp[1] == -1 || temp[2] == -1)
      continue;

    fill_item(&item, temp[0], temp[1], temp[2]);
    push_list(&sl, &item);
  }

  fclose(fp);

  /* dynamic array */
  int size = sl.length;
  int i = 0;
  struct item *dynamic_arr = (struct item*)malloc(sizeof(struct item) * size);
  
  while (!empty_list(&sl)) {
    copy_item(dynamic_arr + i, front_list(&sl));
    pop_list(&sl);
    i++;
  }

  /* sort */
  sort_items(dynamic_arr, size);

  /* make BST */
  stock_root root;
  fill_BST(dynamic_arr, size, &root);
  free(dynamic_arr);

  return root;
}
/* $end stock tree */


stock_root root;

/* $begin functions related to user command */
void fill_stock_list(stock_list *sl, stock_node* node) {
  if (nill_item(&node->item))
    return;

  push_list_protected(sl, &node->item);

  fill_stock_list(sl, node->left);
  fill_stock_list(sl, node->right);
}

struct item* show_stocks(stock_root root, int *size) {
  stock_list sl;
  init_list(&sl);

  fill_stock_list(&sl, root);

  int idx = 0;
  struct item* arr = (struct item*)malloc(sizeof(struct item) * sl.length);
  
  while (!empty_list(&sl)) {
    copy_item(arr + idx, front_list(&sl));
    pop_list(&sl);
    idx++;
  }

  *size = idx;

  return arr;
}

int buy_stock(stock_root root, int ID, int cnt) {
  struct item *item = find_item(root, ID);

  if (item == NULL)
    return -1;

  int success = 1;

  /* wirte lock */
  P(&item->mutex_write);

  if (item->left_stock >= cnt) 
    item->left_stock -= cnt;
  else
    success = 0;

  V(&item->mutex_write);
  
  return success;
}

int sell_stock(stock_root root, int ID, int cnt) {
  struct item *item = find_item(root, ID);

  if (item == NULL)
    return -1;

  /* write lock */
  P(&item->mutex_write);
  
  item->left_stock += cnt;

  V(&item->mutex_write);

  return 1;
}

void show(int connfd) {
  int n;

  char buf[MAXLINE];
  char temp[MAXLINE];

  struct item* arr = show_stocks(root, &n);

  buf[0] = '\0';
  temp[0] = '\0';

  /* client에게 보낼 string line 개수 */
  sprintf(temp, "%d", n);
  strcat(temp, "\n");
  Rio_writen(connfd, temp, strlen(temp));

  /* stock 재고 보내기 */
  for (int i = 0; i < n; i++) {
    sprintf(buf, "%d %d %d\n", arr[i].ID, arr[i].left_stock, arr[i].price);
    Rio_writen(connfd, buf, strlen(buf));
  }

  free(arr);
}

void buy(int connfd, int ID, int cnt) {
  /* client에게 보낼 string line 개수 */
  Rio_writen(connfd, "1\n", 2);
  
  int success;
  if ((success = buy_stock(root, ID, cnt)) == 1) {
    char str_success[] = "[buy] success\n";
    Rio_writen(connfd, str_success, strlen(str_success));
  }
  else if (success == -1) {
    char no_id[32];
    sprintf(no_id, "No stock_%d\n", ID);
    Rio_writen(connfd, no_id, strlen(no_id));
  }
  else {
    char not_enough[64];
    sprintf(not_enough, "Not enough left stock_%d\n", ID);
    Rio_writen(connfd, not_enough, strlen(not_enough));
  }
}

void sell(int connfd, int ID, int cnt) {
  /* client에게 보낼 string line 개수 */
  Rio_writen(connfd, "1\n", 2);
  
  int success;
  if ((success = sell_stock(root, ID, cnt)) == 1) {
    char str_success[] = "[sell] success\n";
    Rio_writen(connfd, str_success, strlen(str_success));
  }
  else {
    char no_id[32];
    sprintf(no_id, "No stock_%d\n", ID);
    Rio_writen(connfd, no_id, strlen(no_id));
  }
}

int stock(int connfd) {
    int n; 
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
	      printf("server received %d bytes\n", n); //log

        int i = 0;
        while (buf[i] != '\n' && buf[i] != '\0' && buf[i] != ' ')
            i++;
        buf[i++] = '\0';

        /* show request */
        if (!strcmp(buf, "show")) { 
          show(connfd);
        }
        /* buy/sell request */
        else {
          /* get stock_id */
          char *cur = buf + i;

          while(buf[i] != '\n' && buf[i] != '\0' && buf[i] != ' ')
            i++;
          buf[i++] = '\0';

          int stock_id = stoi(cur);

          /* get stock_cnt */
          cur = buf + i;
          while(buf[i] != '\n' && buf[i] != '\0' && buf[i] != ' ')
            i++;
          buf[i++] = '\0';

          int stock_cnt= stoi(cur);

          /* operation, buy/sell */
          if (!strcmp(buf, "buy"))
              buy(connfd, stock_id, stock_cnt);
          else if (!strcmp(buf, "sell"))
              sell(connfd, stock_id, stock_cnt);
        }
    }

    if (n == 0) // EOF (client broke connection descriptor)
      return 0;
    return 1; // not EOF
}
/* $end functions related to user command */
