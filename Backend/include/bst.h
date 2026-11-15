#ifndef BST_H
#define BST_H

typedef struct BSTNode {

    int id;

    struct BSTNode *left;
    struct BSTNode *right;

} BSTNode;

BSTNode *insertBST(BSTNode *root, int id);
BSTNode *searchBST(BSTNode *root, int id);
void inorderBST(BSTNode *root);
void freeBST(BSTNode *root);

#endif