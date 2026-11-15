#include <stdio.h>
#include <stdlib.h>

#include "bst.h"

BSTNode *insertBST(BSTNode *root, int id) {

    if(root == NULL) {

        BSTNode *node = malloc(sizeof(BSTNode));

        if(node == NULL)
            return NULL;

        node->id = id;
        node->left = NULL;
        node->right = NULL;

        return node;
    }

    if(id < root->id)
        root->left = insertBST(root->left, id);

    else if(id > root->id)
        root->right = insertBST(root->right, id);

    return root;
}

BSTNode *searchBST(BSTNode *root, int id) {

    if(root == NULL || root->id == id)
        return root;

    if(id < root->id)
        return searchBST(root->left, id);

    return searchBST(root->right, id);
}

void inorderBST(BSTNode *root) {

    if(root == NULL)
        return;

    inorderBST(root->left);

    printf("%d ", root->id);

    inorderBST(root->right);
}

void freeBST(BSTNode *root) {

    if(root == NULL)
        return;

    freeBST(root->left);
    freeBST(root->right);

    free(root);
}