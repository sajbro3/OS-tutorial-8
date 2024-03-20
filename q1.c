#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#define MAX_LINE_LENGTH 256
//creates the proc stucture
typedef struct proc {
    char parent[MAX_LINE_LENGTH];
    char name[MAX_LINE_LENGTH];
    int priority;
    int memory;
    
    struct proc *left; //pointer on left child node
    struct proc *right; //pointer on right child node
} proc;
//creating the new process
struct proc *createprocnode(char *parent, char *name,int priority, int memory){
struct proc *newNode=(struct proc *)malloc(sizeof(struct proc));
if (newNode == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    //creates new nodes for the parent on the tree
    strcpy(newNode->parent,parent);
    strcpy(newNode->name,name);
    newNode->priority=priority;
    newNode->memory=memory;
    //will show NULL if theres no child on right or left
    newNode->left=NULL; 
    newNode->right=NULL;
    return newNode;
    }
    //function to insert process in the binary tree
    struct proc *insertProcess(struct proc *root,struct proc *process){
    if(root==NULL)
    return process;
    
    //determines if the process goes on the left
    if(strcmp(process->parent, root->name)<0)
    root->left=insertProcess(root->left,process);
   
    //determines if the process on the right.
    else if(strcmp(process->parent,root->name)>0)
    root->right=insertProcess(root->right,process);
    
    return root;
    }
    //prints process recurisvely
    void printProcessTree(struct proc *root){
    if(root==NULL)
    return;
    //prints current process
printf("Parent: %s, Name: %s, Priority: %d , Memory: %d\n",root->parent,root->name,root->priority,root->memory);  
 
printProcessTree(root->left); //prints on the left side of the tree
printProcessTree(root->right); //prints on the right side of the tree
}

int main(void){
char parent[MAX_LINE_LENGTH];
char name[MAX_LINE_LENGTH];
struct proc *proc_tree=NULL;
 FILE* file = fopen("process_tree.txt", "r");
    //checks to see if the .txt opens up properly or not
    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
char line[256];
while(fgets(line, sizeof(line),file)){
int priority,memory;
sscanf(line, "%[^,],%[^,],%d,%d", parent,name,&priority,&memory);
struct proc *newproc=createprocnode(parent,name,priority,memory);
proc_tree=insertProcess(proc_tree,newproc);//put the new process in the tree
}
fclose(file);
//prints the binary tree
printf("Process tree:\n");
printProcessTree(proc_tree);
return 0;
}  
