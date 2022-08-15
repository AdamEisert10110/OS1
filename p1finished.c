#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<dirent.h>

struct Node {
	char *name;
	char *path;
	int id;
	struct Node *next;
	struct Node *prev;
};
struct List {
	struct Node *head;
	struct Node *tail;
};
struct Node *create_node(char *name, char *path, int id){
	struct Node *node = (struct Node*)malloc(sizeof(struct Node));
	if (node == NULL) {
		printf("ERROR!");
		exit(-1);
	}

	node->name = strdup(name);
	node->path = strdup(path);
	node->id = id;
	node->next = NULL;
	node->prev = NULL;
	return node;
}
struct List *create_list() {
	struct List *list = (struct List*)malloc(sizeof(struct List));
	
	if(list == NULL) {
		printf("Couldnt create list, %s", strerror (errno));
		exit(-1);
	}

	list->head = NULL;
	list->tail = NULL;
	return list;
}
void destroy_list(struct List *list) {
	struct Node *ptr = list->head;
	struct Node *tmp;
	while(ptr != NULL) {
		free(ptr->name);
		free(ptr->path);
		tmp = ptr;
		ptr = ptr->next;
		free(tmp);
	}
	free(list);
}
void print(struct List *list) {
	struct Node *tmp = list->head;
	while(tmp != NULL) {
		printf("%d : %s/%s\n", tmp->id, tmp->path, tmp->name);
		tmp = tmp->next;
	}
}
void print_from_tail(struct List *list) {

	struct Node *tmp = list->tail;
	while(tmp != NULL) {
		printf("%d : %s/%s\n", tmp->id, tmp->path, tmp->name);
		tmp = tmp->prev;
	}	
}
void print_extra(struct List *list) {
	struct Node *tmp = list->head;
	tmp = tmp->next;
	while(tmp->next != NULL) {
		printf("(%d %s/%s)<-(%d %s%s)->(%d %s%s)\n", tmp->prev->id,
				tmp->prev->path,tmp->prev->name,
				tmp->id,tmp->path,tmp->name,
				tmp->next->id,tmp->next->path,tmp->next->name);
		tmp= tmp->next;
	}
}
int levels_deep(char *path) {
	int count = 0;
	int i;
	for(i = 0; i < strlen(path); i++){
		if(path[i] == '/' || path[i] == '\\') {
			count++;
		}
	}
	count++;
	//needed since dirs start with one extra in linux, might not work w Window
	return count;
}
void insert_by_level(char *name, char *path, int id, struct List *list) {	
	struct Node *tmp = list->head;
	struct Node *new_node = create_node(name, path, id);
	//empty list
	if(list->head == NULL && list->tail == NULL) {
		//printf("D");
		list->head = new_node;
		list->tail = new_node;
		return;
	}
	while(tmp->id <= new_node->id) {
		//at end of line, new_node needs to be tail
		if(tmp->next == NULL) {
			tmp->next = new_node;
			new_node->prev = tmp;
			list->tail = new_node;
			return;
		}
		tmp = tmp->next;
	}
	//In front
	if(tmp->prev == NULL) {
		tmp->prev = new_node;
		new_node->next = tmp;
		list->head = new_node;
		//return;
	}
	//in middle
	else {
		new_node->next = tmp;
		new_node->prev = tmp->prev;
		tmp->prev = new_node;
		new_node->prev->next = new_node;	
		//return;
	}
	return;
}			
void recurse(char *basepath, int level, struct List *list, int basep_length) {
	char path[1000];
	struct dirent *dp;
	DIR *dir = opendir(basepath);
	if(!dir)
		return;
	while ((dp = readdir(dir)) != NULL)
	{
		if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..") != 0)
		{
			int p_len;
			p_len = levels_deep(basepath) - basep_length;
			insert_by_level(dp->d_name, basepath, p_len, list);
			strcpy(path, basepath);
			strcat(path, "/");
			strcat(path, dp->d_name);
			recurse(path, level, list, basep_length);
		}
		level++;
	}
	closedir(dir);
}
void print_to_file(struct List *list, char *output_name) {
	FILE *fp;
	struct Node *tmp = list->head;
	int count, current_level;
	count = 0;
	current_level = 0;
	strcat(output_name,".txt");
	fp = fopen(output_name,"w+");
	while(tmp != NULL) {
		if(current_level != tmp->id){
			count = 0;
			current_level = tmp->id;
			//printf("%d\n",current_level);
		}
		count++;
		fprintf(fp, "%d:%d:%s/%s\n", tmp->id, count, tmp->path, tmp->name);
		tmp = tmp->next;
	}
	fclose(fp);
	return;
}

int main(int argc, char *argv[]) {
	
	if(argc < 3) {
		printf("Error, not enough arguments");
		exit(-1);
	}

	char *path;
	path = argv[1];
	char *output_name;
	output_name = argv[2];
	int basepath_depth;

	//path = "/mnt/c/Users/aleis/Documents/Spring2022/OS/Project1";
	//output = "o1.txt";

	//-1 for linux, +0 for windows
	basepath_depth = levels_deep(path) -1;
	struct List *list = create_list();
	
	insert_by_level("",path,1,list);
	recurse(path, 0, list,basepath_depth-1/*1*/ /*basepath_depth+1*/);
	//print(list);
	print_to_file(list, output_name);
	destroy_list(list);
	return 0;
}
